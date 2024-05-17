/*
 * $Log: vserverinfo.c,v $
 * Revision 1.3  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <subfd.h>
#endif
#include "variables.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "findhost.h"
#include "is_distributed_domain.h"
#include "LoadDbInfo.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: vserverinfo.c,v 1.3 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      display_user, display_passwd, display_database, display_port,
				display_server, display_mdahost, display_all;

static int
get_options(int argc, char **argv, char **mdahost, char **server, const char **domain, char **email)
{
	int             c;
	char           *ptr;

	*email = *mdahost = *server = NULL;
	*domain = NULL;
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
			for (ptr = optarg; *ptr; ptr++) {
				if (isupper(*ptr))
					strerr_die3x(100, "vserverinfo: domain [", optarg, "] has an uppercase character");
			}
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
		if (optind < argc) {
			for (ptr = argv[optind]; *ptr; ptr++) {
				if (isupper(*ptr))
					strerr_die3x(100, "vserverinfo: email [", argv[optind], "] has an uppercase character");
			}
			*email = argv[optind++];
		} else {
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
	char           *mdahost, *server, *email, *mailstore;
	const char     *domain, *real_domain;
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
				if (server)
					subprintfe(subfdout, "vserverinfo", "Record  : %03d\n", found + 1);
				if (display_server || display_all)
					subprintfe(subfdout, "vserverinfo", "server  : %s\n", (*rhostsptr)->server);
				if (display_mdahost || display_all)
					subprintfe(subfdout, "vserverinfo", "mdahost : %s\n", (*rhostsptr)->mdahost);
				if (display_user || display_all)
					subprintfe(subfdout, "vserverinfo", "user    : %s\n", (*rhostsptr)->user);
				if (display_passwd || display_all)
					subprintfe(subfdout, "vserverinfo", "password: %s\n", (*rhostsptr)->password);
				if (display_port || display_all)
					subprintfe(subfdout, "vserverinfo", "password: %d\n", (*rhostsptr)->port);
				if (display_database || display_all)
					subprintfe(subfdout, "vserverinfo", "password: %s\n", (*rhostsptr)->database);
				flush("vserverinfo");
				found++;
			}
		}
	}
	if (!found)
		strerr_warn2("vserverinfo: could not locate server info for ", real_domain, 0);
	return (found ? 0 : 1);
}
