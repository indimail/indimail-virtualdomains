/*
 * $Log: vhostid.c,v $
 * Revision 1.2  2019-04-22 23:19:34+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-13 18:21:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vhostid.c,v 1.2 2019-04-22 23:19:34+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <fmt.h>
#include <strerr.h>
#include <qprintf.h>
#include <subfd.h>
#endif
#include "sql_getip.h"
#include "vhostid_select.h"
#include "vhostid_insert.h"
#include "vhostid_update.h"
#include "vhostid_delete.h"
#include "get_local_hostid.h"
#include "update_local_hostid.h"
#include "variables.h"

#define FATAL   "vhostid: fatal: "
#define WARN    "vhostid: warning: "

#define HOST_SELECT 0
#define HOST_INSERT 1
#define HOST_DELETE 2
#define HOST_UPDATE 3
#define HOST_LOCAL  4

static char    *usage =
	"usage: vhostid [options] [HostId]\n"
	"options: -V        (print version number)\n"
	"         -v        (verbose)\n"
	"         -S        (print local HostID)\n"
	"         -s        (show HostId Mappings)\n"
	"         -d HostId (delete Mapping for HostId)\n"
	"         -i Ipaddr (Add Mapping for HostId to Ipaddr)\n"
	"         -u Ipaddr (Map HostId to New_Ipaddr)\n"
	;

int
get_options(int argc, char **argv, char **hostid, char **ipaddr, int *hostaction)
{
	int             c;

	*hostid = *ipaddr = 0;
	*hostaction = HOST_SELECT;
	while ((c = getopt(argc, argv, "vsSdu:i:")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'S':
			*hostaction = HOST_LOCAL;
			break;
		case 's':
			*hostaction = HOST_SELECT;
			*ipaddr = optarg;
			break;
		case 'u':
			*hostaction = HOST_UPDATE;
			*ipaddr = optarg;
			break;
		case 'd':
			*hostaction = HOST_DELETE;
			*ipaddr = optarg;
			break;
		case 'i':
			*hostaction = HOST_INSERT;
			*ipaddr = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return(1);
		}
	}
	if (optind < argc)
		*hostid = argv[optind++];
	if (*hostaction != HOST_SELECT && *hostaction != HOST_LOCAL && !*hostid) {
		strerr_warn1("must supply Host Id", 0);
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
	return(0);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	char           *tmphost_line, *ptr, *hostid, *ipaddr;
	char            strnum[FMT_ULONG];
	int             hostaction;

	if(get_options(argc, argv, &hostid, &ipaddr, &hostaction))
		return(0);
	switch (hostaction)
	{
	case HOST_SELECT:
		qprintf(subfdoutsmall, "Hostid", "%-30s");
		qprintf(subfdoutsmall, " ", "%s");
		qprintf(subfdoutsmall, "IP Address\n", "%s");
		if(hostid && (tmphost_line = sql_getip(hostid))) {
			qprintf(subfdoutsmall, hostid, "%-30s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, tmphost_line, "%s");
			qprintf(subfdoutsmall, "\n", "%s");
		} else
		for(;;) {
			if(!(tmphost_line = vhostid_select())) /*- "hostid ip_address */
				break;
			for (hostid = ptr = tmphost_line; *ptr && !isspace(*ptr); ptr++);
			*ptr++ = 0;
			for (;*ptr && isspace(*ptr); ptr++);
			qprintf(subfdoutsmall, hostid, "%-30s");
			qprintf(subfdoutsmall, " ", "%s");
			qprintf(subfdoutsmall, ptr, "%s");
			qprintf(subfdoutsmall, "\n", "%s");
		}
		qprintf_flush(subfdoutsmall);
		break;
	case HOST_INSERT:
		vhostid_insert(hostid, ipaddr);
		break;
	case HOST_DELETE:
		vhostid_delete(hostid);
		break;
	case HOST_UPDATE:
		vhostid_update(hostid, ipaddr);
		break;
	case HOST_LOCAL:
		if (hostid && *hostid) {
			if (!update_local_hostid(hostid)) {
				qprintf(subfdoutsmall, "updated local hostid to ", "%s");
				qprintf(subfdoutsmall, hostid, "%s");
				qprintf(subfdoutsmall, "\n", "%s");
				qprintf_flush(subfdoutsmall);
				return (0);
			} else
				_exit(111);
		}
		if (!(hostid = get_local_hostid()))
			strerr_die1sys(111, "vhostid: failed to get localhostid");
		qprintf(subfdoutsmall, hostid, "%s");
		qprintf(subfdoutsmall, "\n", "%s");
		qprintf_flush(subfdoutsmall);
		break;
	default:
		strnum[fmt_uint(strnum, (unsigned int) hostaction)] = 0;
		strerr_warn2("vhostid: error, HostId Action is invalid ", strnum, 0);
		break;
	}
	return(0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return(0);
}
#endif
