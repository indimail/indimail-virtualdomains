/*****************************************************************************

NAME:
datastore_kc.c -- implements the datastore, using kyotocabinet.

AUTHORS:
Gyepi Sam <gyepi@praxis-sw.com>          2003
Matthias Andree <matthias.andree@gmx.de> 2003
Stefan Bellon <sbellon@sbellon.de>       2003-2004
Pierre Habouzit <madcoder@debian.org>    2007
Denny Lin <dennylin93@hs.ntnu.edu.tw>    2015

******************************************************************************/

#include "common.h"

#include <kclangc.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "datastore.h"
#include "datastore_db.h"
#include "error.h"
#include "paths.h"
#include "xmalloc.h"
#include "xstrdup.h"

#define UNUSED(x) ((void)(x))

typedef struct {
    char *name;
    bool created;
    bool writable;
    KCDB *dbp;
} dbh_t;

static int kc_txn_begin(void *vhandle) {
    dbh_t *dbh = (dbh_t *)vhandle;

    if (!dbh->writable || kcdbbegintran(dbh->dbp, false))
        return DST_OK;
    print_error(__FILE__, __LINE__, "kcdbbegintran(%p), err: %d, %s",
                (void *)dbh->dbp,
                kcdbecode(dbh->dbp), kcdbemsg(dbh->dbp));
    return DST_FAILURE;
}

static int kc_txn_abort(void *vhandle) {
    dbh_t *dbh = (dbh_t *)vhandle;

    if (!dbh->writable || kcdbendtran(dbh->dbp, false))
        return DST_OK;
    print_error(__FILE__, __LINE__, "kcdbendtran(%p, false), err: %d, %s",
                (void *)dbh->dbp,
                kcdbecode(dbh->dbp), kcdbemsg(dbh->dbp));
    return DST_FAILURE;
}

static int kc_txn_commit(void *vhandle) {
    dbh_t *dbh = (dbh_t *)vhandle;

    if (!dbh->writable || kcdbendtran(dbh->dbp, true))
        return DST_OK;
    print_error(__FILE__, __LINE__, "kc_txn_commit(%p, true), err: %d, %s",
                (void *)dbh->dbp,
                kcdbecode(dbh->dbp), kcdbemsg(dbh->dbp));
    return DST_FAILURE;
}

