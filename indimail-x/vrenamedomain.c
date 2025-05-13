/*
 * $Id: vrenamedomain.c,v 1.8 2025-05-13 20:37:36+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <error.h>
#include <strerr.h>
#include <str.h>
#include <open.h>
#include <getln.h>
#include <env.h>
#include <fmt.h>
#include <replacestr.h>
#include <setuserid.h>
#endif
#include "variables.h"
#include "check_group.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "sql_renamedomain.h"
#include "common.h"
#include "addaliasdomain.h"
#include "deldomain.h"
#include "add_domain_assign.h"
#include "del_domain_assign.h"
#include "add_control.h"
#include "del_control.h"
#include "CreateDomainDirs.h"
#include "is_alias_domain.h"
#include "sql_getall.h"
#include "post_handle.h"

#ifndef	lint
static char     sccsid[] = "$Id: vrenamedomain.c,v 1.8 2025-05-13 20:37:36+05:30 Cprogrammer Exp mbhangui $";
#endif

#define WARN    "vrenamedomain: warning: "
#define FATAL   "vrenamedomain: fatal: "

static void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

static int
getch(char *ch)
{
	int             r;

	if ((r = substdio_get(subfdinsmall, ch, 1)) == -1)
		strerr_die2sys(111, FATAL, "getch: unable to read input: ");
	return (r);
}

int
main(int argc, char **argv)
{
	uid_t           uid, domainuid;
	gid_t           gid, domaingid;
	int             fd, i, match;
	static stralloc OldDir = {0}, NewDir = {0}, TmpDir = {0},
					tmpbuf = {0}, line = {0};
	const char     *real_domain;
	char           *ptr, *tmpstr, *base_argv0;
	struct passwd  *pw;
	struct stat     statbuf;
	char            inbuf[4096], outbuf[512], linkbuf[512],
					strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	struct substdio ssin, ssout;

	if (argc != 3) {
		strerr_warn1("USAGE: vrenamedomain old_domain_name new_domain_name", 0);
		return (1);
	}
	for (ptr = argv[1]; *ptr; ptr++) {
		if (isupper((int) *ptr)) {
			strerr_die4x(100, WARN, "domain [", argv[1], "] has an uppercase character");
		}
	}
	for (ptr = argv[2]; *ptr; ptr++) {
		if (isupper((int) *ptr)) {
			strerr_die4x(100, WARN, "domain [", argv[2], "] has an uppercase character");
		}
	}
	if (!get_assign(argv[1], &OldDir, &domainuid, &domaingid))
		strerr_die3x(1, WARN, argv[1], ": domain does not exist");
	if (!domainuid)
		strerr_die4x(100, WARN, "domain ", argv[1], " with uid 0");
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != domainuid && gid != domaingid && check_group(domaingid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, domainuid)] = 0;
		strnum2[fmt_ulong(strnum2, domaingid)] = 0;
		strerr_die6x(100, WARN, "you must be root or domain user (uid=", strnum1, ", gid=", strnum2, ") to run this program");
	}
	if (uid && setuid(0))
		strerr_die2sys(111, FATAL, "setuid-root: ");
	if ((real_domain = get_real_domain(argv[2])) != (char *) 0) /*- don't rename if domain exists in rcpthosts */
		strerr_die3x(1, WARN, argv[2], ": domain exists");
	else
	if (get_assign(argv[2], &tmpbuf, 0, 0))
		strerr_die3x(1, WARN, argv[2], ": domain exists");
	else
	if (is_alias_domain(argv[1]) == 1) {
		if (!(real_domain = get_real_domain(argv[1])))
			strerr_die3x(1, WARN, argv[1], ": No such domain");
		subprintfe(subfdout, "vrenamedomain", "Renaming alias domain %s (%s) to %s\n", argv[1], real_domain, argv[2]);
		flush("vrenamedomain");
		if (addaliasdomain(real_domain, argv[2]))
			strerr_warn4("addaliasdomain: ", real_domain, " -> ", argv[2], 0);
		else
		if (deldomain(argv[1]))
			strerr_warn2("deldomain: ", argv[1], 0);
		return (0);
	}
	tmpbuf.len = 0;
	if ((i = replacestr(OldDir.s, argv[1], argv[2], &tmpbuf)) == -1)
		die_nomem();
	if (i && rename(OldDir.s, tmpbuf.s))
		strerr_die6sys(1, FATAL, "rename: ", OldDir.s, " -> ", tmpbuf.s, ": ");
	if (!stralloc_copy(&NewDir, &tmpbuf) || !stralloc_0(&NewDir))
		die_nomem();
	NewDir.len--;
	subprintfe(subfdout, "vrenamedomain", "Renaming real domain %s to %s\n", argv[1], argv[2]);
	flush("vrenamedomain");
	if ((tmpstr = str_str(tmpbuf.s, "/domains")) != (char *) 0)
		*tmpstr = 0;
	if (sql_renamedomain(argv[1], argv[2], NewDir.s))
		strerr_die6x(1, WARN, "sql_renamedomain: ", argv[1], " --> ", argv[2], " failed");
	else
	if (add_domain_assign(argv[2], tmpbuf.s, domainuid, domaingid))
		strerr_die3x(1, WARN, "add_domain_assign: ", argv[2]);
	else
	if (add_control(argv[2], 0))
		strerr_die3x(1, WARN, "add_control: ", argv[2]);
	else
	if (del_domain_assign(argv[1], OldDir.s, domainuid, domaingid))
		strerr_die3x(1, WARN, "add_domain_assign: ", argv[1]);
	CreateDomainDirs(argv[2], uid, gid);
	if (tmpstr && !*tmpstr) /*- restore the dir name in tmpbuf */
		*tmpstr = '/';
	if (!stralloc_catb(&tmpbuf, "/.aliasdomains", 14) || !stralloc_0(&tmpbuf))
		die_nomem();
	tmpbuf.len--;
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_warn3("vrenamedomain: open: ", tmpbuf.s, ": ", &strerr_sys);
		return (1);
	}
	if (fd != -1) {
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		subprintfe(subfdout, "vrenamedomain", "Relinking domains aliased to %s\n", argv[1]);
		substdio_flush(subfdout);
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn4(WARN, "read: ", tmpbuf.s, ": ", &strerr_sys);
				close(fd);
				return (1);
			}
			if (!line.len)
				break;
			if (match) {
				line.len--;
				if (!line.len)
					continue;
				line.s[line.len] = 0;
			} else {
				if (!stralloc_0(&line))
					die_nomem();
				line.len--;
			}
			if (!get_assign(line.s, &TmpDir, &uid, &gid)) {
				strerr_warn4(WARN, "Domain ", line.s, " does not exist", 0);
				continue;
			} else
			if (lstat(TmpDir.s, &statbuf)) {
				strerr_warn3(WARN, TmpDir.s, ": lstat: ", &strerr_sys);
				continue;
			} else
			if (!S_ISLNK(statbuf.st_mode)) {
				strerr_warn5(WARN, TmpDir.s, " (", line.s, "): Not an alias domain", 0);
				continue;
			}
			if ((i = readlink(TmpDir.s, linkbuf, sizeof(linkbuf))) == -1) {
				strerr_warn4(WARN, "readlink: ", TmpDir.s, ": ", &strerr_sys);
				return -1;
			}
			if (i == sizeof(linkbuf)) {
				errno = ENAMETOOLONG;
				strerr_warn4(WARN, "readlink: ", TmpDir.s, ": ", &strerr_sys);
				return -1;
			}
			linkbuf[i] = 0;
			if (str_diffn(linkbuf, argv[1], i + 1))
				continue; /*- this shouldn't happen */
			/*-
			 * we have already renamed OldDir
			 * remove the dangling symlink and
			 * relink to the renamed domain
			 */
			if (unlink(TmpDir.s)) {
				strerr_warn3(WARN, TmpDir.s, ": unlink: ", &strerr_sys);
				continue;
			} else
			if (symlink(NewDir.s, TmpDir.s)) {
				char            ch[1];
				strerr_warn6(WARN, "symlink: ", TmpDir.s, " -> ", NewDir.s, ": ", &strerr_sys);
				getch(ch);
				continue;
			} else {
				subprintfe(subfdout, "vrenamedomain", "Linked Domain %s to %s [%s->%s]\n", line.s, argv[2], TmpDir.s, NewDir.s);
				flush("vrenamedomain");
			}
		}
	}
	tmpbuf.len -= 14; /*- 14 = length of /.aliasdomains */
	if (!stralloc_catb(&tmpbuf, "/.domain_rename", 15) || !stralloc_0(&tmpbuf))
		die_nomem();
	close(open(tmpbuf.s, O_TRUNC|O_CREAT, INDIMAIL_QMAIL_MODE));
	for (pw = sql_getall(argv[2], 1, 0); pw; pw = sql_getall(argv[2], 0, 0)) {
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		tmpbuf.len--;
		if (access(tmpbuf.s, F_OK)) {
			if (errno != ENOENT)
				strerr_warn2(tmpbuf.s, ": ", &strerr_sys);
			continue;
		}
		if (!stralloc_catb(&tmpbuf, "/domain", 7) || !stralloc_0(&tmpbuf))
			die_nomem();
		tmpbuf.len--;
		if ((fd = open_trunc(tmpbuf.s)) == -1) {
			strerr_warn4(WARN, "open_trunc: ", tmpbuf.s, ": ", &strerr_sys);
			continue;
		}
		substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
		if (substdio_puts(&ssout, argv[1]) ||
				substdio_put(&ssout, "\n", 1) ||
				substdio_flush(&ssout)) {
			strerr_warn4(WARN, "write: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			continue;
		}
		close(fd);
	}
	/*- delete the old domain from the qmail control files */
	if (del_control(argv[1]) == -1)
		return (-1);
	if (!stralloc_copy(&tmpbuf, &NewDir) ||
			!stralloc_catb(&tmpbuf, "/.domain_rename", 15) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (unlink(tmpbuf.s))
		strerr_warn4(WARN, "unlink: ", tmpbuf.s, ": ", &strerr_sys);
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return(post_handle("%s/%s %s %s", LIBEXECDIR, base_argv0, argv[1], argv[2]));
	} else {
		if (setuser_privileges(domainuid, domaingid, "indimail")) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_die6sys(111, FATAL, "setuser_privilege: (", strnum1, "/", strnum2, "): ");
		}
		return(post_handle("%s %s %s", ptr, argv[1], argv[2]));
	}
}
/*
 * $Log: vrenamedomain.c,v $
 * Revision 1.8  2025-05-13 20:37:36+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.7  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2023-03-23 22:27:49+05:30  Cprogrammer
 * multiple bug fixes
 *
 * Revision 1.5  2023-03-20 10:39:05+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.4  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.3  2021-09-12 20:18:04+05:30  Cprogrammer
 * moved replacestr to libqmail
 *
 * Revision 1.2  2019-06-07 15:43:14+05:30  Cprogrammer
 * removed not needed sgetopt.h include file
 *
 * Revision 1.1  2019-04-18 08:33:38+05:30  Cprogrammer
 * Initial revision
 *
 */
