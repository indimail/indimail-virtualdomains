/*
 * $Log: resetquota.c,v $
 * Revision 1.2  2019-06-07 16:01:45+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:36:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_GRP_H
#include <grp.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#include <fmt.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <strmsg.h>
#endif
#include "dblock.h"
#include "count_dir.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: resetquota.c,v 1.2 2019-06-07 16:01:45+05:30 mbhangui Exp mbhangui $";
#endif

#define FATAL         "resetquota: fatal: "
#define WARN          "resetquota: warning: "

static char    *usage =
		"usage: resetquota [options] directory(s)\n"
		"       -v            ( verbose )\n"
		"       -u user       ( username )\n"
		"       -g group      ( group )\n"
		"       -p perm       ( permission )\n"
		"       -q quota_spec ( maildirquota quota_spec )"
		;

static void
die_nomem()
{
	strerr_warn1("resetquota: out of memory", 0);
	_exit(111);
}

int
get_options(int argc, char **argv, char **user, char **group, char **perm, char **quota)
{
	int             c;

	*user = *group = *perm = *quota = 0;
	verbose = 0;
	while ((c = getopt(argc, argv, "vu:g:p:q:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'u':
			*user = optarg;
			break;
		case 'g':
			*group = optarg;
			break;
		case 'p':
			*perm = optarg;
			break;
		case 'q':
			*quota = optarg;
			break;
		default:
			return(1);
		}
	}
	if (optind < argc)
		return (0);
	return (1);
}

int
main(int argc, char **argv)
{
	mdir_t          mailcount, mailsize;
	int             i, j, match, fd, lockfd, status;
	char           *user, *group, *perm, *quota;
	char            inbuf[4096], outbuf[512], strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	struct substdio ssin, ssout;
	static stralloc filename = {0}, line = {0};
	uid_t           uid = -1;
	gid_t           gid = -1;
	unsigned long   perms = 0600;
	struct passwd  *pw;
	struct group   *gr;

	if (get_options(argc, argv, &user, &group, &perm, &quota)) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}

	if (user) {
		if (!(pw = getpwnam(user))) {
			strerr_warn2(user, ": failed to get user info", 0);
			return(1);
		}
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	if (group) {
		if (!(gr = getgrnam(group))) {
			strerr_warn2(group, ": failed to get group info", 0);
			return(1);
		}
		gid = gr->gr_gid;
	}
	if (perm)
		scan_8long(perm, &perms);
	for (i = optind, status = 0; i < argc; i++) {
		if (!stralloc_copys(&filename, argv[i]) ||
				!stralloc_catb(&filename, "/maildirsize", 12) ||
				!stralloc_0(&filename))
			die_nomem();
#ifdef FILE_LOCKING
		if ((lockfd = getDbLock(filename.s, 1)) == -1) {
			strerr_warn3("resetquota: get_write_lock: ", filename.s, ": ", &strerr_sys);
			status = -1;
			continue;
		}
#endif
		if (!quota) {
			if ((fd = open_read(filename.s)) == -1) {
				strerr_warn3("resetquota: open-read: ", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
				delDbLock(lockfd, filename.s, 1);
#endif
				status = -1;
				continue;
			}
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("resetquota: read: ", filename.s, ": ", &strerr_sys);
				close(fd);
#ifdef FILE_LOCKING
				delDbLock(lockfd, filename.s, 1);
#endif
				status = -1;
				continue;
			}
			close(fd);
			if (match) {
				line.len--;
				line.s[line.len] = 0;
			} else {
				if (!stralloc_0(&line))
					die_nomem();
				line.len--;
			}
			quota = line.s;
		}
		if ((mailsize = count_dir(argv[i], &mailcount)) == -1) {
			status = -1;
			continue;
		}
		if ((fd = open_write(filename.s)) == -1) {
			strerr_warn3("resetquota: open-write: ", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(lockfd, filename.s, 1);
#endif
			status = -1;
			continue;
		}
		if (uid != -1 && gid != -1) {
			if (fchown(fd, uid, gid)) {
				strerr_warn3("resetquota: fchown: ", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
				delDbLock(lockfd, filename.s, 1);
#endif
				status = -1;
				continue;
			}
		}
		if (fchmod(fd, perms)) {
			strnum1[fmt_8long(strnum1, perms)] = 0;
			strerr_warn5("resetquota: fchmod (", strnum1, "): ", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(lockfd, filename.s, 1);
#endif
			status = -1;
			continue;
		}
		substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
		strnum1[i = fmt_ulong(strnum1, (unsigned long) mailsize)] = 0;
		strnum2[j = fmt_ulong(strnum2, (unsigned long) mailcount)] = 0;
		if (substdio_puts(&ssout, quota) ||
				substdio_put(&ssout, "\n", 1) ||
				substdio_put(&ssout, strnum1, i) ||
				substdio_put(&ssout, " ", 1) ||
				substdio_put(&ssout, strnum2, j) ||
				substdio_put(&ssout, "\n", 1) ||
				substdio_flush(&ssout)) {
			strerr_warn3("resetquota: write: ", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			delDbLock(lockfd, filename.s, 1);
#endif
			status = -1;
			continue;
		}
		close(fd);
		strmsg_out6(quota, "\n", strnum1, " ", strnum2, "\n");
		delDbLock(lockfd, filename.s, 1);
	}
	return (status);
}
