/*
 * $Log: vipmap.c,v $
 * Revision 1.2  2019-04-22 23:19:40+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-15 10:30:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vipmap.c,v 1.2 2019-04-22 23:19:40+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef IP_ALIAS_DOMAINS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <memory.h>
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#endif
#include "ip_map.h"
#include "common.h"
#include "variables.h"

#define FATAL     "vipmap: fatal: "
#define WARN      "vipmap: warning: "
#define PRINT_IT  0
#define ADD_IT    1
#define DELETE_IT 2
#define UPDATE_IT 3

static char    *usage =
	"usage: vipmap [options] domain\n"
	"options: -d ipaddr (delete mapping)\n"
	"         -i ipaddr (add    mapping)\n"
	"         -u ipaddr (update mapping)\n"
	"         -s print mapping\n"
	"         -V print version number\n"
	"         -v verbose"
	;

int
get_options(int argc, char **argv, int *action, char **ip, char **domain)
{
	int             c;

	*action = PRINT_IT;
	*ip = *domain = 0;
	while ((c = getopt(argc, argv, "vsd:i:u:")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			*action = PRINT_IT;
			break;
		case 'i':
			*action = ADD_IT;
			*ip = optarg;
			break;
		case 'd':
			*action = DELETE_IT;
			*ip = optarg;
			break;
		case 'u':
			*action = UPDATE_IT;
			*ip = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc)
		*domain = argv[optind++];
	if (*action == ADD_IT || *action == DELETE_IT || *action == UPDATE_IT) {
		if (!*ip || !*domain) {
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	return (0);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             result, first, Action;
	static stralloc s_ip = {0}, s_domain = {0};
	char           *domain, *ip;

	if (get_options(argc, argv, &Action, &ip, &domain))
		return (1);
	switch (Action)
	{
	case ADD_IT:
		if ((result = add_ip_map(ip, domain)) == -1)
			_exit(111);
		break;
	case DELETE_IT:
		if ((result = del_ip_map(ip, domain)) == -1)
			_exit(111);
		break;
	case UPDATE_IT:
		if ((result = upd_ip_map(ip, domain)) == -1)
			_exit(111);
		break;
	case PRINT_IT:
		for (first = 1;(result = show_ip_map(first, &s_ip, &s_domain, domain)) == 1;) {
			first = 0;
			out("vipmap", s_ip.s);
			out("vipmap", " ");
			out("vipmap", s_domain.s);
			out("vipmap", "\n");
			flush("vipmap");
		}
		if (first && verbose) {
			out("vipmap", "No IP Maps present\n");
			flush("vipmap");
		}
		break;
	default:
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (result);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("vipmap: IndiMail not configured with --enable-ip-alias-domains=y", 0);
	return (1);
}
#endif
