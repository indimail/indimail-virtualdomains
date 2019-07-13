/*
 * $Log: vserverinfo.c,v $
 * Revision 1.2  2019-06-07 15:41:24+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:37:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <subfd.h>
#include <fmt.h>
#endif
#include "variables.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "findhost.h"
#include "is_distributed_domain.h"
#include "LoadDbInfo.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: vserverinfo.c,v 1.2 2019-06-07 15:41:24+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      display_user, display_passwd, display_database, display_port,
				display_server, display_mdahost, display_all;

static int
get_options(int argc, char **argv, char **mdahost, char **server, char **domain, char **email)
{
	int             c;

	*email = *mdahost = *server = *domain = (char *) 0;
	display_all = 0;
	display_user = display_passwd = display_server = display_mdahost = display_port = display_database = 0;
	while ((c = getopt(argc, argv, "vdsmupPD:M:S:a")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'u':
			display_all = 0;
			display_user = 1;
			break;
		case 'p':
			display_all = 0;
			display_passwd = 1;
			break;
		case 'P':
			display_all = 0;
			display_port = 1;
			break;
		case 's':
			display_all = 0;
			display_server = 1;
			break;
		case 'm':
			display_all = 0;
			display_mdahost = 1;
			break;
		case 'd':
			display_all = 0;
			display_database = 1;
			break;
		case 'D':
			*domain = optarg;
			break;
		case 'M':
			*mdahost = optarg;
			break;
		case 'S':
			*server = optarg;
			break;
		case 'a':
			display_all = 1;
			break;
		default:
			strerr_warn1("usage: vserserverinfo [-upPsmd] [-D domain -M host | -S server] | [email]", 0);
			return (1);
		}
	}
	if (!display_user && !display_passwd && !display_server && !display_mdahost
		&& !display_port && !display_database && !display_all)
		display_all = 1;
	if ((!*mdahost && !*server) || !*domain) {
		if (optind < argc)
			*email = argv[optind++];
		else {
			strerr_warn1("usage: vserserverinfo [-upPsmd] [-D domain -M host | -S server] | [email]", 0);
			return (1);
		}
	}
	return (0);
}

int
main(int argc, char **argv)
{
	DBINFO        **rhostsptr;
	int             i, total, found;
	char           *mdahost, *server, *domain, *email, *mailstore, *real_domain;
	char            strnum[FMT_ULONG];
	static stralloc User = {0}, Domain = {0};

	if (get_options(argc, argv, &mdahost, &server, &domain, &email))
		return (1);
	if (email && *email) {
		server = (char *) 0;
		parse_email(email, &User, &Domain);
		if (!(real_domain = get_real_domain(Domain.s)))
			real_domain = Domain.s;
		domain = real_domain;
		if ((found = is_distributed_domain(real_domain)) == -1) {
			strerr_warn3("vserverinfo: ", real_domain, ": is_distributed_domain failed", 0);
			return (1);
		} else
		if (found) {
			if (!(mailstore = findhost(email, 2))) {
				if (userNotFound)
					strerr_warn3("vserverinfo: email ", email, " not found", 0);
				else
					strerr_warn2("vserverinfo: No mailstore for ", email, 0);
				return (1);
			}
			i = str_rchr(mailstore, ':');
			if (mailstore[i])
				mailstore[i] = 0;
			i = str_chr(mailstore, ':');
			if (mailstore[i])
				mailstore += i + 1;
			mdahost = mailstore;
		} else
			strerr_warn3("vserverinfo: ", real_domain, ": not a distributed domain", 0);
	} else
		real_domain = domain;
	if (!RelayHosts && !(RelayHosts = LoadDbInfo_TXT(&total))) {
		strerr_warn1("vserverinfo: LoadDbInfo_TXT: ", &strerr_sys);
		return (1);
	}
	for (found = 0, rhostsptr = RelayHosts;*rhostsptr;rhostsptr++) {
		if (!str_diffn((*rhostsptr)->domain, domain, DBINFO_BUFF)) {
			if ((mdahost && !str_diffn((*rhostsptr)->mdahost, mdahost, DBINFO_BUFF))
					|| (server && !str_diffn((*rhostsptr)->server, server, DBINFO_BUFF))
				) {
				if (server) {
					strnum[fmt_uint(strnum, (unsigned int) found + 1)] = 0;
					out("vserverinfo", "Record  : ");
					if (found + 1 < 10)
						out("vserverinfo", "00");
					else
					if (found + 1 < 100)
						out("vserverinfo", "0");
					out("vserverinfo", strnum);
					out("vserverinfo", "\n");
				}
				if (display_server || display_all) {
					out("vserverinfo", "server  : ");
					out("vserverinfo", (*rhostsptr)->server);
					out("vserverinfo", "\n");
				}
				if (display_mdahost || display_all) {
					out("vserverinfo", "mdahost : ");
					out("vserverinfo", (*rhostsptr)->mdahost);
					out("vserverinfo", "\n");
				}
				if (display_user || display_all) {
					out("vserverinfo", "user    : ");
					out("vserverinfo", (*rhostsptr)->user);
					out("vserverinfo", "\n");
				}
				if (display_passwd || display_all) {
					out("vserverinfo", "password: ");
					out("vserverinfo", (*rhostsptr)->password);
					out("vserverinfo", "\n");
				}
				if (display_port || display_all) {
					strnum[fmt_uint(strnum, (unsigned int) (*rhostsptr)->port)] = 0;
					out("vserverinfo", "port    : ");
					out("vserverinfo", strnum);
					out("vserverinfo", "\n");
				}
				if (display_database || display_all) {
					out("vserverinfo", "database: ");
					out("vserverinfo", (*rhostsptr)->database);
					out("vserverinfo", "\n");
				}
				flush("vserverinfo");
				found++;
			}
		}
	}
	if (!found)
		strerr_warn2("vserverinfo: could not locate server info for ", real_domain, 0);
	return (found ? 0 : 1);
}
