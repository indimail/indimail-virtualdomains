/*
 * $Log: vatrn.c,v $
 * Revision 1.3  2022-10-20 11:58:33+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.2  2019-06-07 15:55:03+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-15 10:29:56+05:30  Cprogrammer
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
#include <stralloc.h>
#include <strerr.h>
#include <qprintf.h>
#include <subfd.h>
#include <sgetopt.h>
#endif
#include "variables.h"
#include "atrn_map.h"
#include "parse_email.h"

#ifndef	lint
static char     sccsid[] = "$Id: vatrn.c,v 1.3 2022-10-20 11:58:33+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL     "vatrn: fatal: "
#define WARN      "vatrn: warning: "
#define PRINT_IT  0
#define ADD_IT    1
#define DELETE_IT 2
#define UPDATE_IT 3

static char    *usage =
	"usage: vatrn [options] email|ATRNdomain\n"
	"options: -d ATRNdomain (delete mapping)\n"
	"         -i ATRNdomain (add    mapping)\n"
	"         -u ATRNdomain -i newATRNDomain (update mapping)\n"
	"         -s print mapping\n"
	"         -V print version number\n"
	"         -v verbose"
	;

int
get_options(int argc, char **argv, int *Action, char **emailid,
		stralloc *user, stralloc *domain, char **domain_list, char **old_domain)
{
	int             c;

	/*- Action = PRINT_IT; -*/
	*Action = -1;
	*emailid = *domain_list = *old_domain = 0;
	while ((c = getopt(argc, argv, "vsd:i:u:n:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			*Action = PRINT_IT;
			break;
		case 'i':
			if (*Action != UPDATE_IT)
				*Action = ADD_IT;
			else
				*Action = UPDATE_IT;
			*domain_list = optarg;
			break;
		case 'd':
			*Action = DELETE_IT;
			*domain_list = optarg;
			break;
		case 'u':
			*Action = UPDATE_IT;
			*old_domain = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (*Action == -1) {
		strerr_warn1("vatrn: must specify one of -s, -i, -u and -i, -d options", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (optind < argc) {
		*emailid = argv[optind++];
		parse_email(*emailid, user, domain);
	}
	if (*Action != PRINT_IT && !*emailid) {
		strerr_warn1("must supply email address or domain", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if ((*Action == ADD_IT || *Action == DELETE_IT || *Action == UPDATE_IT) && !*domain_list) {
		strerr_warn1("must supply argument for domain", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	int             result, Action;
	char           *email, *domain_list, *old_domain;
	static stralloc User = {0}, Domain = {0};
	char           *ptr, *user, *domain;

	if (get_options(argc, argv, &Action, &email, &User, &Domain, &domain_list, &old_domain))
		return (1);
	switch (Action)
	{
	case ADD_IT:
		if ((result = add_atrn_map(User.s, Domain.s, domain_list)) == -1)
			_exit(111);
		break;
	case DELETE_IT:
		if ((result = del_atrn_map(User.s, Domain.s, domain_list)) == -1)
			_exit(111);
		break;
	case UPDATE_IT:
		if ((result = upd_atrn_map(User.s, Domain.s, old_domain, domain_list)) == -1)
			_exit(111);
		break;
	case PRINT_IT:
		domain = Domain.s;
		for(result = 1;;) {
			if (User.len)
				user = User.s;
			else
				user = (char *) 0;
			if (!(ptr = show_atrn_map(&user, &domain)))
				break;
			result = 0;
			subprintf(subfdoutsmall, "%-20s %-20s %s\n", user, domain, ptr);
			substdio_flush(subfdoutsmall);
		}
		if (result && verbose) {
			subprintf(subfdoutsmall, "No ATRN Maps present\n");
			substdio_flush(subfdoutsmall);
		}
		break;
	default:
		strerr_warn2(WARN, usage, 0);
		return (1);
		break;
	}
	return (result);
}
