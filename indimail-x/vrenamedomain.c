/*
 * $Log: vrenamedomain.c,v $
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
#include <replacestr.h>
#endif
#include "get_indimailuidgid.h"
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
#include "CreateDomainDirs.h"
#include "is_alias_domain.h"
#include "sql_getall.h"
#include "post_handle.h"

#ifndef	lint
static char     sccsid[] = "$Id: vrenamedomain.c,v 1.3 2021-09-12 20:18:04+05:30 Cprogrammer Exp mbhangui $";
#endif

#define WARN    "vrenamedomain: warning: "
#define FATAL   "vrenamedomain: fatal: "

static void
die_nomem()
{
	strerr_warn1("vrenamedomain: out of memory", 0);
	_exit(111);
}

static int
getch(char *ch)
{
	int             r;

	if ((r = substdio_get(subfdinsmall, ch, 1)) == -1)
		strerr_die1sys(111, "vrenamedomain: getch: unable to read input: ");
	return (r);
}

int
main(int argc, char **argv)
{
	uid_t           uid;
	gid_t           gid;
	int             fd, i, match;
	static stralloc OldDir = {0}, NewDir = {0}, tmpbuf = {0}, line = {0};
	char           *real_domain, *ptr, *tmpstr, *base_argv0;
	struct passwd  *pw;
	struct stat     statbuf;
	char            inbuf[4096], outbuf[512];
	struct substdio ssin, ssout;

	if (argc != 3) {
		strerr_warn1("USAGE: vrenamedomain old_domain_name new_domain_name", 0);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1)
		strerr_die1x(100, "you must be root or indimail to run this program");
	if (uid && setuid(0))
		strerr_die2sys(111, FATAL, "setuid: ");
	if ((real_domain = get_real_domain(argv[2])) != (char *) 0) {
		strerr_warn5("vrenamedomain: domain ", argv[2], " exists [", real_domain, "]", 0);
		return (1);
	} else
	if (get_assign(argv[2], &tmpbuf, &uid, &gid)) {
		strerr_warn2(argv[2], ": domain exists", 0);
		return (1);
	} else
	if (is_alias_domain(argv[1]) == 1) {
		if (!(real_domain = get_real_domain(argv[1]))) {
			strerr_warn2(argv[1], ": domain does not exist", 0);
			return (1);
		} 
		out("vrenamedomain", "Renaming alias domain ");
		out("vrenamedomain", argv[1]); 
		out("vrenamedomain", " (");
		out("vrenamedomain", real_domain); 
		out("vrenamedomain", ") to ");
		out("vrenamedomain", argv[2]); 
		out("vrenamedomain", "\n");
		flush("vrenamedomain");
		if (addaliasdomain(real_domain, argv[2]))
			strerr_warn4("vaddaliasdomain: ", real_domain, " -> ", argv[2], 0);
		else
		if (deldomain(argv[1]))
			strerr_warn2("vdeldomain: ", argv[1], 0);
		return (0);
	} else
	if (!get_assign(argv[1], &OldDir, &uid, &gid)) {
		strerr_warn2(argv[1], ": failed to read assign file", 0);
		return (1);
	}
	tmpbuf.len = 0;
	if ((i = replacestr(OldDir.s, argv[1], argv[2], &tmpbuf)) == -1)
		die_nomem();
	if (i && rename(OldDir.s, tmpbuf.s)) {
		strerr_warn4("vrenamedomain: rename: ", OldDir.s, " -> ", tmpbuf.s, &strerr_sys);
		return (1);
	}
	if (!stralloc_copy(&NewDir, &tmpbuf) || !stralloc_0(&NewDir))
		die_nomem();
	NewDir.len--;
	out("vrenamedomain", "Renaming real domain ");
	out("vrenamedomain", argv[1]);
	out("vrenamedomain", " to ");
	out("vrenamedomain", argv[2]);
	out("vrenamedomain", "\n");
	flush("vrenamedomain");
	if ((tmpstr = str_str(tmpbuf.s, "/domains")) != (char *) 0)
		*tmpstr = 0;
	if (sql_renamedomain(argv[1], argv[2], NewDir.s))
		return (1);
	else
	if (add_domain_assign(argv[2], tmpbuf.s, uid, gid))
		return (1);
	else
	if (add_control(argv[2], 0))
		return (1);
	else
	if (del_domain_assign(argv[1], OldDir.s, uid, gid))
		return (1);
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
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		out("vrenamedomain", "Relinking domains aliased to ");
		out("vrenamedomain", argv[1]);
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("vrenamedomain: read: ", tmpbuf.s, ": ", &strerr_sys);
				close(fd);
				return (1);
			}
			if (line.len == 0)
				break;
			if (match) {
				line.len--;
				line.s[line.len] = 0;
			} else {
				if (!stralloc_0(&line))
					die_nomem();
				line.len--;
				if (!is_alias_domain(line.s)) {
					strerr_warn2(line.s, ": Not an alias domain", 0);
					continue;
				} else
				if (!get_assign(line.s, &OldDir, &uid, &gid)) {
					strerr_warn3("Domain ", line.s, " does not exist", 0);
					continue;
				} else
				if (lstat(OldDir.s, &statbuf)) {
					strerr_warn2(OldDir.s, ": lstat: ", &strerr_sys);
					continue;
				} else
				if (!S_ISLNK(statbuf.st_mode)) {
					strerr_warn4(OldDir.s, " (", line.s, "): Not an alias domain", 0);
					continue;
				} else
				if (unlink(OldDir.s)) {
					strerr_warn2(OldDir.s, ": unlink: ", &strerr_sys);
					continue;
				}
				if (symlink(NewDir.s, OldDir.s)) {
					char            ch[1];
					strerr_warn5("vrenamedomain: symlink: ", OldDir.s, " -> ", NewDir.s, ": ", &strerr_sys);
					getch(ch);
					continue;
				}
				out("vrenamedomain", "Linked Domain ");
				out("vrenamedomain", line.s);
				out("vrenamedomain", " to ");
				out("vrenamedomain", argv[2]);
				out("vrenamedomain", " [");
				out("vrenamedomain", OldDir.s);
				out("vrenamedomain", "->");
				out("vrenamedomain", NewDir.s);
				out("vrenamedomain", "]\n");
				flush("vrenamedomain");
			}
		}
	}
	tmpbuf.len -= 14;
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
			strerr_warn3("vrenamedomain: open_trunc: ", tmpbuf.s, ": ", &strerr_sys);
			continue;
		}
		substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
		if (substdio_puts(&ssout, argv[1]) ||
				substdio_put(&ssout, "\n", 1) ||
				substdio_flush(&ssout)) {
			strerr_warn3("vrenamedomain: write: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			continue;
		}
		close(fd);
	}
	if (stralloc_copy(&tmpbuf, &NewDir) ||
			!stralloc_catb(&tmpbuf, "/.domain_rename", 15) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (unlink(tmpbuf.s))
		strerr_warn3("vrenamedomain: unlink: ", tmpbuf.s, ": ", &strerr_sys);
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return(post_handle("%s/%s %s %s", LIBEXECDIR, base_argv0, argv[1], argv[2]));
	} else
		return(post_handle("%s %s %s", ptr, argv[1], argv[2]));
}
