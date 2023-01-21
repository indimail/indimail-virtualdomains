/*
 * $Log: vmoddomain.c,v $
 * Revision 1.2  2019-06-07 15:45:51+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:34:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#include <fmt.h>
#include <str.h>
#include <substdio.h>
#include <subfd.h>
#endif
#include "get_indimailuidgid.h"
#include "get_assign.h"
#include "check_group.h"
#include "r_mkdir.h"
#include "dblock.h"
#include "common.h"
#include "indimail.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: vmoddomain.c,v 1.2 2019-06-07 15:45:51+05:30 Cprogrammer Exp mbhangui $";
#endif

#define WARN    "vmoddomain: warning: "
#define FATAL   "vmoddomain: fatal: "

static stralloc tmpbuf = {0};

int             set_handler(char *, char *, uid_t, gid_t, int);
int             set_domain_limits(char *, uid_t, gid_t, int);

static void
usage()
{
	errout("vmoddomain",
			"usage: vmoddomain [options] domain\n"
			"options: -V         (print version number)\n"
			"         -v         (verbose)\n"
			"         -l 0|1     Enable Domain Limits\n"
			"         -f 0|1     Enable VFILTER capability\n"
			"         -h handler (can be one of the following\n"
			"                    "
			DELETE_ALL
			"\n"
			"                    "
			BOUNCE_ALL
			"\n"
			"                    Maildir    - Maildir Path\n"
			"                    email      - Email Addres\n"
			"                    IP Address - SMTPROUTE/QMTPROUTE spec)\n"
			"you need to specify handler and vfilter option or domain limits\n");
	errflush("vmoddomain");
	return;
}

static void
die_nomem()
{
	strerr_warn1("vmoddomain: out of memory", 0);
	_exit (111);
}

static int
get_options(int argc, char **argv, int *use_vfilter, int *domain_limits, 
	char **handler, char **domain)
{
	int             c;

	*use_vfilter = -1;
	*domain_limits = -1;
	*handler = *domain = 0;
	while ((c = getopt(argc, argv, "avf:h:l:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'l':
			scan_int(optarg, domain_limits);
			break;
		case 'f':
			scan_int(optarg, use_vfilter);
			break;
		case 'h':
			*handler = optarg;
			break;
		default:
			usage();
			return (1);
		}
	}
	if ((*use_vfilter != -1 && !*handler) || (*handler && *use_vfilter == -1) ||
		(!*handler && *domain_limits == -1 && *use_vfilter == -1)) {
		usage();
		return (1);
	} else
	if (optind < argc)
		*domain = argv[optind++];
	else {
		usage();
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{

	char           *domain = 0, *handler = 0;
	static stralloc TheDir = {0};
	int             use_vfilter, domain_limits;
	uid_t           uid, myuid;
	gid_t           gid, mygid;

	if (get_options(argc, argv, &use_vfilter, &domain_limits, &handler, &domain))
		return (1);
	if (!get_assign(domain, &TheDir, &uid, &gid)) {
		strerr_warn2(domain, ": domain does not exist", 0);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	myuid = getuid();
	mygid = getgid();
	if (myuid != 0 && myuid != indimailuid && mygid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strerr_die1x(100, "you must be root or indimail to run this program");
		return (1);
	}
	if (myuid & setuid(0))
		strerr_die2sys(111, FATAL, "setuid: ");
	if (access(TheDir.s, F_OK) && r_mkdir(TheDir.s, INDIMAIL_DIR_MODE, uid, gid))
		strerr_die4sys(111, FATAL, "r_mkdir: ", TheDir.s, ": ");
	if (chdir(TheDir.s))
		strerr_die4sys(111, FATAL, "chdir: ", TheDir.s, ": ");
	umask(0077);
	if (handler && set_handler(TheDir.s, handler, uid, gid, use_vfilter))
		return (1);
	if ((domain_limits == 0 || domain_limits == 1) && 
		set_domain_limits(TheDir.s, uid, gid, domain_limits))
		return (1);
	return (0);
}

int
set_handler(char *dir, char *handler, uid_t uid, gid_t gid, int use_vfilter)
{
	char            dbuf[FMT_ULONG + FMT_ULONG + 21], outbuf[512];
	char           *s;
	int             i, fd;
	struct substdio ssout;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

	if (!handler)
		return (0);
	/* Last case: the last parameter is a Maildir, an email address, ipaddress or hostname */
	if (str_diff(handler, BOUNCE_ALL) && str_diff(handler, DELETE_ALL)) {
		if (*handler == '/') {
			if (chdir (handler)) {
				strerr_warn3("vmoddomain: chdir: ", handler, ": ", &strerr_sys);
				return (1);
			}
			if (access("new", F_OK) || access("cur", F_OK) || access("tmp", F_OK)) {
				strerr_warn3("vmoddomain: ", handler, ": not a Maildir", 0);
				return (1);
			}
		} else {
			i = str_chr(handler, '@');
			if (!handler[i]) {/*- IP address */
				char           *ptr;
				int             i;

				for (i = 0, ptr = handler; *ptr; ptr++) {
					if (*ptr == ':')
						i++;
				}
				if (i != 2) {
					strerr_warn3("Invalid SMTPROUTE/QMTPROUTE Specification [", handler, "]", 0);
					return (1);
				}
			}
		}
	}
	if (!stralloc_copys(&tmpbuf, dir) ||
			!stralloc_catb(&tmpbuf, "/.qMail-default", 15) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(tmpbuf.s, 1)) == -1) {
		strerr_warn3("vmoddomain: get_write_lock: ", tmpbuf.s, ": ", &strerr_sys);
		return (1);
	}
