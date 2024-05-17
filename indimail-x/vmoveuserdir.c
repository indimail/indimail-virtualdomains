/*
 * $Log: vmoveuserdir.c,v $
 * Revision 1.8  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2023-03-23 22:23:13+05:30  Cprogrammer
 * remove domain component from User
 *
 * Revision 1.6  2023-03-22 11:25:32+05:30  Cprogrammer
 * run POST_HANDLE program (if set) with domain user uid/gid
 *
 * Revision 1.5  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.4  2022-08-07 13:09:58+05:30  Cprogrammer
 * updated for scram argument to sql_getpw()
 *
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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <str.h>
#include <env.h>
#include <replacestr.h>
#include <subfd.h>
#include <setuserid.h>
#endif
#include "post_handle.h"
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
static char     sccsid[] = "$Id: vmoveuserdir.c,v 1.8 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

#define FATAL   "vmoveuserdir: fatal: "
#define WARN    "vmoveuserdir: warning: "

static void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

int
main(int argc, char **argv)
{
	struct passwd  *pw;
	struct passwd   PwTmp;
	char           *tmpstr, *Domain, *NewDir, *User, *base_argv0;
	const char     *real_domain;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	static stralloc OldDir = {0}, Dir = {0};
	char           *ptr;
#ifdef VALIAS
	static stralloc tmp_domain = {0};
#endif
	uid_t           uid, domainuid;
	gid_t           gid, domaingid;
	int             i;
#ifdef CLUSTERED_SITE
	int             err;
	static stralloc tmp = {0};
	char           *mailstore;
#endif

	if (argc != 3) {
		strerr_warn1("usage: vmoveuserdir user new_dir", 0);
		return (1);
	}
	for (ptr = argv[1]; *ptr; ptr++) {
		if (isupper(*ptr))
			strerr_die4x(100, WARN, "email [", argv[1], "] has an uppercase character");
	}
	User = argv[1];
	NewDir = argv[2];
	i = str_chr(User, '@');
	if (User[i]) {
		User[i] = 0;
		Domain = User + i + 1;
	} else
		strerr_die3x(1, WARN, User, ": Not a fully qualified email");
	if (!(real_domain = get_real_domain(Domain)))
		strerr_die3x(1, WARN, Domain, ": No such domain");
	else
	if (!get_assign(real_domain, &Dir, &domainuid, &domaingid))
		strerr_die4x(1, WARN, "domain ", Domain, " does not exist");
	if (!domainuid)
		strerr_die4x(100, WARN, "domain ", real_domain, " with uid 0");
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != domainuid && gid != domaingid && check_group(domaingid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, domainuid)] = 0;
		strnum2[fmt_ulong(strnum2, domaingid)] = 0;
		strerr_die5x(100, "vmoduser: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program");
	}
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1)
		strerr_die4x(1, WARN, "Unable to verify ", real_domain, " as a distributed domain");
	else
	if (err == 1) {
		if (open_master())
			strerr_die2x(1, FATAL, "failed to open master db");
		if (!stralloc_copys(&tmp, User) ||
				!stralloc_append(&tmp, "@") ||
				!stralloc_cats(&tmp, real_domain))
			die_nomem();
		if ((mailstore = findhost(tmp.s, 0)) != (char *) 0) {
			i = str_rchr(mailstore, ':');
			if (mailstore[i])
				mailstore[i] = 0;
			else
				strerr_die7x(1, WARN, "invalid mailstore [", mailstore, "] for ", User, "@", real_domain);
			for(; *mailstore && *mailstore != ':'; mailstore++);
			if  (*mailstore != ':')
				strerr_die7x(1, WARN, "invalid mailstore [", mailstore, "] for ", User, "@", real_domain);
			mailstore++;
		} else {
			if (userNotFound)
				strerr_die5x(1, WARN, User, "@", real_domain, ": No such user");
			else
				strerr_die2x(1, FATAL, "error connecting to db");
		}
		if (!islocalif(mailstore))
			strerr_die7x(1, WARN, User, "@", real_domain, " not local (mailstore ", mailstore, ")");
	}
#endif
	if (!(pw = sql_getpw(User, real_domain)))
		strerr_die5x(1, WARN, "sql_getpw: No such user ", User, "@", real_domain);
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
	if (uid && setuid(0))
		strerr_die2sys(111, FATAL, "setuid-root: ");
	if (!access(OldDir.s, F_OK) && MoveFile(OldDir.s, NewDir))
		strerr_die6x(111, FATAL, "MoveFile: ", OldDir.s, " --> ", pw->pw_dir, ": ");
	/*- update database */
	if ((i = sql_setpw(pw, Domain, NULL))) {
		strerr_warn2(WARN, "sql_setpw failed", 0);
		if (!access(NewDir, F_OK) && MoveFile(NewDir, OldDir.s))
			strerr_die6x(111, FATAL, "MoveFile: ", NewDir, " --> ", OldDir.s, ": ");
		return (1);
	}
	subprintfe(subfdout, "vmoveuserdir", "%s@%s old %s new %s done\n", User, Domain, OldDir.s, NewDir);
	flush("vmoveuserdir");
	if (!(tmpstr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s@%s %s %s", LIBEXECDIR, base_argv0, User, real_domain, OldDir.s, NewDir));
	} else {
		if (setuser_privileges(uid, gid, "indimail")) {
			strnum1[fmt_ulong(strnum1, domainuid)] = 0;
			strnum2[fmt_ulong(strnum2, domaingid)] = 0;
			strerr_die6sys(111, FATAL, "setuser_privilege: (", strnum1, "/", strnum2, "): ");
		}
		return (post_handle("%s %s@%s %s %s", tmpstr, User, real_domain, OldDir.s, NewDir));
	}
}
