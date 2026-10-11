/*-
 * $Id: crcdiff.c,v 1.9 2026-10-11 20:45:30+05:30 Cprogrammer Exp mbhangui $
 *
 * crcdiff.c - fast version. Compare two crc lists and report differences.
 *
 * This progam will compare two crc lists and report the differences.
 *
 * By Jon Zeeff (zeeff@b-tech.ann-arbor.mi.us)
 *
 * Permission is granted to use this in any manner provided that
 * 1) the copyright notice is left intact,
 * 2) you don't hold me responsible for any bugs and
 * 3) you mail me any improvements that you make.
 *
 * Both files are read into memory once and indexed with hash tables, so
 * the run time is O(n + m)
 *
 * Line format:
 *   0x5d784e43 dr-xr-xr-x root\troot\tJun 11 07:57:46 2024 /usr/bin,tag=os
 *
 * Matching rules
 *   - a line in file1 is matched with the FIRST usable line in file2 that
 *     has the same file name (or is identical once ",tag=..." is ignored)
 *   - identical lines (ignoring ",tag=...") -> nothing reported
 *   - otherwise crc / permission / owner+group differences are reported
 * The file name is everything after the year field, so names containing
 * spaces are handled correctly.
 *
 * Created using claude
 *
 * © 2024 Manvendra Bhangui
 * All intellectual property developed during the term of this agreement, including but not limited
 * to crcdiff.c, shall be and remain the sole and exclusive property of Manvendra Bhangui
 */
#define _POSIX_C_SOURCE 200809L
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_QMAIL
#include <makeargs.h>
#include <strerr.h>
#include <fmt.h>
#include <subfd.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#include <alloc.h>
#include <byte.h>
#include <qprintf.h>
#endif

#define FATAL "crcdiff: "
#define WARN  "crcdiff: "

#ifndef	lint
static char     sccsid[] = "$Id: crcdiff.c,v 1.9 2026-10-11 20:45:30+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_QMAIL
typedef struct {
	char           *line;   /* whole line, no newline */
	char           *tail;   /* from the last tab: "Mon DD HH:MM:SS YYYY path" */
	char           *path;   /* file name incl. ",tag=..." */
	char           *key;    /* line truncated at ",tag=" */
	size_t          plen;   /* strlen(path) */
	size_t          klen;
	size_t          olen;   /* length of "owner\tgroup\t" (starts at col 22) */
} entry;

typedef struct {
	entry          *e;
	size_t          n, cap;
} list;

typedef struct {
	size_t         *slot;   /* index+1 into list, 0 = empty */
	size_t          mask;
} htab;

stralloc        line = { 0 };

static unsigned long
hash(const char *s, size_t len)
{
	unsigned long   h = 5381;
	while (len--)
		h = h * 33 + (unsigned char) *s++;
	return h;
}

static int
parse(entry *e, char *line, size_t len, int *errc)
{
	char           *p, *t;
	int             i;

	(void) errc;
	e->line = line;
	if (len < 23 || !(t = strrchr(line, '\t')))
		return -1;
	e->tail = t;
	/* skip: Mon DD HH:MM:SS YYYY  (4 blank separated fields) */
	p = t + 1;
	for (i = 0; i < 4; i++) {
		while (*p && *p != ' ')
			p++;
		while (*p == ' ')
			p++;
	}
	if (!*p)
		return -1;
	e->path = p;
	e->plen = strlen(p);
	e->olen = (size_t) (t - line) - 21;
	e->key = line;
	e->klen = len;
	if ((p = strstr(line, ",tag=")))
		e->klen = (size_t) (p - line);
	return 0;
}

