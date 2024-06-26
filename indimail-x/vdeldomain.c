/*
 * $Log: vdeldomain.c,v $
 * Revision 1.8  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2023-12-03 15:44:34+05:30  Cprogrammer
 * use entry for autoturn in users/assign for ETRN, ATRN domains
 *
 * Revision 1.6  2023-03-22 10:42:23+05:30  Cprogrammer
 * run POST_HANDLE program (if set) with domain user uid/gid
 *
 * Revision 1.5  2023-01-22 10:32:24+05:30  Cprogrammer
 * fixed incorrectly passed stralloc * instead of char *
 *
 * Revision 1.4  2022-10-20 11:58:39+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.3  2020-04-01 18:58:35+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 15:54:18+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:14:54+05:30  Cprogrammer
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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <sgetopt.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <env.h>
#include <error.h>
#include <getEnvConfig.h>
#include <setuserid.h>
#endif
#include "variables.h"
#include "iclose.h"
#include "check_group.h"
#include "post_handle.h"
#include "is_distributed_domain.h"
#include "is_alias_domain.h"
#include "get_local_ip.h"
#include "deldomain.h"
#include "dbinfoDel.h"
#include "LoadDbInfo.h"
#include "post_handle.h"
#include "get_assign.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdeldomain.c,v 1.8 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static char    *usage =
	"usage: vdeldomain [options] domain_name\n"
	"options: -c (Remove MCD Information)\n"
	"options: -T (Remove Autoturn Domain)\n"
	"options: -V (print version number)\n"
	"options: -v (verbose)"
	;
#define WARN    "vdeldomain: warning: "
#define FATAL   "vdeldomain: fatal: "

static void
die_nomem()
{
	strerr_warn2(FATAL, "out of memory", 0);
	_exit(111);
}

int
get_options(int argc, char **argv, stralloc *Domain, int *mcd_remove)
{
	int             c;
	char           *ptr;

	while ((c = getopt(argc, argv, "cTv")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'T':
			use_etrn = 1;
			break;
		case 'c':
			*mcd_remove = 1;
			break;
		default:
			strerr_die3x(100, WARN, "\n", usage);
			break;
		}
	}
	if (optind < argc) {
		for (ptr = argv[optind]; *ptr; ptr++) {
			if (isupper(*ptr))
				strerr_die4x(100, WARN, "domain [", argv[optind], "] has an uppercase character");
		}
		if (!stralloc_copys(Domain, argv[optind]) ||
				!stralloc_0(Domain))
			die_nomem();
		++optind;
	}
	if (!Domain->len)
		strerr_die3x(100, WARN, "Domain not specified\n", usage);
	return (0);
}

int
main(int argc, char **argv)
{
	int             i, err, mcd_remove = 0;
	uid_t           uid, domainuid;
	gid_t           gid, domaingid;
	char           *ptr, *base_argv0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	static stralloc Domain = {0};
#ifdef CLUSTERED_SITE
	static stralloc mcdFile = {0};
	char           *ipaddr, *mcdfile, *sysconfdir, *controldir;
	int             total, is_dist;
#endif

	if (get_options(argc, argv, &Domain, &mcd_remove))
		return (0);
	if (use_etrn)
		ptr = get_assign("autoturn", 0, &domainuid, &domaingid);
	else
		ptr = get_assign(Domain.s, 0, &domainuid, &domaingid);
	if (!ptr)
		strerr_die4x(1, WARN, use_etrn ? "autoturn domain " : "domain ", Domain.s, " does not exist");
	if (!domainuid)
		strerr_die4x(100, WARN, "domain ", Domain.s, " with uid 0");
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != domainuid && gid != domaingid && check_group(domaingid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, domainuid)] = 0;
		strnum2[fmt_ulong(strnum2, domaingid)] = 0;
		strerr_die6x(100, WARN, "you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program");
	}
	if (uid && setuid(0))
		strerr_die2sys(111, FATAL, "setuid: ");
	if ((err = deldomain(Domain.s)))
		return (err);
#ifdef CLUSTERED_SITE
	if (is_alias_domain(Domain.s) == 1) {
		iclose();
		return (1);
	}
	if ((is_dist = is_distributed_domain(Domain.s)) == -1) {
		strerr_warn4(WARN, "Unable to verify ", Domain.s, " as a distributed domain", 0);
		iclose();
		return (1);
	} else
	if (is_dist || mcd_remove) {
		if (!(ipaddr = get_local_ip(PF_INET))) {
			strerr_warn2(FATAL, "failed to get local ipaddr: ", 0);
			iclose();
			return (1);
		}
		if (dbinfoDel(Domain.s, ipaddr)) {
			strerr_warn5(FATAL, "failed to get remove dbinfo entry for ", Domain.s, "@", ipaddr, 0);
			iclose();
			return (1);
		}
		getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
		if (*mcdfile == '/') {
			if (!stralloc_copys(&mcdFile, mcdfile) ||
					!stralloc_0(&mcdFile))
				die_nomem();
		} else {
			getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
			getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
			if (*controldir == '/') {
				if (!stralloc_copys(&mcdFile, controldir) ||
						!stralloc_append(&mcdFile, "/") ||
						!stralloc_cats(&mcdFile, mcdfile) ||
						!stralloc_0(&mcdFile))
					die_nomem();
			} else {
				if (!stralloc_copys(&mcdFile, sysconfdir) ||
						!stralloc_append(&mcdFile, "/") ||
						!stralloc_cats(&mcdFile, controldir) ||
						!stralloc_append(&mcdFile, "/") ||
						!stralloc_cats(&mcdFile, mcdfile) ||
						!stralloc_0(&mcdFile))
					die_nomem();
			}
		}
		if (!access(mcdFile.s, F_OK) && unlink(mcdFile.s)) {
			strerr_warn4(FATAL, "unlink: ", mcdFile.s, ": ", &strerr_sys);
			iclose();
		}
		LoadDbInfo_TXT(&total);
	}
#endif
	iclose();
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s", LIBEXECDIR, base_argv0, Domain.s));
	} else {
		if (setuser_privileges(domainuid, domaingid, "indimail")) {
			strnum1[fmt_ulong(strnum1, domainuid)] = 0;
			strnum2[fmt_ulong(strnum2, domaingid)] = 0;
			strerr_die6sys(111, FATAL, "setuser_privilege: (", strnum1, "/", strnum2, "): ");
		}
		return (post_handle("%s %s", ptr, Domain.s));
	}
}
