/*
 * $Id: vmoddomain.c,v 1.7 2025-05-13 20:37:32+05:30 Cprogrammer Exp mbhangui $
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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#include <fmt.h>
#include <str.h>
#include <substdio.h>
#include <hashmethods.h>
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
static char     sccsid[] = "$Id: vmoddomain.c,v 1.7 2025-05-13 20:37:32+05:30 Cprogrammer Exp mbhangui $";
#endif

#define WARN    "vmoddomain: warning: "
#define FATAL   "vmoddomain: fatal: "

static stralloc tmpbuf = {0};

int             set_handler(char *, char *, uid_t, gid_t, int);
int             set_domain_limits(char *, uid_t, gid_t, int);
int             set_hash_method(char *, uid_t, gid_t, int);

static void
usage(void)
{
	errout("vmoddomain",
			"usage: vmoddomain [options] domain\n"
			"options: -V             (print version number)\n"
			"         -v             (verbose)\n"
			"         -l 0|1         Enable Domain Limits\n"
			"         -H 0|1|2|3|4|5 Enable Hash Method (DES, MD5, SHA-256,\n"
			"                        SHA-512, YESCRYPT)\n"
			"         -f 0|1         Enable VFILTER capability\n"
			"         -h handler     (can be one of the following\n"
			"                        "
			DELETE_ALL
			"\n"
			"                        "
			BOUNCE_ALL
			"\n"
			"                        Maildir    - Maildir Path\n"
			"                        email      - Email Addres\n"
			"                        IP Address - SMTPROUTE/QMTPROUTE spec)\n"
			"you need to specify handler and vfilter option, domain limits\n"
			"or hash method\n");
	errflush("vmoddomain");
	return;
}

static void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

static int
get_options(int argc, char **argv, int *use_vfilter, int *domain_limits,
	int *hash_method, char **handler, char **domain)
{
	int             c;
	int             do_hash = 0;
	char           *ptr;

	*use_vfilter = -1;
	*domain_limits = -1;
	*hash_method = -1;
	*handler = *domain = 0;
	while ((c = getopt(argc, argv, "avf:h:H:l:")) != opteof) {
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
		case 'H':
			do_hash = 1;
			if (!str_diffn(optarg, "DES", 4) || !str_diffn(optarg, "0", 2))
				*hash_method = DES_HASH;
			else
			if (!str_diffn(optarg, "MD5", 4) || !str_diffn(optarg, "1", 2))
				*hash_method = MD5_HASH;
			else
			if (!str_diffn(optarg, "SHA-256", 7) || !str_diffn(optarg, "2", 2))
				*hash_method = SHA256_HASH;
			else
			if (!str_diffn(optarg, "SHA-256", 7) || !str_diffn(optarg, "3", 2))
				*hash_method = SHA512_HASH;
			else
			if (!str_diffn(optarg, "YESCRYPT", 7) || !str_diffn(optarg, "4", 2))
				*hash_method = YESCRYPT_HASH;
			else
				usage();
			break;
		default:
			usage();
			return (1);
		}
	}
	if ((*use_vfilter != -1 && !*handler) || (*handler && *use_vfilter == -1) ||
			(!*handler && *domain_limits == -1 && *use_vfilter == -1 && *hash_method == -1) ||
			(do_hash && (*hash_method < DES_HASH || *hash_method > YESCRYPT_HASH))) {
		usage();
		return (1);
	} else
	if (optind < argc) {
		for (ptr = argv[optind]; *ptr; ptr++) {
			if (isupper(*ptr))
				strerr_die4x(100, WARN, "domain [", argv[optind], "] has an uppercase character");
		}
		*domain = argv[optind++];
	} else {
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
	int             use_vfilter, domain_limits, hash_method;
	uid_t           uid, myuid;
	gid_t           gid, mygid;

	if (get_options(argc, argv, &use_vfilter, &domain_limits, &hash_method, &handler, &domain))
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
	if (hash_method != -1 && set_hash_method(TheDir.s, uid, gid, hash_method))
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
			if (chdir (handler))
				strerr_die4sys(111, FATAL, ": chdir: ", handler, ": ");
			if (access("new", F_OK) || access("cur", F_OK) || access("tmp", F_OK)) {
				strerr_warn3(WARN, handler, ": not a Maildir", 0);
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
	if ((lockfd = getDbLock(tmpbuf.s, 1)) == -1)
		strerr_die4sys(111, FATAL, "get_write_lock: ", tmpbuf.s, ": ");
#endif
	if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, 0600)) == -1) {
		strerr_warn4(FATAL, "open: ", tmpbuf.s, ": ", &strerr_sys);
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
		strerr_warn4(FATAL, "chown: ", tmpbuf.s, dbuf, &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (1);
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
#ifdef VFILTER
	if (use_vfilter == 1) {
		if (substdio_put(&ssout, "| ", 2) ||
				substdio_puts(&ssout, PREFIX) ||
				substdio_put(&ssout, "/sbin/vfilter '' ", 17) ||
				substdio_puts(&ssout, handler) ||
				substdio_flush(&ssout)) {
			strerr_warn4(FATAL, "write error: ", tmpbuf.s, ": ", &strerr_sys);
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
		strerr_warn4(FATAL, "write error: ", tmpbuf.s, ": ", &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (1);
	}
	close(fd);
	if (rename(tmpbuf.s, ".qmail-default")) {
		strerr_warn4(FATAL, "update_file: rename: ", tmpbuf.s, " --> .qmail-default: ", &strerr_sys);
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
			strerr_warn4(FATAL, "unlink: ", tmpbuf.s, ": ", &strerr_sys);
			return (1);
		}
		return (0);
	}
	if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, 0644)) == -1)
	{
		strerr_warn4(FATAL, "open: ", tmpbuf.s, ": ", &strerr_sys);
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
		strerr_warn4(FATAL, "chown: ", tmpbuf.s, dbuf, &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
		return (1);
	}
	close(fd);
	return (0);
}

int
set_hash_method(char *dir, uid_t uid, gid_t gid, int hash_method)
{
	int             fd, len = 8;
	char            dbuf[FMT_ULONG + FMT_ULONG + 21], outbuf[512];
	char           *s, *h = "YESCRYPT";
	struct substdio ssout;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

	if (!stralloc_copys(&tmpbuf, dir) ||
			!stralloc_catb(&tmpbuf, "/hash_method", 12) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(tmpbuf.s, 1)) == -1)
		strerr_die4sys(111, FATAL, "get_write_lock: ", tmpbuf.s, ": ");
#endif
	if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, 0600)) == -1) {
		strerr_warn4(FATAL, "open: ", tmpbuf.s, ": ", &strerr_sys);
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
		strerr_warn4(FATAL, "chown: ", tmpbuf.s, dbuf, &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
		return (1);
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
	switch (hash_method)
	{
		case DES_HASH:
			h = "DES";
			len = 3;
			break;
		case MD5_HASH:
			h = "MD5";
			len = 3;
			break;
		case SHA256_HASH:
			h = "SHA-256";
			len = 7;
			break;
		case SHA512_HASH:
			h = "SHA-512";
			len = 7;
			break;
		case YESCRYPT_HASH:
			h = "YESCRYPT";
			len = 8;
			break;
		default:
			h = "YESCRYPT";
			len = 8;
			break;
	}
	if (substdio_put(&ssout, h, len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout)) {
		strerr_warn4(FATAL, "write error: ", tmpbuf.s, ": ", &strerr_sys);
		close(fd);
		unlink(tmpbuf.s);
#ifdef FILE_LOCKING
		delDbLock(lockfd, tmpbuf.s, 1);
#endif
		return (1);
	}
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, tmpbuf.s, 1);
#endif
	return (0);
}
/*
 * $Log: vmoddomain.c,v $
 * Revision 1.7  2025-05-13 20:37:32+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.6  2024-05-28 21:20:22+05:30  Cprogrammer
 * added -H option to configure hash method for a domain
 *
 * Revision 1.5  2024-05-17 16:24:31+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2023-03-22 09:46:49+05:30  Cprogrammer
 * updated error messages
 *
 * Revision 1.3  2023-01-22 10:32:00+05:30  Cprogrammer
 * reformatted error message strings
 *
 * Revision 1.2  2019-06-07 15:45:51+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:34:04+05:30  Cprogrammer
 * Initial revision
 *
 */
