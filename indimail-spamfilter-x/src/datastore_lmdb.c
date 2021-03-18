/*
 * NAME:
 * datastore_lmdb.c -- implements the datastore, using LMDB.
 *
 * AUTHORS:
 * Steffen Nurpmeso <steffen@sdaoden.eu>    2018, 2019
 * (copied from datastore_kc.c:
 * Gyepi Sam <gyepi@praxis-sw.com>          2003
 * Matthias Andree <matthias.andree@gmx.de> 2003, 2018
 * Stefan Bellon <sbellon@sbellon.de>       2003-2004
 * Pierre Habouzit <madcoder@debian.org>    2007
 * Denny Lin <dennylin93@hs.ntnu.edu.tw>    2015)
 */

/*
 * Remarks.
 *
 * 1. LMDB places anything inside transactions (txn).
 *    You open an environment (which may contain multiple DBs), create
 *    a transaction and open a DB in that transaction.
 * 2. LMDB is based on a finite-sized memory map.  When a writable transaction
 *    reaches the size limit, the transaction must be aborted, then the
 *    environment must be resized, then a new transaction has to be created.
 *    Resizing will not shrink, effectively.
 * 3. mdb_env_get_maxkeysize():
 *      Depends on the compile-time constant #MDB_MAXKEYSIZE. Default 511.
 *    We reject any keys which excess this.
 * 4. We assume xmalloc() aborts if out of memory.
 * 5. We assume no token->leng actually exceeds int32_t.
 *
 * In order to be able to deal with 2. we need to track all changes that are
 * performed in a txn, so that in case we are running against the wall we are
 * capable to replay all changes after having resized the map.
 *
 * Alternatively, define a_BFLM_FIXED_SIZE, in which case all the replay code
 * is not compiled, but instead the given size is fixed, and any DB overflow
 * results in program abortion.  Since the DB should only consume disc space
 * for those pages which are used, this should not hurt in practice.
 */

/* Alternative implementation: fixed DB size */
/*#define a_BFLM_FIXED_SIZE (ULONG_MAX >> (ULONG_MAX != UINT_MAX ? 22 : 1))*/

/* mdb_env_set_maxreaders() */
#define a_BFLM_MAXREADERS 7

#ifndef a_BFLM_FIXED_SIZE
    /* DB size grow.  Must be a power of two (we perform alignment)!
     * Space it so that a DB load does not run against walls too many times.
     * We try _TRIES times to resize for a single new entry before giving up */
# define a_BFLM_GROW (1u << 24)
# define a_BFLM_GROW_TRIES 3

    /* Size of one chunk of the intermediate txn cache, as above.
     * Space it so that a DB load does not require all too many.
     * Of course, if a token requires more space, we allocate a larger chunk */
# define a_BFLM_TXN_CACHE_SIZE (1u << 20)

    /* A cache entry consists of an uint32_t describing the length of the key.
     * If the high bit is set an uint32_t describing the length of the value
     * follows.  After the data buffers there possibly is alignment pad */
# define a_BFLM_TXN_CACHE_ALIGN(X) \
    (((X) + (sizeof(uint32_t) - 1)) & ~(sizeof(uint32_t) - 1))
#endif /* a_BFLM_FIXED_SIZE */

/* The DB names we use: one for our "is-created" event, the other for data */
#define a_BFLM_DB_NAME_MAN "BF_MAN"
#define a_BFLM_DB_NAME_DAT "BF_DAT"

#include "common.h"

#include <errno.h>
#include <limits.h>

#include <lmdb.h>

#include "datastore.h"
#include "datastore_db.h"
#include "error.h"
#include "paths.h"
#include "xmalloc.h"

#if MDB_VERSION_FULL < MDB_VERINT(0, 9, 22)
# error "Required LMDB version: 0.9.22 or later (0.9.11 may do, but untested)"
#endif

#ifndef MAX
# define MAX(A,B) ((A) < (B) ? (B) : (A))
#endif
#define UNUSED(x) ((void)(x))

enum a_bflm_flags{
    a_BFLM_NONE,
    a_BFLM_DEBUG = 1u<<0,
    a_BFLM_RDONLY = 1u<<1,
    a_BFLM_DB_CREATED = 1u<<2,  /* DBs were newly created */
    a_BFLM_DB_UNAVAIL = 1u<<3,  /* rdonly open, but no DB exists yet! */
    a_BFLM_HAS_TXN = 1u<<4
};

