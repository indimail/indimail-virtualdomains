/*
 * $Log: hostcntrl.c,v $
 * Revision 1.5  2022-10-20 11:57:37+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.4  2019-06-07 15:59:31+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.3  2019-04-22 23:10:49+05:30  Cprogrammer
 * replaced atol() with scan_ulong()
 *
 * Revision 1.2  2019-04-14 21:48:53+05:30  Cprogrammer
 * use hostid, email address pair when deleting entries
 *
 * Revision 1.1  2019-04-14 21:34:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: hostcntrl.c,v 1.5 2022-10-20 11:57:37+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <sgetopt.h>
#include <fmt.h>
#include <scan.h>
#include <qprintf.h>
#include <subfd.h>
#endif
#include "variables.h"
#include "parse_email.h"
#include "sql_getip.h"
#include "hostcntrl_select.h"
#include "addusercntrl.h"
#include "updusercntrl.h"
#include "delusercntrl.h"

#define V_USER_SELECT 0
#define V_USER_INSERT 1
#define V_USER_DELETE 2
#define V_USER_UPDATE 3
#define V_USER_DELUSR 4
#define V_SELECT_ALL  5

#define FATAL   "hostcntrl: fatal: "
#define WARN    "hostcntrl: warning: "

static char    *usage =
	"usage: hostcntrl [options] emailid\n"
	"options: -V        ( print version number )\n"
	"         -v        ( verbose )\n"
	"         -l        ( List all hostcntrl entries )\n"
	"         -s        ( show   hostcntrl entry )\n"
	"         -d hostid ( remove hostcntrl entry )\n"
	"         -i hostid ( insert hostcntrl entry )\n"
	"         -m hostid ( modify hostcntrl entry )"
	;

int
get_options(int argc, char **argv, char **email, char **hostid, int *action)
{
	int             c;

	verbose = 0;
	*action = -1;
	*hostid = *email = 0;
	while ((c = getopt(argc, argv, "vlsd:i:m:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'l':
			*action = V_SELECT_ALL;
			break;
		case 's':
			*action = V_USER_SELECT;
			break;
		case 'd':
			*action = V_USER_DELETE;
			*hostid = optarg;
			break;
		case 'i':
			*action = V_USER_INSERT;
			*hostid = optarg;
			break;
		case 'm':
			*action = V_USER_UPDATE;
			*hostid = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return(1);
		}
	}
	if (*action == -1) {
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
	if (*action == V_SELECT_ALL)
		return(0);
	if (optind < argc)
		*email = argv[optind++];
	if (!*email) {
		strerr_warn1("must supply emailid", 0);
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
	if ((*action == V_USER_INSERT || *action == V_USER_UPDATE || *action == V_USER_DELETE) && !*hostid) {
		strerr_warn1("must supply hostid", 0);
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
	return(0);
}

int
main(int argc, char **argv)
{
	int             action, i;
	static stralloc HostID = {0}, user = {0}, domain = {0};
	char            strnum[FMT_ULONG];
	char           *hostid, *emailid, *ipaddr;
	char          **row;
	time_t          tmval;

	if (get_options(argc, argv, &emailid, &hostid, &action))
		return(1);
	if (action != V_SELECT_ALL && action != -1)
		parse_email(emailid, &user, &domain);
	switch (action)
	{
	case V_SELECT_ALL:
		qprintf(subfdoutsmall, "User", "%-20s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, "Domain", "%-20s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, "Host ID", "%-9s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, "IP Address", "%-15s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, "Added On\n", "%s");
		for(;;) {
			if (!(row = hostcntrl_select_all()))
				break;
			ipaddr = ((ipaddr = sql_getip(row[2])) ? ipaddr : "????");
			scan_ulong(row[3], (unsigned long *) &tmval);
			qprintf(subfdoutsmall, row[0], "%-20s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, row[1], "%-20s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, row[2], "%-9s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, ipaddr, "%-15s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, ctime(&tmval), "%s");
		}
		qprintf_flush(subfdoutsmall);
		break;
	case V_USER_SELECT:
		if (!hostcntrl_select(user.s, domain.s, &tmval, &HostID)) {
			ipaddr = ((ipaddr = sql_getip(HostID.s)) ? ipaddr : "????");
			qprintf(subfdoutsmall, "Email", "%-25s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, "HOST ID", "%-11s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, "IP Address", "%-16s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, "Added On\n", "%s");
			qprintf(subfdoutsmall, emailid, "%-25s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, HostID.s, "%-11s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, ipaddr, "%-16s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, ctime(&tmval), "%s");
			qprintf_flush(subfdoutsmall);
			return(0);
		} else
			return (1);
	case V_USER_INSERT:
		switch ((i = addusercntrl(user.s, domain.s, hostid, "manual", 1)))
		{
		case 0:
		case 1:
		default:
			return(i);
		case 2:
			strerr_warn6("hostcntrl: duplicate entry ", user.s, "@", domain.s, " host ", hostid, 0);
			return(1);

		}
		break;
	case V_USER_DELETE: /*- Delete user */
		return (delusercntrl(user.s, domain.s, 1));
	case V_USER_UPDATE:
		return (updusercntrl(user.s, domain.s, hostid, 1));
	default:
		strnum[fmt_uint(strnum, action)] = 0;
		strerr_warn3("hostnctrl: error, Action ", strnum, " is invalid", 0);
		return(1);
	}
	return(0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("hostcntrl: IndiMail not configured with --enable-user-cluster=y", 0);
	return(0);
}
#endif
