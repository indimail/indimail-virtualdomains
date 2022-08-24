/*
 * $Log: vpasswd.c,v $
 * Revision 1.11  2022-08-24 18:35:53+05:30  Cprogrammer
 * made setting hash method and scram method independent
 *
 * Revision 1.10  2022-08-07 20:40:51+05:30  Cprogrammer
 * check return value of gsasl_mkpasswd() function
 *
 * Revision 1.9  2022-08-07 13:12:16+05:30  Cprogrammer
 * updated usage string
 *
 * Revision 1.8  2022-08-06 19:34:25+05:30  Cprogrammer
 * fix compilation when libgsasl is missing or of wrong version
 *
 * Revision 1.7  2022-08-06 11:19:07+05:30  Cprogrammer
 * include gsasl.h
 *
 * Revision 1.6  2022-08-05 23:39:53+05:30  Cprogrammer
 * compile gsasl code for libgsasl version >= 1.8.1
 *
 * Revision 1.5  2022-08-05 23:15:01+05:30  Cprogrammer
 * conditional compilation of gsasl code
 *
 * Revision 1.4  2022-08-05 21:21:57+05:30  Cprogrammer
 * added option to update scram passwords
 *
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
#include <fmt.h>
#include <makesalt.h>
#include <scan.h>
#include <hashmethods.h>
#endif
#ifdef HAVE_GSASL_H
#include <gsasl.h>
#include "gsasl_mkpasswd.h"
#endif
#include "parse_email.h"
#include "get_real_domain.h"
#include "vgetpasswd.h"
#include "ipasswd.h"
#include "iclose.h"
#include "post_handle.h"
#include "variables.h"
#include "indimail.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: vpasswd.c,v 1.11 2022-08-24 18:35:53+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vpasswd: fatal: "
#define WARN    "vpasswd: warning: "

static char    *usage =
	"usage: vpasswd [options] email_address [password]\n"
	"options: -v (verbose)\n"
	"         -e encrypted  (set the encrypted password field)\n"
	"         -r            (Generate a random password of specfied length)\n"
	"         -h hash       (use one of DES, MD5, SHA256, SHA512, hash method)\n"
	"         -m SCRAM method (use one of SCRAM-SHA-1, SCRAM-SHA-256 SCRAM method\n"
	"         -S salt       (Use a fixed base64 encoded salt for generating SCRAM password)\n"
	"                       (If salt is not specified, it will be generated)\n"
	"         -i iter_count (Use iter_count instead of 4096 for generating SCRAM password)\n"
	"         -H            (display this usage)"
	;

int
get_options(int argc, char **argv, char **email, char **clear_text, int *encrypt_flag, int *scram, int *iter, char **salt)
{
	int             c, i, Random, passwd_len = 8;
	char           *ptr;
	char            optstr[14], strnum[FMT_ULONG];

	*email = *clear_text = *salt = 0;
	*iter = 4096;
	*scram = 0;
	*encrypt_flag = -1;
	Random = 0;
	/*- make sure optstr has enough size to hold all options + 1 */
	i = 0;
	i += fmt_strn(optstr + i, "veh:r:", 6);
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	i += fmt_strn(optstr + i, "m:S:I:", 6);
#endif
#endif
	optstr[i] = 0;
	while ((c = getopt(argc, argv, optstr)) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'h':
			if (!str_diffn(optarg, "DES", 3))
				strnum[fmt_int(strnum, DES_HASH)] = 0;
			else
			if (!str_diffn(optarg, "MD5", 3))
				strnum[fmt_int(strnum, MD5_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-256", 7))
				strnum[fmt_int(strnum, SHA256_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-512", 7))
				strnum[fmt_int(strnum, SHA512_HASH)] = 0;
			else {
				errout("vpasswd", WARN);
				errout("vpasswd", optarg);
				errout("vpasswd", ": wrong hash method\n");
				errout("vpasswd", "Supported HASH Methods: DES MD5 SHA-256 SHA-512\n");
				errflush("vpasswd");
				strerr_die2x(100, WARN, usage);
			}
			if (!env_put2("PASSWORD_HASH", strnum))
				strerr_die1x(111, "out of memory");
			*encrypt_flag = 1;
			break;
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
		case 'm':
			if (!str_diffn(optarg, "SCRAM-SHA-1", 11))
				*scram = 1;
			else
			if (!str_diffn(optarg, "SCRAM-SHA-256", 13))
				*scram = 2;
			else {
				errout("vpasswd", WARN);
				errout("vpasswd", optarg);
				errout("vpasswd", ": wrong SCRAM method\n");
				errout("vpasswd", "Supported SCRAM Methods: SCRAM-SHA-1 SCRAM-SHA-256\n");
				errflush("vpasswd");
				strerr_die2x(100, WARN, usage);
			}
			break;
		case 'S':
			i = str_chr(optarg, ',');
			if (optarg[i]) {
				strerr_die3x(100, WARN, optarg, ": salt cannot have a comma character");
			}
			*salt = optarg;
			break;
		case 'I':
			scan_int(optarg, iter);
			break;
#endif
#endif
		case 'r':
			Random = 1;
			scan_int(optarg, &passwd_len);
			break;
		case 'e':
			/*- ignore encrypt flag option if -h option is provided */
			if (*encrypt_flag == -1)
				*encrypt_flag = 0;
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
			*clear_text = argv[optind++];
		else {
			if (Random) {
				ptr = genpass(passwd_len);
				strerr_warn3("Generated random passwd [", ptr, "]", 0);
			} else
			if (!(ptr = vgetpasswd(*email))) {
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*clear_text = ptr;
		}
	}
	if (!*email || !*clear_text) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (*encrypt_flag == -1)
		*encrypt_flag = 1;
	return (0);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             i, encrypt_flag, scram, iter;
	char           *real_domain, *ptr, *email, *clear_text, *base_argv0, *b64salt;
	static stralloc user = {0}, domain = {0}, result = {0};

	if (get_options(argc, argv, &email, &clear_text, &encrypt_flag, &scram, &iter, &b64salt))
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
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	switch (scram)
	{
	case 1: /*- SCRAM-SHA-1 */
		if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-1", iter, b64salt, clear_text, &result)) != NO_ERR)
			strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
		break;
	case 2: /*- SCRAM-SHA-256 */
		if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-256", iter, b64salt, clear_text, &result)) != NO_ERR)
			strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
		break;
	}
#endif
#endif
	if ((i = ipasswd(user.s, real_domain, clear_text, encrypt_flag, scram ? result.s : 0)) != 1) {
		if (!i)
			strerr_warn5("vpasswd: ", user.s, "@", real_domain, ": No such user", 0);
		iclose();
		if (i == -1)
			return (-1);
		return (1);
	}
	iclose();
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s@%s", LIBEXECDIR, base_argv0, user.s, real_domain));
	} else
		return (post_handle("%s %s@%s", ptr, user.s, real_domain));
}