struct a_bflm{
    char *bflm_filepath;    /* bfpath.filepath (points to &self[1]) */
    MDB_env *bflm_env;
    MDB_txn *bflm_txn;
    MDB_cursor *bflm_cursor;
    MDB_dbi bflm_dbi;
    uint32_t bflm_flags;
    size_t bflm_maxkeysize; /* mdb_env_get_maxkeysize() */
    size_t bflm_dbsize;     /* LMDB bug: forgets env size after txn abort */
#ifndef a_BFLM_FIXED_SIZE
    struct a_bflm_txn_cache *bflm_txn_cache;    /* Stack thereof */
#endif
};

#ifndef a_BFLM_FIXED_SIZE
struct a_bflm_txn_cache{
    struct a_bflm_txn_cache *bflmtc_last;   /* Up-to-date (stack usage) */
    struct a_bflm_txn_cache *bflmtc_next;   /* Needs to be build before use! */
    char *bflmtc_caster;                    /* Current caster */
    char *bflmtc_max;                       /* ..imum usable byte, exclusive */
    /* Actually points to &self[1] TODO [0] or [8], dep. __STDC_VERSION__! */
    char *bflmtc_data;
};
#endif

static char const a_bflm_db_name_man[] = a_BFLM_DB_NAME_MAN;
static char const a_bflm_db_name_dat[] = a_BFLM_DB_NAME_DAT;

/**/
static struct a_bflm *a_bflm_init(bfpath *bfp, bool rdonly);
static int a_bflm__check_create(struct a_bflm *bflmp);
static void a_bflm_free(struct a_bflm *bflmp);

/**/
static int a_bflm_txn_begin(void *vhandle);
static int a_bflm_txn_abort(void *vhandle);
static int a_bflm_txn_commit(void *vhandle);

#ifndef a_BFLM_FIXED_SIZE
/* A transaction needs to be resized and all modifications in the cache need to
 * be replayed, because we have seen MDB_MAP_FULL (or MDB_MAP_RESIZED) */
static bool a_bflm_txn_mapfull(struct a_bflm *bflmp, bool close_cursor);

/* (NULL on success or an error message otherwise) */
static char const *a_bflm_txn__replay(struct a_bflm *bflmp);

/* Put an entry; it is a deletion if val_or_null is NULL.
 * Return NULL on success or an error message otherwise */
static char const *a_bflm_txn_cache_put(struct a_bflm *bflmp, MDB_val *key,
                    MDB_val *val_or_null);

/* Free the recovery stack and possible heap data */
static void a_bflm_txn_cache_free(struct a_bflm *bflmp);
#endif /* a_BFLM_FIXED_SIZE */

static dsm_t /* TODO const*/ a_bflm_dsm = {
    /* public -- used in datastore.c */
    &a_bflm_txn_begin,
    &a_bflm_txn_abort,
    &a_bflm_txn_commit,
    /* private -- used in datastore_db_*.c */
    NULL,	/* dsm_env_init          */
    NULL,	/* dsm_cleanup           */
    NULL,	/* dsm_cleanup_lite      */
    NULL,	/* dsm_get_env_dbe       */
    NULL,	/* dsm_database_name     */
    NULL,	/* dsm_recover_open      */
    NULL,	/* dsm_auto_commit_flags */
    NULL,	/* dsm_get_rmw_flag      */
    NULL,	/* dsm_lock              */
    NULL,	/* dsm_common_close      */
    NULL,	/* dsm_sync              */
    NULL,	/* dsm_log_flush         */
    NULL,	/* dsm_pagesize          */
    NULL,	/* dsm_purgelogs         */
    NULL,	/* dsm_checkpoint        */
    NULL,	/* dsm_recover           */
    NULL,	/* dsm_remove            */
    NULL,	/* dsm_verify            */
    NULL,	/* dsm_list_logfiles     */
    NULL	/* dsm_leafpages         */
};