static long
load(const char *fn, list *l, int which)
{
	int             fd, match;
	char            strnum[FMT_ULONG];
	long            lineno = 0;
	char           *copy;
	char            inbuf[8192];
	substdio        ssin;

	if ((fd = open_read(fn)) == -1)
		strerr_die4sys(-111, FATAL, "Unable to open ", fn, ": ");
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		entry           e;

		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn4(WARN, "read: ", fn, ": ", &strerr_sys);
			close(fd);
			return -1;
		}
		if (!line.len)
			break;
		if (match) {
			if (line.len < 1)
				continue;
			line.s[line.len - 1] = 0;
		} else
		if (!stralloc_0(&line))
			strerr_die2x(-111, FATAL, "out of memory");
		if (!(copy = (char *) malloc((size_t) line.len)))
			strerr_die2x(-111, FATAL, "out of memory");
		byte_copy(copy, line.len, line.s);
		line.len--;
		if (parse(&e, copy, (size_t) line.len, NULL)) {
			alloc_free(copy);
			strnum[fmt_int(strnum, which)] = 0;
			strerr_warn7(WARN, fn, ": Error in input data", strnum, "[", line.s, "]", 0);
			continue;
		}
		lineno++;
		if (l->n == l->cap) {
			l->cap = l->cap ? l->cap * 2 : 4096;
			if (!(l->e = realloc(l->e, l->cap * sizeof (entry))))
				strerr_die2x(-111, FATAL, "out of memory");
		}
		l->e[l->n++] = e;
	}
	close(fd);
	return lineno;
}

/* index by path and by key; first entry wins */
static int
build(const list *l, htab *byp, htab *byk)
{
	size_t          sz = 16, i, h;

	while (sz < l->n * 2 + 2)
		sz <<= 1;
	byp->mask = byk->mask = sz - 1;
	byp->slot = calloc(sz, sizeof (size_t));
	byk->slot = calloc(sz, sizeof (size_t));
	if (!byp->slot || !byk->slot)
		return -1;
	for (i = 0; i < l->n; i++) {
		const entry    *e = l->e + i;

		for (h = hash(e->path, e->plen) & byp->mask; byp->slot[h]; h = (h + 1) & byp->mask) {
			const entry    *o = l->e + byp->slot[h] - 1;
			if (o->plen == e->plen && !memcmp(o->path, e->path, e->plen))
				break;
		}
		if (!byp->slot[h])
			byp->slot[h] = i + 1;
		for (h = hash(e->key, e->klen) & byk->mask; byk->slot[h]; h = (h + 1) & byk->mask) {
			const entry    *o = l->e + byk->slot[h] - 1;
			if (o->klen == e->klen && !memcmp(o->key, e->key, e->klen))
				break;
		}
		if (!byk->slot[h])
			byk->slot[h] = i + 1;
	} /*- for (i = 0; i < l->n; i++) */
	return 0;
}

/* returns 0 if none, else lowest index+1 of matching entry */
static size_t
find(const list *l, const htab *byp, const htab *byk, const entry *e)
{
	size_t          h, a = 0, b = 0;

	for (h = hash(e->path, e->plen) & byp->mask; byp->slot[h]; h = (h + 1) & byp->mask) {
		const entry    *o = l->e + byp->slot[h] - 1;
		if (o->plen == e->plen && !memcmp(o->path, e->path, e->plen)) {
			a = byp->slot[h];
			break;
		}
	}
	for (h = hash(e->key, e->klen) & byk->mask; byk->slot[h]; h = (h + 1) & byk->mask) {
		const entry    *o = l->e + byk->slot[h] - 1;
		if (o->klen == e->klen && !memcmp(o->key, e->key, e->klen)) {
			b = byk->slot[h];
			break;
		}
	}
	if (a && b)
		return a < b ? a : b;
	return a ? a : b;
}

/* is the path (without ",tag=...") in the critical list? */
static char   **crit;
static size_t   ncrit;

static int
is_critical(const entry *e)
{
	size_t          i, len = e->plen;
	const char     *p = strstr(e->path, ",tag=");

	if (!ncrit)
		return 0;
	if (p)
		len = (size_t) (p - e->path);
	for (i = 0; i < ncrit; i++)
		if (strlen(crit[i]) == len && !memcmp(crit[i], e->path, len))
			return 1;
	return 0;
}

