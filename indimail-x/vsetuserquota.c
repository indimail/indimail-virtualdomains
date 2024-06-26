/*
 * $Log: vsetuserquota.c,v $
 * Revision 1.7  2024-05-28 19:37:57+05:30  Cprogrammer
 * handle -1 return code for vget_limits()
 * removed check for perms_defaultquota
 *
 * Revision 1.6  2024-05-27 22:54:09+05:30  Cprogrammer
 * initialize struct vlimits
 *
 * Revision 1.5  2024-05-24 14:52:01+05:30  Cprogrammer
 * print current usage in bytes on stdout
 *
 * Revision 1.4  2024-05-17 16:24:31+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2022-10-20 11:59:18+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.2  2019-06-07 15:40:53+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-14 18:28:07+05:30  Cprogrammer
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
#ifdef HAVE_SYS_TYPES_H
#include <sys/stat.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <sgetopt.h>
#include <qprintf.h>
#include <subfd.h>
#endif
#include "get_indimailuidgid.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "sqlOpen_user.h"
#include "get_assign.h"
#include "sql_getpw.h"
#include "check_group.h"
#include "iopen.h"
#include "iclose.h"
#include "variables.h"
#include "vlimits.h"
#include "setuserquota.h"

#ifndef	lint
static char     sccsid[] = "$Id: vsetuserquota.c,v 1.7 2024-05-28 19:37:57+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vsetuserquota: fatal: "
#define WARN    "vsetuserquota: warning: "

static char    *usage =
	"usage: vsetuserquota [options] email_address quota_in_bytes\n"
	"options:\n"
	"-V (print version number)\n"
	"-v (verbose)"
	;

int
get_options(int argc, char **argv, char **email, char **quota)
{
	int             c;
	char           *ptr;

	*email = *quota = 0;
	while ((c = getopt(argc, argv, "v")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc) {
		for (ptr = argv[optind]; *ptr; ptr++) {
			if (isupper(*ptr))
				strerr_die4x(100, WARN, "email [", argv[optind], "] has an uppercase character");
		}
		*email = argv[optind++];
	}
	if (optind < argc)
		*quota = argv[optind++];
	if (!*email || !*quota) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vsetuserquota: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	mdir_t          i;
	uid_t           uid;
	gid_t           gid;
	char           *email, *quota;
	const char     *real_domain;
	struct passwd  *pw;
	static stralloc User = {0}, Domain = {0};
#ifdef ENABLE_DOMAIN_LIMITS
	static stralloc tmpbuf = {0}, TheDir = {0};
	int             domain_limits;
	struct vlimits  limits = { 0 };
#endif

	if (get_options(argc, argv, &email, &quota))
		return (1);
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1)
		strerr_die1x(100, "you must be root or indimail to run this program");
	if (uid && setuid(0))
		strerr_die2sys(111, FATAL, "setuid: ");
	parse_email(email, &User, &Domain);
	if (!(real_domain = get_real_domain(Domain.s))) {
		strerr_warn2(Domain.s, ": No such domain", 0);
		return (1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (!get_assign(real_domain, &TheDir, 0, 0)) {
		strerr_warn2(real_domain, ": domain does not exist", 0);
		return (1);
	}
	if (!stralloc_copy(&tmpbuf, &TheDir) ||
			!stralloc_catb(&tmpbuf, "/.domain_limits", 15) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	domain_limits = !access(tmpbuf.s, F_OK);
#endif
#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email, 0))
#else
	if (iopen((char *) 0))
#endif
	{
		if (userNotFound) {
			strerr_warn2(email, ": No such user\n", 0);
			return (1);
		}
		strerr_warn1("temporary database error", 0);
		return (1);
	}
	if (!(pw = sql_getpw(User.s, Domain.s))) {
		strerr_warn2(email, ": No such user\n", 0);
		iclose();
		return (1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (!(pw->pw_gid & V_OVERRIDE) && domain_limits) {
		if (vget_limits(real_domain, &limits) == -1) {
			strerr_warn2("Unable to get domain limits for ", real_domain, 0);
			iclose();
			return (1);
		}
	}
#endif
	i = setuserquota(User.s, real_domain, quota);
	iclose();
	subprintf(subfdout, "usage %ld bytes\n", i);
	substdio_flush(subfdout);
	return (i < 0 ? 1 : 0);
}