static struct a_bflm *
a_bflm_init(bfpath *bfp, bool rdonly){
    /* No variable array for .bflm_filepath, use same method as in word.h */
    int e;
    char const *emsg;
    struct a_bflm *rv;
    size_t i;

    i = strlen(bfp->filepath) +1;
    rv = (struct a_bflm *)xmalloc(sizeof(*rv) + i);
    memset(rv, 0, sizeof *rv);
    memcpy(rv->bflm_filepath = (char*)&rv[1], bfp->filepath, i);

    rv->bflm_flags = (((DEBUG_DATABASE(1) || getenv("BF_DEBUG_DB") != NULL)
                ? a_BFLM_DEBUG : a_BFLM_NONE) |
            (rdonly ? a_BFLM_RDONLY : a_BFLM_NONE));

    e = mdb_env_create(&rv->bflm_env);
    if(e != MDB_SUCCESS){
        emsg = "mdb_env_open()";
        goto jerr1;
    }

    rv->bflm_maxkeysize = mdb_env_get_maxkeysize(rv->bflm_env);

    /* To acommodate with bogofilter's db_created() mechanism we cannot use the
     * unnamed DB which "always exists", but must place data in named ones */
    e = mdb_env_set_maxdbs(rv->bflm_env, 2);
    if(e != MDB_SUCCESS){
        emsg = "mdb_env_set_maxdbs()";
        goto jerr1;
    }

    mdb_env_set_maxreaders(rv->bflm_env, a_BFLM_MAXREADERS);

    /* TODO We may not do this unless going for a huge fixed size, because with
     * TODO v0.9.22 a further DB open will then crash in mdb_*_put() after
     * TODO a growing _mapsize call! ... */
#ifdef a_BFLM_FIXED_SIZE
    e = mdb_env_set_mapsize(rv->bflm_env, a_BFLM_FIXED_SIZE);
    if(e != MDB_SUCCESS){
        emsg = "mdb_env_set_mapsize()";
        goto jerr2;
    }
#endif

    e = mdb_env_open(rv->bflm_env, rv->bflm_filepath, MDB_NOSUBDIR, 0660);
    if(e != MDB_SUCCESS){
        emsg = "mdb_env_open()";
        goto jerr2;
    }

    /* Let us fake a "has been created" event :( */
    if(!(rv->bflm_flags & a_BFLM_RDONLY) &&
            (e = a_bflm__check_create(rv)) != MDB_SUCCESS){
        emsg = "cannot handle management DB";
        goto jerr2;
    }

    if(rv->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: init: %p [%s]\n",
            (long)getpid(), rv, rv->bflm_filepath);
jleave:
    return rv;

jerr2:
    mdb_env_close(rv->bflm_env);
jerr1:
    if(emsg != NULL)
        print_error(__FILE__, __LINE__, "LMDB[%ld]: init, %s: %d, %s",
            (long)getpid(), emsg, e, mdb_strerror(e));
    xfree(rv);
    rv = NULL;
    goto jleave;
}

static int
a_bflm__check_create(struct a_bflm *bflmp){
    char const *db_name;
    unsigned int f;
    int e;
    /* TODO compile-time-assert that MDB_CREATE is not 0 */

jredo_txn:
    e = mdb_txn_begin(bflmp->bflm_env, NULL, 0, &bflmp->bflm_txn);
    if(e != MDB_SUCCESS){
        if(e == MDB_MAP_RESIZED){
            mdb_env_set_mapsize(bflmp->bflm_env, 0);
            goto jredo_txn;
        }
        goto jleave;
    }

    db_name = a_bflm_db_name_man;
    f = 0;
jredo_dbi:
    e = mdb_dbi_open(bflmp->bflm_txn, db_name, f, &bflmp->bflm_dbi);
    if(e != MDB_SUCCESS){
        if(e == MDB_NOTFOUND && f == 0){
            bflmp->bflm_flags |= a_BFLM_DB_CREATED;
            f = MDB_CREATE;
            goto jredo_dbi;
        }
        goto jerr;
    }

    if(f == MDB_CREATE && db_name == a_bflm_db_name_man){
        db_name = a_bflm_db_name_dat;
        goto jredo_dbi;
    }

    e = mdb_txn_commit(bflmp->bflm_txn);
    if(e != MDB_SUCCESS)
jerr:
        mdb_txn_abort(bflmp->bflm_txn);
jleave:
    return e;
}

static void
a_bflm_free(struct a_bflm *bflmp){
    if(bflmp != NULL){
#ifndef a_BFLM_FIXED_SIZE
        if(bflmp->bflm_txn_cache != NULL){
            if(DEBUG_DATABASE(1))
                fprintf(dbgout, "LMDB _free(): error: there is txn_cache!\n");
            a_bflm_txn_cache_free(bflmp);
        }
#endif

        mdb_env_close(bflmp->bflm_env);

        if(bflmp->bflm_flags & a_BFLM_DEBUG)
            fprintf(dbgout, "LMDB[%ld]: a_bflm_free(%p [%s])\n",
                (long)getpid(), bflmp, bflmp->bflm_filepath);

        xfree(bflmp);
    }
}

static int
a_bflm_txn_begin(void *vhandle){
    char const *emsg;
    struct a_bflm *bflmp;
    int e;

    e = DST_OK;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: txn_begin(%p [%s])\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath);

    bflmp->bflm_flags &= ~(a_BFLM_HAS_TXN | a_BFLM_DB_UNAVAIL);
