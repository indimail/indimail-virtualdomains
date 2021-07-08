/*
 * $Log: vdeluser.c,v $
 * Revision 1.4  2021-07-08 11:49:05+05:30  Cprogrammer
 * add check for misconfigured assign file
 *
 * Revision 1.3  2020-06-16 17:56:32+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.2  2019-06-07 15:53:23+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:15:41+05:30  Cprogrammer
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <sgetopt.h>
#include <strerr.h>
#include <env.h>
#include <str.h>
#include <fmt.h>
#include <setuserid.h>
#endif
#include "parse_email.h"
#include "get_assign.h"
#include "variables.h"
#include "iclose.h"
#include "deluser.h"
#include "post_handle.h"
#include "check_group.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdeluser.c,v 1.4 2021-07-08 11:49:05+05:30 Cprogrammer Exp mbhangui $";
#endif

#define WARN    "vdeluser: warning: "
#define FATAL   "vdeluser: fatal: "

char           *usage =
	"usage: vdeluser [options] email_address\n"
	"options: -V (print version number)\n"
	"options: -v (verbose)"
	;

static void
die_nomem()
{
	strerr_warn1("vdeluser: out of memory", 0);
	_exit(111);
}

int
get_options(int argc, char **argv, stralloc *email)
{
	int             c;

	while ((c = getopt(argc, argv, "v")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_die3x(100, WARN, "\n", usage);
			break;
		}
	}

	if (optind < argc) {
		if (!stralloc_copys(email, argv[optind++]) ||
				!stralloc_0(email))
			die_nomem();
		email->len--;
	}
	if (!email->len) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return(0);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             i;
	char           *ptr, *base_argv0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	uid_t           uid, uidtmp;
	gid_t           gid, gidtmp;
	static stralloc email = {0}, user = {0}, domain = {0};

	if (get_options(argc, argv, &email))
		return (1);
	parse_email(email.s, &user, &domain);
	if (!get_assign(domain.s, 0, &uid, &gid)) {
		strerr_warn2(domain.s, ": domain does not exist", 0);
		return (-1);
	}
	if (!uid)
		strerr_die3x(100, "vdeluser: domain ", domain.s, " with uid 0");
	uidtmp = getuid();
	gidtmp = getgid();
	if (uidtmp != 0 && uidtmp != uid && gidtmp != gid && check_group(gid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vdeluser: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	if (setuser_privileges(uid, gid, "indimail")) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vdeluser: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
		return (1);
	}
	if ((i = deluser(user.s, domain.s, 1))) {
		iclose();
		return (i);
	}
	iclose();
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s@%s", LIBEXECDIR, base_argv0, user.s, domain.s));
	} else
		return (post_handle("%s %s@%s", ptr, user.s, domain.s));
}