static void
load_critical(const char *fn)
{
	int             fd, match;
	size_t          n = 0;
	char            inbuf[8192];
	substdio        ssin;

	if ((fd = open_read(fn)) == -1)
		strerr_die4sys(-111, FATAL, "Unable to open ", fn, ": ");
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn4(WARN, "read: ", fn, ": ", &strerr_sys);
			close(fd);
			return;
		}
		if (!line.len)
			break;
		if (match) {
			if (line.len <= 1)
				continue;
			line.s[line.len - 1] = 0;
		} else
		if (!stralloc_0(&line))
			strerr_die2x(-111, FATAL, "out of memory");
		if (ncrit == n) {
			n = n ? n * 2 : 64;
			if (!(crit = realloc(crit, n * sizeof (char *))))
				strerr_die2x(-111, FATAL, "out of memory");
		}
		crit[ncrit++] = strdup(line.s);
	}
	close(fd);
}

static unsigned long add_count, del_count, changed_count, perm_count, owner_count,
				mod_count, crit_l1_count, crit_l2_count;
static int      show_crc;

static void
l2(const char *tag, const entry *e)
{
	if (is_critical(e)) {
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
		strerr_warn4(WARN, "L2 < ", tag, e->line + 11, 0);
		crit_l2_count++;
	}
}

static void
pr(const char *what, const entry *e)
{
	if (subprintf(subfdout, "%s%s\n", what, e->line + (show_crc ? 0 : 11)) == -1)
		strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
}

static int
suid(const entry *e)
{
	return e->line[14] == 's' || e->line[17] == 's';
}

static void
l1(const entry *o, const entry *n, int *flag)
{
	if (suid(o)) {
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
		strerr_warn3(WARN, "L1 < ", o->line + 11, 0);
		if (!*flag) {
			crit_l1_count++;
			*flag = 1;
		}
	}
	if (suid(n)) {
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
		strerr_warn3(WARN, "L1 > ", n->line + 11, 0);
		if (!*flag) {
			crit_l1_count++;
			*flag = 1;
		}
	}
}

static void
usage(void)
{
	strerr_die2x(-100, FATAL, "usage: crcdiff [-Cdi] [-c critical_list] file1 file2");
}