jredo_txn:
    e = mdb_txn_begin(bflmp->bflm_env, NULL,
            (bflmp->bflm_flags & a_BFLM_RDONLY ? MDB_RDONLY : 0),
            &bflmp->bflm_txn);
    if(e != MDB_SUCCESS){
        if(e == MDB_MAP_RESIZED){
            mdb_env_set_mapsize(bflmp->bflm_env, 0);
            goto jredo_txn;
        }
        emsg = "mdb_txn_begin()";
        goto jerr1;
    }

    e = mdb_dbi_open(bflmp->bflm_txn, a_bflm_db_name_dat, 0, &bflmp->bflm_dbi);
    if(e != MDB_SUCCESS){
        if(e == MDB_NOTFOUND && (bflmp->bflm_flags & a_BFLM_RDONLY)){
            bflmp->bflm_flags |= a_BFLM_DB_UNAVAIL;
            goto junavail;
        }
        emsg = "mdb_dbi_open()";
        goto jerr2;
    }

    e = mdb_cursor_open(bflmp->bflm_txn, bflmp->bflm_dbi, &bflmp->bflm_cursor);
    if(e != MDB_SUCCESS){
        emsg = "mdb_cursor_open()";
        goto jerr2;
    }

junavail:
    bflmp->bflm_flags |= a_BFLM_HAS_TXN;
    e = DST_OK;
jleave:
    return e;

jerr2:
    mdb_txn_abort(bflmp->bflm_txn);
jerr1:
    print_error(__FILE__, __LINE__, "LMDB[%ld]: txn_begin(), %s: %d, %s",
        (long)getpid(), emsg, e, mdb_strerror(e));
    e = DST_FAILURE;
    goto jleave;
}

static int
a_bflm_txn_abort(void *vhandle){
    struct a_bflm *bflmp;
    int e;

    e = DST_OK;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: txn_abort(%p [%s])\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath);

    if(!(bflmp->bflm_flags & a_BFLM_DB_UNAVAIL))
        mdb_cursor_close(bflmp->bflm_cursor);

    mdb_txn_abort(bflmp->bflm_txn);

#ifndef a_BFLM_FIXED_SIZE
    a_bflm_txn_cache_free(bflmp);
#endif

    bflmp->bflm_flags &= ~a_BFLM_HAS_TXN;
jleave:
    return e;
}

static int
a_bflm_txn_commit(void *vhandle){
    struct a_bflm *bflmp;
#ifndef a_BFLM_FIXED_SIZE
    int retries;
#endif
    int e;

    e = DST_OK;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: txn_commit(%p [%s])\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath);

    if(!(bflmp->bflm_flags & a_BFLM_DB_UNAVAIL))
        mdb_cursor_close(bflmp->bflm_cursor);

#ifndef a_BFLM_FIXED_SIZE
    retries = 0;
jredo:
#endif
    e = mdb_txn_commit(bflmp->bflm_txn);
    if(e != MDB_SUCCESS){
#ifndef a_BFLM_FIXED_SIZE
        if((e == MDB_MAP_FULL || e == MDB_MAP_RESIZED) &&
                ++retries <= a_BFLM_GROW_TRIES &&
                a_bflm_txn_mapfull(bflmp, false)){
            mdb_cursor_close(bflmp->bflm_cursor);
            goto jredo;
        }
#endif
        mdb_txn_abort(bflmp->bflm_txn);
        e = MDB_PANIC;
    }

#ifndef a_BFLM_FIXED_SIZE
    a_bflm_txn_cache_free(bflmp);
#endif

    bflmp->bflm_flags &= ~a_BFLM_HAS_TXN;
    if(e == MDB_SUCCESS)
        e = DST_OK;
    else{
        print_error(__FILE__, __LINE__, "LMDB[%ld]: txn_commit(): %d, %s",
            (long)getpid(), e, mdb_strerror(e));
        e = DST_FAILURE;
    }
jleave:
    return e;
}

