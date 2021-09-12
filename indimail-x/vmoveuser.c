/*
 * $Log: vmoveuser.c,v $
 * Revision 1.3  2021-09-12 20:18:01+05:30  Cprogrammer
 * moved replacestr to libqmail
 *
 * Revision 1.2  2021-07-08 11:49:41+05:30  Cprogrammer
 * add check for misconfigured assign file
 *
 * Revision 1.1  2019-04-15 11:58:23+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <str.h>
#include <env.h>
#include <replacestr.h>
#endif
#include "post_handle.h"
#include "get_indimailuidgid.h"
#include "variables.h"
#include "check_group.h"
#include "get_assign.h"
#include "get_real_domain.h"
#include "open_master.h"
#include "is_distributed_domain.h"
#include "findhost.h"
#include "islocalif.h"
#include "sql_getpw.h"
#include "sql_setpw.h"
#include "valias_select.h"
#include "valias_update.h"
#include "MoveFile.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: vmoveuser.c,v 1.3 2021-09-12 20:18:01+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vmoveuser: fatal: "
#define WARN    "vmoveuser: warning: "

static void
die_nomem()
{
	strerr_warn1("vmoveuser: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	struct passwd  *pw;
	struct passwd   PwTmp;
	char           *tmpstr, *Domain, *NewDir, *User, *real_domain, *base_argv0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	static stralloc OldDir = {0}, Dir = {0};
#if defined(CLUSTERED_SITE) || defined(VALIAS)
	char           *ptr;
#endif
#ifdef VALIAS
	static stralloc tmp_domain = {0};
#endif
	uid_t           uid;
	gid_t           gid;
	int             i;
#ifdef CLUSTERED_SITE
	int             err;
	static stralloc tmp = {0};
	char           *mailstore;
#endif

	if (argc != 3) {
		strerr_warn1("usage: vmoveuser user new_dir", 0);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
		strerr_warn5("vadduser: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	User = argv[1];
	NewDir = argv[2];
	i = str_chr(User, '@');
	if (User[i])
		Domain = User + i + 1;
	else {
		strerr_warn3("vmoveuser: ", User, ": Not a fully qualified email", 0);
		return (1);
	}
	if (!(real_domain = get_real_domain(Domain))) {
		strerr_warn3("vmoveuser: domain ", Domain, " does not exist", 0);
		return (1);
	} else
	if (!get_assign(real_domain, &Dir, &uid, &gid)) {
		strerr_warn3("vmoveuser: domain ", real_domain, " does not exist", 0);
		return (1);
	}
	if (!uid)
		strerr_die3x(100, "vmovuser: domain ", real_domain, " with uid 0");
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1) {
		strerr_warn3("vmoveuser: unable to verify ", real_domain, " as a distributed domain", 0);
		return (1);
	} else
	if (err == 1) {
		if (open_master()) {
			strerr_warn1("vmoveuser: failed to open master db", 0);
			return (1);
		}
		if (!stralloc_copys(&tmp, User) ||
				!stralloc_append(&tmp, "@") ||
				!stralloc_cats(&tmp, real_domain))
			die_nomem();
		if ((mailstore = findhost(tmp.s, 0)) != (char *) 0) {
			i = str_rchr(mailstore, ':');
			if (mailstore[i])
				mailstore[i] = 0;
			else {
				strerr_warn6("vmoveuser: invalid mailstore [", mailstore, "] for ", User, "@", real_domain, 0);
				return (1);
			}
			for(; *mailstore && *mailstore != ':'; mailstore++);
			if  (*mailstore != ':') {
				strerr_warn6("vmoveuser: invalid mailstore [", mailstore, "] for ", User, "@", real_domain, 0);
				return (1);
			}
			mailstore++;
		} else {
			if (userNotFound)
				strerr_warn4(User, "@", real_domain, ": No such user", 0);
			else
				strerr_warn1("vmoveuser: error connecting to db", 0);
			return (1);
		}
		if (!islocalif(mailstore)) {
			strerr_warn6(User, "@", real_domain, " not local (mailstore ", mailstore, ")", 0);
			return (1);
		}
	}
#endif
	if (!(pw = sql_getpw(User, real_domain))) {
		strerr_warn4(User, "@", real_domain, ": No such user", 0);
		return (1);
	}
	PwTmp = *pw;
	pw = &PwTmp;
	if (!stralloc_copys(&OldDir, pw->pw_dir) ||
			!stralloc_0(&OldDir))
		die_nomem();
	OldDir.len--;
	pw->pw_dir = NewDir;
#ifdef VALIAS
	/*- Replace all occurence of OldDir with NewDir */
	for(;;) {
		tmp_domain.len = 0;
		if (!(ptr = valias_select_all(0, &tmp_domain)))
			break;
		tmp.len = 0;
		if ((i = replacestr(ptr, OldDir.s, NewDir, &tmp)) == -1)
			continue;
		if (!i)
			continue;
		valias_update(User, tmp_domain.s, ptr, tmp.s);
	}
#endif
	/*- move directory */
	if (setuid(0)) {
		strerr_warn1("vmoveuser: setuid-root: ", &strerr_sys);
		return (1);
	}
	if (!access(OldDir.s, F_OK) && MoveFile(OldDir.s, NewDir)) {
		strerr_warn5("vmoveuser: MoveFile: ", OldDir.s, " --> ", pw->pw_dir, ": ", &strerr_sys);
		return (1);
	}
	/*- update database */
	if ((i = sql_setpw(pw, Domain))) {
		strerr_warn1("vmoveuser: sql_setpw failed", 0);
		if (!access(NewDir, F_OK) && MoveFile(NewDir, OldDir.s))
			strerr_warn5("vmoveuser: MoveFile: ", NewDir, " --> ", OldDir.s, ": ", &strerr_sys);
		return (1);
	}
	out("vmoveuser", User);
	out("vmoveuser", "@");
	out("vmoveuser", Domain);
	out("vmoveuser", " old ");
	out("vmoveuser", OldDir.s);
	out("vmoveuser", " new ");
	out("vmoveuser", NewDir);
	out("vmoveuser", " done\n");
	flush("vmoveuser");
	if (!(tmpstr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s@%s %s %s", LIBEXECDIR, base_argv0, User, real_domain, OldDir.s, NewDir));
	} else
		return (post_handle("%s %s@%s %s %s", tmpstr, User, real_domain, OldDir, NewDir));
}
