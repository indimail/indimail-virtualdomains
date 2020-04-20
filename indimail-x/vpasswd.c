/*
 * $Log: vpasswd.c,v $
 * Revision 1.3  2020-04-01 18:59:07+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 15:44:30+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-14 18:29:31+05:30  Cprogrammer
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
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <str.h>
#include <makesalt.h>
#endif
#include "parse_email.h"
#include "get_real_domain.h"
#include "vgetpasswd.h"
#include "ipasswd.h"
#include "iclose.h"
#include "post_handle.h"
#include "variables.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vpasswd.c,v 1.3 2020-04-01 18:59:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vpasswd: fatal: "
#define WARN    "vpasswd: warning: "

static char    *usage =
	"usage: vpasswd [options] email_address [password]\n"
	"options: -V (print version number)\n"
	"         -v (verbose)\n"
	"         -a (use apop, pop is default)\n"
	"         -e encrypted password (set the encrypted password field)\n"
	"         -r Generate a random password"
	;
extern int      encrypt_flag;

int
get_options(int argc, char **argv, char **email, char **passwd, int *apop)
{
	int             c, Random;
	char           *ptr;

	*email = *passwd = 0;
	*apop = USE_POP;
	Random = 0;
	while ((c = getopt(argc, argv, "vrae")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
		case 'a':
			*apop = USE_APOP;
			break;
		case 'r':
			Random = 1;
			break;
		case 'e':
			encrypt_flag = 1;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc)
		*email = argv[optind++];
	if (*email) {
		if (optind < argc)
			*passwd = argv[optind++];
		else {
			if (Random)
				ptr = genpass(8);
			else
			if (!(ptr = vgetpasswd(*email))) {
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*passwd = ptr;
		}
	}
	if (!*email || !*passwd) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             i, apop;
	char           *real_domain, *ptr, *email, *passwd, *base_argv0;
	static stralloc user = {0}, domain = {0};

	if (get_options(argc, argv, &email, &passwd, &apop))
		return (1);
	parse_email(email, &user, &domain);
	if (!domain.len) {
		strerr_warn2(user.s, ": No domain specified", 0);
		return (1);
	}
	real_domain = (char *) 0;
	if (!(real_domain = get_real_domain(domain.s))) {
		strerr_warn2(domain.s, ": No such domain\n", 0);
		return (1);
	}
	if ((i = ipasswd(user.s, real_domain, passwd, apop)) != 1) {
		if (!i)
			strerr_warn5("vpasswd: ", user.s, "@", real_domain, ": No such user", 0);
		iclose();
		if (i == -1)
			return (-1);
		return (1);
	}
	iclose();
	if (!(ptr = env_get("POST_HANDLE")))
	{
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s@%s", LIBEXECDIR, base_argv0, user.s, real_domain));
	} else
		return (post_handle("%s %s@%s", ptr, user.s, real_domain));
}