#ifndef a_BFLM_FIXED_SIZE
static bool
a_bflm_txn_mapfull(struct a_bflm *bflmp, bool close_cursor){
    MDB_envinfo envinfo;
    char const *emsg;
    int e;
    size_t i;

    /* Abort transaction */
    if(DEBUG_DATABASE(1) && (bflmp->bflm_flags & a_BFLM_DB_UNAVAIL))
        exit(EX_ERROR);

    if(close_cursor)
        mdb_cursor_close(bflmp->bflm_cursor);

    mdb_txn_abort(bflmp->bflm_txn);

    /* Resize map.  To be super-safe, synchronize current map size first */
jredo_txn:
    mdb_env_set_mapsize(bflmp->bflm_env, 0);
    /* no error defined */mdb_env_info(bflmp->bflm_env, &envinfo);
    i = envinfo.me_mapsize;
    /* LMDB v0.9.23 bug: forgets the environment size upon TXN abort time
     * unless a successful TXN has been seen */
    if(bflmp->bflm_dbsize > i)
        i = bflmp->bflm_dbsize;

    if((size_t)-1 - i >= a_BFLM_GROW * 2){
        i += a_BFLM_GROW / 10;
        i = (i + (a_BFLM_GROW - 1)) & ~(a_BFLM_GROW - 1);
    }else if((size_t)-1 - i >= 1024u * 1024u * 2)
        i = (size_t)-1 - (1024u * 1024u - 1);
    else{
        emsg = "DB size too large";
        goto jerr1;
    }

    e = mdb_env_set_mapsize(bflmp->bflm_env, i);
    if(e != MDB_SUCCESS){
        emsg = "mdb_env_set_mapsize()";
        goto jerr1;
    }
    bflmp->bflm_dbsize = i;

    /* Recreate transaction */
    e = mdb_txn_begin(bflmp->bflm_env, NULL, 0, &bflmp->bflm_txn);
    if(e != MDB_SUCCESS){
        if(e == MDB_MAP_RESIZED)
            goto jredo_txn;
        emsg = "mdb_txn_begin()";
        goto jerr1;
    }

    e = mdb_dbi_open(bflmp->bflm_txn, a_bflm_db_name_dat, 0, &bflmp->bflm_dbi);
    if(e != MDB_SUCCESS){
        emsg = "mdb_dbi_open()";
        goto jerr2;
    }

    e = mdb_cursor_open(bflmp->bflm_txn, bflmp->bflm_dbi, &bflmp->bflm_cursor);
    if(e != MDB_SUCCESS){
        emsg = "mdb_cursor_open()";
        goto jerr2;
    }

    if((emsg = a_bflm_txn__replay(bflmp)) != NULL)
        goto jerr3;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: txn_mapfull(%p [%s]): "
            "recreated, new size %lu\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath,
            (unsigned long)envinfo.me_mapsize);
    e = 0;
jleave:
    return (e == 0);
jerr3:
    /* Done by TXN abort mdb_cursor_close(bflmp->bflm_cursor); */
jerr2:
    /* Done by TXN abort mdb_txn_abort(bflmp->bflm_txn); */
jerr1:
    print_error(__FILE__, __LINE__, "LMDB[%ld]: txn_mapfull(): %s, %d, %s",
        (long)getpid(), emsg, e, mdb_strerror(e));
    e = 1;
    goto jleave;
}

static char const *
a_bflm_txn__replay(struct a_bflm *bflmp){
    MDB_val key, val;
    char const *emsg;
    int e;
    uint32_t kl, vl;
    char *dp;
    struct a_bflm_txn_cache *head, *bflmtcp;

    /* First of all create a list in the right order */
    for(head = NULL, bflmtcp = bflmp->bflm_txn_cache; bflmtcp != NULL;
            bflmtcp = bflmtcp->bflmtc_last){
        bflmtcp->bflmtc_next = head;
        head = bflmtcp;
    }

    /* Then replay, using it */
    for(; head != NULL; head = head->bflmtc_next){
        for(dp = head->bflmtc_data; dp < head->bflmtc_caster;){
            bool isins;

            /* For actual loading always use memcpy() for simplicity.
             * (That is: C standard and undefined behaviour, who knows?) */
            memcpy(&kl, dp, sizeof kl);
            dp += sizeof kl;
            if((isins = ((kl & 0x80000000u) != 0))){
                kl ^= 0x80000000u;
                memcpy(&vl, dp, sizeof vl);
                dp += sizeof vl;
            }

            key.mv_size = kl;
            key.mv_data = dp;
            dp += kl;
            if(isins){
                val.mv_size = vl;
                val.mv_data = dp;
                dp += vl;

                e = mdb_cursor_put(bflmp->bflm_cursor, &key, &val, 0);
                if(e != MDB_SUCCESS){
                    emsg = "mdb_cursor_put()";
                    goto jleave;
                }
            }else{
                e = mdb_cursor_get(bflmp->bflm_cursor, &key, NULL,
                        MDB_SET_KEY);
                if(e != MDB_SUCCESS){
                    emsg = "mdb_cursor_get() for delete";
                    goto jleave;
                }
                e = mdb_cursor_del(bflmp->bflm_cursor, 0);
                if(e != MDB_SUCCESS){
                    emsg = "mdb_cursor_del()";
                    goto jleave;
                }
            }
            dp = (char*)a_BFLM_TXN_CACHE_ALIGN((uintptr_t)dp);
        }
    }

    emsg = NULL;
jleave:
    return emsg;
}