int
main(int argc, char **argv)
{
#ifdef CHECK_LICENSE
#ifdef HAVE_DKIM
	char          **lic_argv;
#endif
#endif
	int             c, ignore_changed = 0, ignore_dir = 0;
	list            l1s = { 0 }, l2s = { 0 };
	htab            p2, k2, p1, k1;
	size_t          i, m;

#ifdef CHECK_LICENSE
#ifdef HAVE_DKIM
	if (!(lic_argv = makeargs("checklic -p "VIGILPREFIX"/etc/vigil.pub "VIGILPREFIX"/etc/vigil.lic")))
		strerr_die2x(-111, FATAL, "out of memory");
	if (check_license(4, lic_argv))
		strerr_die2x(-100, FATAL, "invalid vigil license "VIGILPREFIX"/etc/vigil.lic: ");
	free_makeargs(lic_argv);
	optind = 1;
#else
	strerr_die1x(-100, "This program needs to be compiled with libdkim2");
#endif
#endif
	while ((c = getopt(argc, argv, "c:Cdi")) != -1) {
		switch (c)
		{
		case 'C':
			show_crc = 1;
			break;
		case 'c':
			load_critical(optarg);
			break;
		case 'i':
			ignore_changed = 1;
			break;
		case 'd':
			ignore_dir = 1;
			break;
		default:
			usage();
		}
	}
	if (argc - optind != 2)
		usage();
	if (load(argv[optind], &l1s, 1) < 0 || load(argv[optind + 1], &l2s, 2) < 0)
		_exit(-111);
	if (build(&l2s, &p2, &k2) || build(&l1s, &p1, &k1))
		strerr_die2x(-111, FATAL, "out of memory");
	/*- pass 1: deleted / changed in file2 */
	for (i = 0; i < l1s.n; i++) {
		const entry    *o = l1s.e + i, *n;
		int             crit_flag = 0;

		if (!(m = find(&l2s, &p2, &k2, o))) {
			pr("deleted    ", o);
			l2("<", o);
			del_count++;
			continue;
		}
		n = l2s.e + m - 1;
		if (o->klen == n->klen && !memcmp(o->key, n->key, o->klen))
			continue;	/*- identical */
		if (!(ignore_dir && o->line[11] == 'd' && n->line[11] == 'd') && strncmp(n->line, o->line, 10)) {
			if (!strcmp(n->tail, o->tail)) {
				if (!ignore_changed) {
					pr("changed    ", n);
					l2(">", n);
					changed_count++;
				}
			} else {
				pr("replaced<  ", o);
				pr("replaced>  ", n);
				l2(">", n);
				mod_count++;
			}
		}
		if (strncmp(n->line + 11, o->line + 11, 10)) {
			pr("permiss<   ", o);
			pr("permiss>   ", n);
			l1(o, n, &crit_flag);
			perm_count++;
		}
		if (strncmp(n->line + 22, o->line + 22, n->olen)) {
			pr("own/grp<   ", o);
			pr("own/grp>   ", n);
			l1(o, n, &crit_flag);
			owner_count++;
		}
	} /*- for (i = 0; i < l1s.n; i++) */

	/*- pass 2: added in file2 */
	for (i = 0; i < l2s.n; i++) {
		const entry    *n = l2s.e + i;

		if (!find(&l1s, &p1, &k1, n)) {
			pr("added      ", n);
			l2(">", n);
			add_count++;
		}
	}
	if (!add_count && !del_count && !changed_count && !perm_count && !owner_count && !mod_count) {
		if (subprintf(subfdout, "0 changes detected\n") == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
		_exit(0);
	}
	if (subprintf(subfdout, "\n%lu Changes detected\n",
				add_count + del_count + changed_count + perm_count + owner_count + mod_count) == -1 ||
			subprintf(subfdout, "--------------------------\n") == -1 ||
			subprintf(subfdout, "1. added               : %lu\n", add_count) == -1 ||
			subprintf(subfdout, "2. deleted             : %lu\n", del_count) == -1 ||
			subprintf(subfdout, "3. replaced            : %lu\n", mod_count) == -1)
		strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
	if (ignore_changed) {
		if (subprintf(subfdout, "4. permission  changes : %lu\n", perm_count) == -1 ||
				subprintf(subfdout, "5. owner/group changes : %lu\n", owner_count) == -1)
			strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
	} else {
		if (subprintf(subfdout, "4. changed             : %lu\n", changed_count) == -1 ||
				subprintf(subfdout, "5. permission  changes : %lu\n", perm_count) == -1 ||
				subprintf(subfdout, "6. owner/group changes : %lu\n", owner_count) == -1)
			strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
	}
	if (crit_l1_count || crit_l2_count) {
		if (subprintf(subfdout, "\n") == -1 ||
				subprintf(subfdout, "NOTE: Critical Changes\n") == -1 ||
				subprintf(subfdout, "1. critical L1 changes : %lu\n", crit_l1_count) == -1 ||
				subprintf(subfdout, "2. critical L2 changes : %lu\n", crit_l2_count) == -1)
			strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
	}
	if (subprintf(subfdout, "--------------------------\n") == -1 ||
			substdio_flush(subfdout) == -1)
		strerr_die2sys(-111, FATAL, "unable to write to stdout: ");
	_exit(1);
}
#else
#error "not compiled with libqmail"
#endif

