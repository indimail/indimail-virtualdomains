/*
 * $Log: vaddaliasdomain.c,v $
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
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <fmt.h>
#include <env.h>
#include <str.h>
#include <sgetopt.h>
#endif
#include "get_indimailuidgid.h"
#include "check_group.h"
#include "iclose.h"
#include "addaliasdomain.h"
#include "setuserid.h"
#include "post_handle.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: vaddaliasdomain.c,v 1.2 2019-06-07 15:56:08+05:30 mbhangui Exp mbhangui $";
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
	return (0);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	char           *ptr, *domain_old, *domain_new, *base_argv0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             i, err;
	uid_t           uid;
	gid_t           gid;

	if(get_options(argc, argv, &domain_new, &domain_old))
		return (1);
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
		strerr_warn5("vaddaliasdomain: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	if (setuser_privileges(uid, gid, "indimail")) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vaddaliasdomain: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
		return (1);
	}
	if((err = addaliasdomain(domain_old, domain_new))) {
		strerr_warn4("vaddaliasdomain: failed to create alias domain ", domain_new, " for domain ", domain_old, 0);
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
	} else
		return (post_handle("%s %s %s", ptr, domain_new, domain_old));
	return (err);
}