static char const *
a_bflm_txn_cache_put(struct a_bflm *bflmp, MDB_val *key, MDB_val *val_or_null){
    uint32_t ui;
    char *dp;
    struct a_bflm_txn_cache *bflmtcp;
    char const *emsg;
    size_t kl, vl, i;

    if((kl = key->mv_size) >= 0x7FFFFFFFu)
        goto jeoverflow;
    if(val_or_null != NULL){
        if((vl = val_or_null->mv_size) >= 0x7FFFFFFFu)
            goto jeoverflow;
        if((i = kl + vl) >= 0x7FFFFFFFu - 2 * sizeof(uint32_t))
            goto jeoverflow;
        i += 2 * sizeof(uint32_t);
    }else{
        vl = 0;
        if((i = kl) >= 0x7FFFFFFFu - sizeof(uint32_t))
            goto jeoverflow;
        i += sizeof(uint32_t);
    }
    i = a_BFLM_TXN_CACHE_ALIGN(i);

    /* XXX We actually should abort() the program instead: cannot be handled */
    if(i >= 0x7FFFFFFFu - sizeof(*bflmtcp)){
jeoverflow:
        emsg = "LMDB: entry too large to be stored";
        goto jleave;
    }

    /* Do we need to create a new cache chunk entry?
     * We are simple and only look into the top of the stack */
    if((bflmtcp = bflmp->bflm_txn_cache) == NULL)
        goto jcache_new;
    else if(i >= (size_t)(bflmtcp->bflmtc_max - bflmtcp->bflmtc_caster)){
jcache_new:
        i += sizeof(*bflmtcp);
        i = max(i, a_BFLM_TXN_CACHE_SIZE);
        dp = (char*)(bflmtcp = (struct a_bflm_txn_cache *)xmalloc(i));
        bflmtcp->bflmtc_last = bflmp->bflm_txn_cache;
        bflmp->bflm_txn_cache = bflmtcp;
        bflmtcp->bflmtc_caster = bflmtcp->bflmtc_data = (char*)&bflmtcp[1];
        i -= 2 * sizeof(uint32_t);
        bflmtcp->bflmtc_max = &dp[i];
    }

    /* For actual storing always use memcpy() for simplicity.
     * (That is: C standard and undefined behaviour, who knows?) */
    dp = bflmtcp->bflmtc_caster;
    ui = (uint32_t)kl;
    if(val_or_null != NULL)
        ui |= 0x80000000u;
    memcpy(dp, &ui, sizeof ui);
    dp += sizeof ui;
    if(val_or_null != NULL){
        ui = (uint32_t)vl;
        memcpy(dp, &ui, sizeof ui);
        dp += sizeof ui;
    }
    memcpy(dp, key->mv_data, kl);
    dp += kl;
    if(vl != 0){
        memcpy(dp, val_or_null->mv_data, vl);
        dp += vl;
    }
    bflmtcp->bflmtc_caster = (char*)a_BFLM_TXN_CACHE_ALIGN((uintptr_t)dp);

    emsg = NULL;
jleave:
    return emsg;
}

static void
a_bflm_txn_cache_free(struct a_bflm *bflmp){
    struct a_bflm_txn_cache *bflmtcp;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: cache_free(%p [%s])\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath);

    while((bflmtcp = bflmp->bflm_txn_cache) != NULL){
        bflmp->bflm_txn_cache = bflmtcp->bflmtc_last;
        xfree(bflmtcp);
    }
}
#endif /* a_BFLM_FIXED_SIZE */

dsm_t /* const TODO */ *dsm = &a_bflm_dsm;

void *
db_open(void *env, bfpath *bfp, dbmode_t open_mode){
    struct a_bflm *bflmp;
    UNUSED(env);

    if((bflmp = a_bflm_init(bfp, (open_mode == DS_READ))) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: db_open(%p [%s; rdonly=%d])\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath,
            !!(bflmp->bflm_flags & a_BFLM_RDONLY));
jleave:
    return bflmp;
}

void
db_close(void *vhandle){
    struct a_bflm *bflmp;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: db_close(%p [%s])\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath);

    a_bflm_free(bflmp);
jleave:;
}

bool
db_is_swapped(void *vhandle){
    UNUSED(vhandle);
    return false;
}

bool
db_created(void *vhandle){
    bool created;
    struct a_bflm *bflmp;

    created = ((bflmp = (struct a_bflm *)vhandle) != NULL &&
            (bflmp->bflm_flags & a_BFLM_DB_CREATED) != 0);
    return created;
}

