/*
 * $Log: vuserinfo.c,v $
 * Revision 1.3  2022-10-20 11:59:24+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.2  2019-06-07 15:39:32+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-14 18:30:52+05:30  Cprogrammer
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
#ifdef ENABLE_AUTH_LOGGING
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <sgetopt.h>
#endif
#include "common.h"
#include "userinfo.h"
#include "parse_email.h"
#include "get_indimailuidgid.h"
#include "variables.h"
#include "iclose.h"
#include "check_group.h"

#define WARN    "vuserinfo: warning: "
#define FATAL   "vuserinfo: fatal: "

#ifndef	lint
static char     sccsid[] = "$Id: vuserinfo.c,v 1.3 2022-10-20 11:59:24+05:30 Cprogrammer Exp mbhangui $";
#endif

char           *usage =
	"usage: vuserinfo [options] email_address\n"
	"options: -V (print version number)\n"
	"         -a (display all fields, this is the default)\n"
	"         -n (display name)\n"
	"         -p (display crypted password)\n"
	"         -u (display uid field)\n"
	"         -g (display gid field)\n"
	"         -c (display comment field)\n"
	"         -d (display directory)\n"
	"         -q (display quota field)"
#ifdef ENABLE_AUTH_LOGGING
	"\n"
	"         -l (display usage times)"
#endif
	;

static void
die_nomem()
{
	strerr_warn1("vuserinfo: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	static stralloc Email = {0}, User = {0}, Domain = {0};
	char            opt_str[56];
	int             c;
	int             DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir, DisplayQuota;
	int             DisplayLastAuth, DisplayAll, DisplayFilter, tmpAll;
	char           *s;
	uid_t           uid;
	gid_t           gid;

	DisplayName = DisplayPasswd = DisplayUid = DisplayGid = DisplayComment = DisplayDir = 0;
	DisplayQuota = DisplayLastAuth = DisplayFilter = 0;
	DisplayAll = 1;
	tmpAll = 0;
	s = opt_str;
	s += fmt_strn(s, "anpugcdq", 8);
#ifdef ENABLE_AUTH_LOGGING
	s += fmt_strn(s, "l", 1);
#endif
#ifdef VFILTER
	s += fmt_strn(s, "f", 1);
#endif
	while ((c = getopt(argc, argv, opt_str)) != opteof) {
		switch (c)
		{
		case 'a':
			tmpAll = 1;
			break;
		case 'n':
			DisplayName = 1;
			DisplayAll = 0;
			break;
		case 'p':
			DisplayPasswd = 1;
			DisplayAll = 0;
			break;
		case 'u':
			DisplayUid = 1;
			DisplayAll = 0;
			break;
		case 'g':
			DisplayGid = 1;
			DisplayAll = 0;
			break;
		case 'c':
			DisplayComment = 1;
			DisplayAll = 0;
			break;
		case 'd':
			DisplayDir = 1;
			DisplayAll = 0;
			break;
		case 'q':
			DisplayQuota = 1;
			DisplayAll = 0;
			break;
#ifdef ENABLE_AUTH_LOGGING
		case 'l':
			DisplayLastAuth = 1;
			DisplayAll = 0;
			break;
#endif
		case 'f':
			DisplayFilter = 1;
			DisplayAll = 0;
			break;
		default:
			strerr_die2x(100, WARN, usage);
			break;
		}
	}
	if (tmpAll)
		DisplayAll = 1;
	if (optind < argc) {
		if (!stralloc_copys(&Email, argv[optind++]) || !stralloc_0(&Email))
			die_nomem();
		Email.len--;
	}
	if (!Email.len) {
		strerr_die2x(100, WARN, usage);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strerr_warn1("you must be root or indimail to run this program", 0);
		return (1);
	}
	if (parse_email(Email.s, &User, &Domain))
		die_nomem();
	c = userinfo(Email.s, User.s, Domain.s, DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir,
		DisplayQuota, DisplayLastAuth, DisplayFilter, DisplayAll);
	iclose();
	return (c);
}

