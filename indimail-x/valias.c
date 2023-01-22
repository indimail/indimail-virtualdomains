/*
 * $Log: valias.c,v $
 * Revision 1.5  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.4  2022-10-20 11:58:30+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.3  2019-06-07 15:55:21+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:16:22+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-18 15:51:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: valias.c,v 1.5 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <subfd.h>
#include <fmt.h>
#endif
#include "iopen.h"
#include "variables.h"
#include "parse_email.h"
#include "valias_select.h"
#include "valias_insert.h"
#include "valias_update.h"
#include "valias_delete.h"
#include "common.h"

#define FATAL   "valias: fatal: "
#define WARN    "valias: warning: "

#define VALIAS_SELECT 0
#define VALIAS_INSERT 1
#define VALIAS_DELETE 2
#define VALIAS_UPDATE 3
#define VALIAS_TRACK  4

int             aliasaction, ignore_mailstore;

static char    *usage =
	"usage: valias [options] [email_address | domain_name]\n"
	"options: -v ( print version number )\n"
	"         -v ( verbose )\n"
	"         -s ( show aliases )\n"
	"         -s ( track aliases )\n"
	"         -d alias_line (delete alias line)\n"
	"         -i alias_line (insert alias line)\n"
	"         -u old_alias_line -i new_alias_line (update alias line)"
#ifdef CLUSTERED_SITE
	"\n"
	"         -m ( ignore requirement of email_address to be local)"
#endif
	;

int
get_options(int argc, char **argv, char **email, stralloc *alias, stralloc *domain,
		char **aliasLine, char **OaliasLine, int *aliasAction, int *ignore_mailstore)
{
	int             c;

	*email = *aliasLine = 0;
	*aliasAction = VALIAS_SELECT;
	while ((c = getopt(argc, argv, "vmsSu:d:i:")) != opteof) {
		switch (c)
		{
#ifdef CLUSTERED_SITE
		case 'm':
			*ignore_mailstore = 1;
#endif
		case 'v':
			verbose = 1;
			break;
		case 'S':
			*aliasAction = VALIAS_TRACK;
			break;
		case 's':
			*aliasAction = VALIAS_SELECT;
			break;
		case 'u':
			*aliasAction = VALIAS_UPDATE;
			if (!*optarg) {
				strerr_warn1("valias: You cannot have an empty alias line", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*OaliasLine = optarg;
			break;
		case 'd':
			*aliasAction = VALIAS_DELETE;
			*aliasLine = optarg;
			break;
		case 'i':
			if(*aliasAction != VALIAS_UPDATE)
				*aliasAction = VALIAS_INSERT;
			if (!*optarg) {
				strerr_warn1("valias: You cannot have an empty alias line", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*aliasLine = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc) {
		*email = argv[optind++];
		parse_email(*email, alias, domain);
	}
	if (aliasAction != VALIAS_SELECT && !*email) {
		strerr_warn1("valias: must supply alias email address or a domain name", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	char           *email, *aliasline, *oaliasline, *tmpalias_line;
	char            strnum[FMT_ULONG];
	static stralloc alias = {0}, domain = {0};
	int             aliasaction, ignore_mailstore;


	if (get_options(argc, argv, &email, &alias, &domain, &aliasline, &oaliasline, &aliasaction, &ignore_mailstore))
		return (1);
	if (iopen((char *) 0))
		return (1);
	switch (aliasaction)
	{
	case VALIAS_SELECT:
		if (!alias.len) {
			for(;;) {
				if(!(tmpalias_line = valias_select_all(&alias, &domain)))
					break;
				subprintfe(subfdout, "valias", "%s@%s -> %s\n", alias.s, domain.s, tmpalias_line);
			}
			flush("valias");
		} else {
			for(;;) {
				if(!(tmpalias_line = valias_select(alias.s, domain.s)))
					break;
				subprintfe(subfdout, "valias", "%s@%s -> %s\n", alias.s, domain.s, tmpalias_line);
			}
			flush("valias");
		}
		break;
	case VALIAS_INSERT:
			valias_insert(alias.s, domain.s, aliasline, ignore_mailstore);
		break;
	case VALIAS_DELETE:
			valias_delete(alias.s, domain.s, aliasline);
		break;
	case VALIAS_UPDATE:
			valias_update(alias.s, domain.s, oaliasline, aliasline);
		break;
	case VALIAS_TRACK:
			for(;;) {
				if (valias_track(email, &alias, &domain))
					break;
				subprintfe(subfdout, "valias", "%s@%s -> %s\n", alias.s, domain.s, email);
				flush("valias");
			}
		break;
	default:
		strnum[fmt_uint(strnum, (unsigned int) aliasaction)] = 0;
		strerr_warn3("valias: error, Alias Action [", strnum, "is invalid", 0);
		break;
	}
	return (0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-valias=y", 0);
	return (0);
}
#endif