int
db_get_dbvalue(void *vhandle, const dbv_t *token, dbv_t *value){
    MDB_val key, val;
    char const *emsg;
    struct a_bflm *bflmp;
    int e;

    e = DS_NOTFOUND;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DB_UNAVAIL)
        goto jleave;

    if((size_t)token->leng > bflmp->bflm_maxkeysize){
        if(bflmp->bflm_flags & a_BFLM_DEBUG)
            fprintf(dbgout, "LMDB[%ld]: get_dbvalue: key too big "
                "(> %lu bytes), ignoring %.*s\n",
                (long)getpid(),(unsigned long)bflmp->bflm_maxkeysize,
                (int)token->leng, (const char *)token->data);
        goto jleave;
    }

    key.mv_data = token->data;
    key.mv_size = token->leng;
    e = mdb_cursor_get(bflmp->bflm_cursor, &key, &val, MDB_SET);
    if(e != MDB_SUCCESS){
        emsg = "mdb_cursor_get()";
        goto jerr;
    }

    if(val.mv_size > value->leng){
        emsg = "value storage too small";
        e = ENOSPC;
        goto jerr;
    }
    memcpy(value->data, val.mv_data, value->leng = val.mv_size);

    e = 0;
jleave:
    if(DEBUG_DATABASE(3))
        fprintf(dbgout, "LMDB db_get_dbvalue(): %lu <%.*s> -> %d\n",
            (unsigned long)token->leng, (int)token->leng,
            (char const*)token->data, (e == 0));
    return e;
jerr:
    if(e != MDB_NOTFOUND){
        print_error(__FILE__, __LINE__, "LMDB[%ld]: db_get_dbvalue(), "
            "%s: %d, %s",
            (long)getpid(), emsg, e, mdb_strerror(e));
        exit(EX_ERROR);
    }
    e = DS_NOTFOUND;
    goto jleave;
}

int
db_set_dbvalue(void *vhandle, const dbv_t *token, const dbv_t *value){
    MDB_val key, val;
    char const *emsg;
    struct a_bflm *bflmp;
#ifndef a_BFLM_FIXED_SIZE
    int retries;
#endif
    int e;

    e = 0;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DB_UNAVAIL) /* XXX assert instead */
        goto jleave;

    if((size_t)token->leng > bflmp->bflm_maxkeysize){
        if(bflmp->bflm_flags & a_BFLM_DEBUG)
            fprintf(dbgout, "LMDB[%ld]: set_dbvalue: key too big "
                "(> %lu bytes), ignoring %.*s\n",
                (long)getpid(),(unsigned long)bflmp->bflm_maxkeysize,
                (int)token->leng, (const char *)token->data);
        goto jleave;
    }

#ifndef a_BFLM_FIXED_SIZE
    retries = 0;
jredo:
#endif
    key.mv_data = token->data;
    key.mv_size = token->leng;
    val.mv_data = value->data;
    val.mv_size = value->leng;
    e = mdb_cursor_put(bflmp->bflm_cursor, &key, &val, 0);
    if(e != MDB_SUCCESS){
#ifndef a_BFLM_FIXED_SIZE
        if(e == MDB_MAP_FULL && ++retries <= a_BFLM_GROW_TRIES &&
                a_bflm_txn_mapfull(bflmp, true))
            goto jredo;
#endif
        emsg = "mdb_cursor_put()";
        goto jerr;
    }

#ifndef a_BFLM_FIXED_SIZE
    if((emsg = a_bflm_txn_cache_put(bflmp, &key, &val)) != NULL)
        goto jerr;
#endif

    e = 0;
jleave:
    if(DEBUG_DATABASE(3))
        fprintf(dbgout, "LMDB db_set_dbvalue(): %lu <%.*s> -> %d\n",
            (unsigned long)token->leng, (int)token->leng,
            (char const*)token->data, (e == 0));
    return e;
jerr:
    print_error(__FILE__, __LINE__, "LMDB[%ld]: db_set_dbvalue(), %s: %d, %s",
        (long)getpid(), emsg, e, mdb_strerror(e));
    exit(EX_ERROR);
}

int
db_delete(void *vhandle, const dbv_t *token){
    MDB_val key;
    char const *emsg;
    struct a_bflm *bflmp;
#ifndef a_BFLM_FIXED_SIZE
    int retries;
#endif
    int e;

    e = 0;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DB_UNAVAIL)
        goto jleave;

    if((size_t)token->leng > bflmp->bflm_maxkeysize){
        if(bflmp->bflm_flags & a_BFLM_DEBUG)
            fprintf(dbgout, "LMDB[%ld]: delete: key too big "
                "(> %lu bytes), ignoring %.*s\n",
                (long)getpid(),(unsigned long)bflmp->bflm_maxkeysize,
                (int)token->leng, (const char *)token->data);
        goto jleave;
    }

#ifndef a_BFLM_FIXED_SIZE
    retries = 0;