static dsm_t dsm_kc = {
    /* public -- used in datastore.c */
    &kc_txn_begin,
    &kc_txn_abort,
    &kc_txn_commit,
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

dsm_t *dsm = &dsm_kc;

const char *db_version_str(void)
{
    static char v[80];
    if (v[0] == '\0')
        snprintf(v, sizeof(v) - 1, "Kyoto Cabinet %s (TreeDB)", KCVERSION);
    return v;
}


static dbh_t *dbh_init(bfpath *bfp)
{
    dbh_t *handle;

    handle = (dbh_t *)xmalloc(sizeof(dbh_t));
    memset(handle, 0, sizeof(dbh_t));

    handle->name = xstrdup(bfp->filepath);
    handle->created = false;
    handle->writable = false;
    handle->dbp = kcdbnew();

    return handle;
}


static void dbh_free(dbh_t *handle)
{
    if (handle != NULL) {
      xfree(handle->name);
      kcdbdel(handle->dbp);
      xfree(handle);
    }
}


bool db_is_swapped(void *vhandle)
{
    UNUSED(vhandle);

    return false;
}


bool db_created(void *vhandle)
{
    dbh_t *handle = (dbh_t *)vhandle;

    return handle->created;
}


void *db_open(void *env, bfpath *bfp, dbmode_t open_mode)
{
    dbh_t *handle;
    uint32_t mode;
    bool ret;

    UNUSED(env);

    handle = dbh_init(bfp);

    handle->writable = open_mode & DS_WRITE;
    mode = handle->writable ? KCOWRITER : KCOREADER;
    ret = kcdbopen(handle->dbp, handle->name, mode);
    if (!ret && handle->writable) {
        ret = kcdbopen(handle->dbp, handle->name, mode | KCOCREATE);
        handle->created = ret;
    }

    if (!ret)
        goto open_err;

    if (DEBUG_DATABASE(1))
        fprintf(dbgout, "kcdbopen(%s, %u)\n", handle->name, mode);

    return handle;

open_err:
    print_error(__FILE__, __LINE__, "kcdbopen(%s, %u), err: %d, %s",
                handle->name, mode,
                kcdbecode(handle->dbp), kcdbemsg(handle->dbp));
    dbh_free(handle);

    return NULL;
}


int db_delete(void *vhandle, const dbv_t *token)
{
    dbh_t *handle = (dbh_t *)vhandle;
    bool ret;

    ret = kcdbremove(handle->dbp, (const char *)token->data, token->leng);
    if (!ret) {
        print_error(__FILE__, __LINE__, "kcdbremove(\"%.*s\"), err: %d, %s",
                    CLAMP_INT_MAX(token->leng), (char *)token->data,
                    kcdbecode(handle->dbp), kcdbemsg(handle->dbp));
        exit(EX_ERROR);
    }

    return 0;
}


int db_get_dbvalue(void *vhandle, const dbv_t *token, dbv_t *val)
{
    dbh_t *handle = (dbh_t *)vhandle;
    char *data;
    size_t dsiz;

    data = kcdbget(handle->dbp, (const char *)token->data, token->leng, &dsiz);
    if (data == NULL)
        return DS_NOTFOUND;

    val->leng = min(val->leng, dsiz);
    memcpy(val->data, data, val->leng);
    kcfree(data);

    return 0;
}

int db_set_dbvalue(void *vhandle, const dbv_t *token, const dbv_t *val)
{
    dbh_t *handle = (dbh_t *)vhandle;
    bool ret;

    ret = kcdbset(handle->dbp, (const char *)token->data, token->leng, (const char *)val->data, val->leng);
    if (!ret) {
        print_error(__FILE__, __LINE__,
                    "kcdbset: (%.*s, %.*s), err: %d, %s",
                    CLAMP_INT_MAX(token->leng), (char *)token->data,
                    CLAMP_INT_MAX(val->leng), (char *)val->data,
                    kcdbecode(handle->dbp), kcdbemsg(handle->dbp));
        exit(EX_ERROR);
    }

    return 0;
}


void db_close(void *vhandle)
{
    dbh_t *handle = (dbh_t *)vhandle;

    if (handle == NULL)
        return;

    if (DEBUG_DATABASE(1))
        fprintf(dbgout, "kcdbclose: %s\n", handle->name);

    if (!kcdbclose(handle->dbp))
        print_error(__FILE__, __LINE__, "kcdbclose: %s, err: %d, %s",
                    handle->name,
                    kcdbecode(handle->dbp), kcdbemsg(handle->dbp));

    dbh_free(handle);
}


void db_flush(void *vhandle)
{
    dbh_t *handle = (dbh_t *)vhandle;

    if (!kcdbsync(handle->dbp, false, NULL, NULL))
        print_error(__FILE__, __LINE__, "kcdbsync(), err: %d, %s",
                    kcdbecode(handle->dbp), kcdbemsg(handle->dbp));
}

ex_t db_foreach(void *vhandle, db_foreach_t hook, void *userdata)
{
    dbh_t *handle = (dbh_t *)vhandle;
    KCCUR *cursor;
    dbv_t dbv_key;
    dbv_const_t dbv_data;
    size_t ksiz, dsiz;
    int ret;
    ex_t retval = EX_OK;
    char *key;
    const char *data;

    cursor = kcdbcursor(handle->dbp);
    if (!kccurjump(cursor)) {
        print_error(__FILE__, __LINE__, "kccurjump(), err: %d, %s",
                    kcdbecode(handle->dbp), kcdbemsg(handle->dbp));
        retval = EX_ERROR;
        goto done;
    }

    while ((key = kccurget(cursor, &ksiz, &data, &dsiz, true)) != NULL) {
        /* Copy to dbv_key and dbv_data */
        dbv_key.data = xstrdup(key);
        dbv_key.leng = ksiz;
        dbv_data.data = data;
        dbv_data.leng = dsiz;

        /* Call function */
        ret = hook(&dbv_key, &dbv_data, userdata);

        xfree(dbv_key.data);
        kcfree(key);

        if (ret != 0)
            break;
    }

done:
    kccurdel(cursor);

    return retval;
}

const char *db_str_err(int e)
{
    UNUSED(e);
    return "unknown error";
}
