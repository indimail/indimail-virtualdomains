/*
 * $Log: vaddaliasdomain.c,v $
 * Revision 1.6  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-03-22 08:16:59+05:30  Cprogrammer
 * run POST_HANDLE program (if set) with domain user uid/gid
 *
 * Revision 1.4  2022-10-20 11:58:22+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.3  2020-06-16 17:56:01+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.2  2019-06-07 15:56:08+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-14 21:01:29+05:30  Cprogrammer
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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <fmt.h>
#include <env.h>
#include <str.h>
#include <stralloc.h>
#include <sgetopt.h>
#include <setuserid.h>
#endif
#include "check_group.h"
#include "iclose.h"
#include "addaliasdomain.h"
#include "post_handle.h"
#include "get_assign.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: vaddaliasdomain.c,v 1.6 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

#define FATAL   "vaddaliasdomain: fatal: "
#define WARN    "vaddaliasdomain: warning: "


static char    *usage =
	"usage: vaddaliasdomain [options] new_domain old_domain\n"
	"options: -V (print version number)\n"
	"options: -v (verbose)"
	;

int
get_options(int argc, char **argv, char **domain_new, char **domain_old)
{
	int             c;
	char          *ptr;

	*domain_old = *domain_new = 0;
	while ((c = getopt(argc, argv, "v")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (100);
		}
	}
	if (optind < argc)
		*domain_new = argv[optind++];
	if (optind < argc)
		*domain_old = argv[optind++];
	if (!*domain_new || !*domain_old || !**domain_new || !**domain_old) {
		strerr_warn2(WARN, usage, 0);
		return (100);
	}
	for (ptr = *domain_old; *ptr; ptr++) {
		if (isupper((int) *ptr)) {
			strerr_die4x(100, WARN, "domain [", *domain_old, "] has an uppercase character");
		}
	}
	for (ptr = *domain_new; *ptr; ptr++) {
		if (isupper((int) *ptr)) {
			strerr_die4x(100, WARN, "domain [", *domain_new, "] has an uppercase character");
		}
	}
	return (0);
}

int
main(int argc, char **argv)
{
	char           *ptr, *domain_old, *domain_new, *base_argv0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             i, err;
	uid_t           uid, domainuid;
	gid_t           gid, domaingid;

	if(get_options(argc, argv, &domain_new, &domain_old))
		return (1);
	if (!get_assign(domain_old, 0, &domainuid, &domaingid)) {
		strerr_warn3(WARN, domain_old, ": domain does not exist", 0);
		return (-1);
	}
	if (!domainuid)
		strerr_die4x(100, WARN, "domain ", domain_old, " with uid 0");
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != domainuid && gid != domaingid && check_group(domaingid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, domainuid)] = 0;
		strnum2[fmt_ulong(strnum2, domaingid)] = 0;
		strerr_die6x(100, WARN, "you must be root or domain user (uid=", strnum1, ", gid=", strnum2, ") to run this program");
	}
	if((err = addaliasdomain(domain_old, domain_new))) {
		strerr_warn5(FATAL, "failed to create alias domain ", domain_new, " for domain ", domain_old, 0);
		return (1);
	}
	iclose();
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s %s", LIBEXECDIR, base_argv0, domain_new, domain_old));
	} else {
		if (setuser_privileges(uid, gid, "indimail")) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_die6sys(111, FATAL, "setuser_privilege: (", strnum1, "/", strnum2, "): ");
		}
		return (post_handle("%s %s %s", ptr, domain_new, domain_old));
	}
	return (err);
}