#endif
	if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, 0400)) == -1) {
		strerr_warn3("vmoddomain: open: ", tmpbuf.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (1);
	}
	if (fchown(fd, uid, gid)) {
			s = dbuf;
			s += fmt_strn(s, ": (uid = ", 9);
			s += fmt_uint(s, uid);
			s += fmt_strn(s, ", gid = ", 8);
			s += fmt_uint(s, gid);
			s += fmt_strn(s, "): ", 3);
			*s++ = 0;
		strerr_warn3("vmoddomain: chown: ", tmpbuf.s, dbuf, &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (1);
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
#ifdef VFILTER
	if (use_vfilter == 1) {
		if (substdio_put(&ssout, "| ", 2) ||
				substdio_puts(&ssout, PREFIX) ||
				substdio_put(&ssout, "/sbin/vfilter '' ", 17) ||
				substdio_puts(&ssout, handler) ||
				substdio_flush(&ssout)) {
			strerr_warn3("vmoddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			unlink(tmpbuf.s);
#ifdef FILE_LOCKING
			delDbLock(lockfd, tmpbuf.s, 1);
#endif
			return (1);
		}
	} else
#endif
	if (substdio_put(&ssout, "| ", 2) ||
			substdio_puts(&ssout, PREFIX) ||
			substdio_put(&ssout, "/sbin/vdelivermail '' ", 22) ||
			substdio_puts(&ssout, handler) ||
			substdio_flush(&ssout)) {
		strerr_warn3("vmoddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (1);
	}
	close(fd);
	if (rename(tmpbuf.s, ".qmail-default")) {
		strerr_warn3("update_file: rename: ", tmpbuf.s, " --> .qmail-default: ", &strerr_sys);
		unlink(tmpbuf.s);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (1);
	}
#ifdef FILE_LOCKING
	delDbLock(lockfd, tmpbuf.s, 1);
#endif
	return (0);
}

int
set_domain_limits(char *dir, uid_t uid, gid_t gid, int domain_limits)
{
	int             fd;
	char            dbuf[FMT_ULONG + FMT_ULONG + 21];
	char           *s;

	if (!stralloc_copys(&tmpbuf, dir) ||
			!stralloc_catb(&tmpbuf, "/.domain_limits", 15) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!domain_limits) {
		if (unlink(tmpbuf.s)) {
			strerr_warn3("vmoddomain: unlink: ", tmpbuf.s, ": ", &strerr_sys);
			return (1);
		}
		return (0);
	}
	if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, 0400)) == -1)
	{
		strerr_warn3("vmoddomain: open: ", tmpbuf.s, ": ", &strerr_sys);
		return (1);
	}
	if (fchown(fd, uid, gid)) {
		s = dbuf;
		s += fmt_strn(s, ": (uid = ", 9);
		s += fmt_uint(s, uid);
		s += fmt_strn(s, ", gid = ", 8);
		s += fmt_uint(s, gid);
		s += fmt_strn(s, "): ", 3);
		*s++ = 0;
		strerr_warn3("vmoddomain: chown: ", tmpbuf.s, dbuf, &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
		return (1);
	}
	close(fd);
	return (0);
}