jredo:
#endif
    key.mv_data = token->data;
    key.mv_size = token->leng;
    e = mdb_cursor_get(bflmp->bflm_cursor, &key, NULL, MDB_SET_KEY);
    if(e != MDB_SUCCESS){
        emsg = "mdb_cursor_get()";
        goto jerr;
    }

    e = mdb_cursor_del(bflmp->bflm_cursor, 0);
    if(e != MDB_SUCCESS){
#ifndef a_BFLM_FIXED_SIZE
        /* Should not happen, though */
        if(e == MDB_MAP_FULL && ++retries <= a_BFLM_GROW_TRIES &&
                a_bflm_txn_mapfull(bflmp, true))
            goto jredo;
#endif
        emsg = "mdb_cursor_del()";
        goto jerr;
    }

#ifndef a_BFLM_FIXED_SIZE
    if((emsg = a_bflm_txn_cache_put(bflmp, &key, NULL)) != NULL)
        goto jerr;
#endif

    e = 0;
jleave:
    if(DEBUG_DATABASE(3))
        fprintf(dbgout, "LMDB db_delete(): %lu <%.*s> -> %d\n",
            (unsigned long)token->leng, (int)token->leng,
            (char const*)token->data, (e == 0));
    return e;
jerr:
    print_error(__FILE__, __LINE__, "LMDB[%ld]: db_delete(), %s: %d, %s",
        (long)getpid(), emsg, e, mdb_strerror(e));
    if(e != MDB_NOTFOUND)
        exit(EX_ERROR);
    e = DS_NOTFOUND;
    goto jleave;
}

void
db_flush(void *vhandle){
    struct a_bflm *bflmp;

    if((bflmp = (struct a_bflm *)vhandle) != NULL){
        int e;

        if(bflmp->bflm_flags & a_BFLM_DEBUG)
            fprintf(dbgout, "LMDB[%ld]: db_flush(%p [%s])\n",
                (long)getpid(), bflmp, bflmp->bflm_filepath);

        e = mdb_env_sync(bflmp->bflm_env, true);
        if(e != MDB_SUCCESS)
            print_error(__FILE__, __LINE__, "LMDB[%ld]: db_flush(): %d, %s",
                (long)getpid(), e, mdb_strerror(e));
    }
}

ex_t
db_foreach(void *vhandle, db_foreach_t hook, void *userdata){
    dbv_t dbv_key;
    dbv_const_t dbv_val;
    char *buf;
    MDB_cursor_op cursor_op;
    MDB_cursor *fecp;
    struct a_bflm *bflmp;
    ex_t rv;

    rv = EX_OK;

    if((bflmp = (struct a_bflm *)vhandle) == NULL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DB_UNAVAIL)
        goto jleave;

    if(bflmp->bflm_flags & a_BFLM_DEBUG)
        fprintf(dbgout, "LMDB[%ld]: db_foreach(%p [%s])\n",
            (long)getpid(), bflmp, bflmp->bflm_filepath);

    if(mdb_cursor_open(bflmp->bflm_txn, bflmp->bflm_dbi, &fecp
            ) != MDB_SUCCESS){
        rv = EX_ERROR;
        goto jleave;
    }

    buf = NULL;
    for(cursor_op = MDB_FIRST;; cursor_op = MDB_NEXT){
        size_t i;
        int e;
        MDB_val key, val;

        e = mdb_cursor_get(fecp, &key, &val, cursor_op);
        if(e != MDB_SUCCESS){
            if(e != MDB_NOTFOUND){
                print_error(__FILE__, __LINE__, "LMDB[%ld]: db_foreach(): "
                    "%d, %s",
                    (long)getpid(), e, mdb_strerror(e));
                rv = EX_ERROR;
            }
            break;
        }

        /* Copy to dbv_key and dbv_val in order to avoid loss upon possible
         * action on the DB; should not matter, but NUL terminate them */
        dbv_key.leng = (uint32_t)(i = key.mv_size);
        dbv_key.data = buf = (char *)xrealloc(buf, i +1);
        memcpy(buf, key.mv_data, i);
        buf[i++] = '\0';
        dbv_val.leng = (uint32_t)val.mv_size;
        dbv_val.data = val.mv_data;

        rv = hook(&dbv_key, &dbv_val, userdata);

        if(rv != EX_OK)
            break;
    }
    if(buf != NULL)
        xfree(buf);

    mdb_cursor_close(fecp);
jleave:
    return rv;
}

const char *
db_version_str(void){
    return MDB_VERSION_STRING;
}

char const *
db_str_err(int e){
    return mdb_strerror(e);
}

/* vim:set et sts=4 sw=4 sts=4 tw=79: */
