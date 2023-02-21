/*
 * $Log: vgroup.c,v $
 * Revision 1.6  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.5  2022-11-02 20:15:00+05:30  Cprogrammer
 * added feature to add scram password during user addition
 *
 * Revision 1.4  2022-08-05 22:44:59+05:30  Cprogrammer
 * removed apop argument to iadduser()
 * added encrypt flag to iadduser()
 *
 * Revision 1.3  2019-06-07 15:51:55+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:19:29+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-18 08:34:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vgroup.c,v 1.6 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#define XOPEN_SOURCE = 600
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <makesalt.h>
#include <hashmethods.h>
#include <subfd.h>
#endif
#ifdef HAVE_GSASL_H
#include <gsasl.h>
#include "gsasl_mkpasswd.h"
#endif
#include "parse_email.h"
#include "sqlOpen_user.h"
#include "iopen.h"
#include "iclose.h"
#include "iadduser.h"
#include "sql_getpw.h"
#include "get_real_domain.h"
#include "parse_quota.h"
#include "sql_getip.h"
#include "SqlServer.h"
#include "vgetpasswd.h"
#include "strmsg.h"
#include "valias_insert.h"
#include "valias_delete.h"
#include "valias_update.h"
#include "common.h"
#include "variables.h"

#define ADDNEW_GROUP  0
#define INSERT_MEMBER 1
#define DELETE_MEMBER 2
#define UPDATE_MEMBER 3
#define FATAL         "vgroup: fatal: "
#define WARN          "vgroup: warning: "

static char    *usage =
	"usage1: vgroup -a [-cqvVhmI] groupAddress [password]\n"
	"usage2: vgroup    [-iduovV]  groupAddress\n\n"
	"options\n"
	"  -V                - print version number\n"
	"  -v                - verbose\n"
	"  -a                - add new group\n"
	"  -r len            - generate a random password of length=len\n"
	"  -e password       - set the encrypted password field\n"
	"  -h hash           - use one of DES, MD5, SHA256, SHA512, hash method\n"
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	"  -C                - store clear txt and scram hex salted password in database\n"
	"                      This allows CRAM methods to be used\n"
	"  -m SCRAM method   - use one of SCRAM-SHA-1, SCRAM-SHA-256 SCRAM method\n"
	"  -S salt           - use a fixed base64 encoded salt for generating SCRAM password\n"
	"                    - if salt is not specified, it will be generated\n"
	"  -I iter_count     - use iter_count instead of 4096 for generating SCRAM password\n"
#endif
#endif
#ifdef CLUSTERED_SITE
	"  -H hostid         - host on which the group needs to be created - specify hostid\n"
	"  -M mdahost        - host on which the group needs to be created - specify mdahost\n"
	"  -n                - Ignore requirement of of groupAddress to be local\n"
#endif
	"  -c comment        - sets the gecos comment field\n"
	"  -q quota_in_bytes - sets the users quota\n"
	"  -i member_address - insert member to group\n"
	"  -d member_address - delete member from group\n"
	"  -u new -o old     - update old member with a new address)"
	;

static int
get_options(int argc, char **argv, int *option, char **group, char **gecos,
	char **member, char **old_member, char **passwd,
	char **hostid, char **mdahost, char **quota, int *ignore,
	int *encrypt_flag, int *random, int *docram, int *scram, int *iter, char **salt)
{
	int             c, i;
	char            optstr[27], strnum[FMT_ULONG];

	*group = *gecos = *member = *old_member = *passwd = *hostid = *mdahost = *quota = 0;
	*option = -1;
	*ignore = 0;
	*random = 0;
	*encrypt_flag = -1;
	if (salt)
		*salt = 0;
	if (iter)
		*iter = 4096;
	if (scram)
		*scram = 0;
	if (docram)
		*docram = 0;
	/*- make sure optstr has enough size to hold all options + 1 */
	i = 0;
	i += fmt_strn(optstr + i, "vanc:i:d:o:u:q:", 15);
#ifdef CLUSTERED_SITE
	i += fmt_strn(optstr + i, "H:M:", 4);
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	i += fmt_strn(optstr + i, "Cm:S:I:", 7);
#endif
#endif
	if ((i + 1) > sizeof(optstr))
		strerr_die2x(100, FATAL, "allocated space for getopt string not enough");
	optstr[i] = 0;

	while ((c = getopt(argc, argv, optstr)) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'r':
			scan_uint(optarg, (unsigned int *) random);
			break;
		case 'a':
			switch (*option)
			{
			case ADDNEW_GROUP:
			case INSERT_MEMBER:
			case DELETE_MEMBER:
			case UPDATE_MEMBER:
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*option = ADDNEW_GROUP;
			break;
		case 'n':
			*ignore = 1;
			break;
		case 'c':
			*gecos = optarg;
			break;
		case 'q':
			*quota = optarg;
			break;
		case 'i':
			if (*option == ADDNEW_GROUP) {
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			if (!*optarg) {
				strerr_warn1("You cannot have an empty email address", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*option = INSERT_MEMBER;
			*member = optarg;
			break;
		case 'd':
			if (*option == ADDNEW_GROUP) {
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*option = DELETE_MEMBER;
			*member = optarg;
			break;
		case 'u':
			if (*option == ADDNEW_GROUP) {
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			if (!*optarg) {
				strerr_warn1("You cannot have an empty email address", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*option = UPDATE_MEMBER;
			*member = optarg;
			break;
		case 'o':
			if (*option == ADDNEW_GROUP) {
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			if (!*optarg) {
				strerr_warn1("You cannot have an empty email address", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*option = UPDATE_MEMBER;
			*old_member = optarg;
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
			else
				strerr_die5x(100, FATAL, "wrong hash method ", optarg, ". Supported HASH Methods: DES MD5 SHA-256 SHA-512\n", usage);
			if (!env_put2("PASSWORD_HASH", strnum))
				strerr_die1x(111, "out of memory");
			*encrypt_flag = 1;
			break;
		case 'e':
			/*- ignore encrypt flag option if -h option is provided */
			if (*encrypt_flag == -1)
				*encrypt_flag = 0;
			break;
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
		case 'C':
			if (docram)
				*docram = 1;
			break;
		case 'm':
			if (!scram)
				break;
			if (!str_diffn(optarg, "SCRAM-SHA-1", 11))
				*scram = 1;
			else
			if (!str_diffn(optarg, "SCRAM-SHA-256", 13))
				*scram = 2;
			else
				strerr_die5x(100, FATAL, "wrong SCRAM method ", optarg, ". Supported SCRAM Methods: SCRAM-SHA1 SCRAM-SHA-256\n", usage);
			break;
		case 'S':
			if (!salt)
				break;
			i = str_chr(optarg, ',');
			if (optarg[i]) {
				strerr_die3x(100, WARN, optarg, ": salt cannot have a comma character");
			}
			*salt = optarg;
			break;
		case 'I':
			if (!iter)
				break;
			scan_int(optarg, iter);
			break;
#endif
#endif
			break;
#ifdef CLUSTERED_SITE
		case 'H':
			*hostid = optarg;
			break;
		case 'M':
			*mdahost = optarg;
			break;
#endif
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc)
		*group = argv[optind++];
	else {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (optind < argc)
		*passwd = argv[optind++];
	if (*option == UPDATE_MEMBER && (!*member || !*old_member)) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (*option == -1) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (!*passwd) {
		if (*random)
			*passwd = genpass(*random);
		else
			*passwd = vgetpasswd(*gecos ? *gecos : *group);
	}
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vgroup: out of memory", 0);
	_exit(111);
}

int
addGroup(char *user, char *domain, char *mdahost, char *gecos,
		char *passwd, char *quota, int encrypt_flag, char *scram)
{
	struct passwd  *pw;
	static stralloc tmpbuf = {0};
	int             i;

	if (!stralloc_copyb(&tmpbuf, "MailGroup ", 10) ||
			!stralloc_cats(&tmpbuf, gecos ? gecos : user) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((i = iadduser(user, domain, mdahost, passwd, tmpbuf.s, quota, 0, 1, encrypt_flag, scram)) < 0)
		return (i);
	if (!(pw = sql_getpw(user, domain))) {
		strerr_warn4(user, "@", domain, ": sql_getpw failed: ", &strerr_sys);
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	static stralloc User = {0}, Domain = {0}, alias_line = {0}, old_alias = {0},
					quotaVal = {0};
	char           *group, *gecos, *member, *old_member, *passwd, *hostid,
				   *mdahost, *Quota, *real_domain, *ptr;
	char            strnum[FMT_ULONG];
	mdir_t          q;
	int             i, option, ignore = 0, ret = -1, encrypt_flag, random;
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	int             scram, iter, docram;
	static stralloc result = {0};
	char           *b64salt;
#endif
#endif

#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	i = get_options(argc, argv, &option, &group, &gecos, &member, &old_member,
			&passwd, &hostid, &mdahost, &Quota, &ignore, &encrypt_flag, &random,
			&docram, &scram, &iter, &b64salt);
#else
	i = get_options(argc, argv, &option, &group, &gecos, &member, &old_member,
			&passwd, &hostid, &mdahost, &Quota, &ignore, &encrypt_flag, &random,
			0, 0, 0, 0);
#endif
#else
	i = get_options(argc, argv, &option, &group, &gecos, &member, &old_member,
			&passwd, &hostid, &mdahost, &Quota, &ignore, &encrypt_flag, &random,
			0, 0, 0, 0);
#endif
	if (i)
		return (i);

	parse_email(group, &User, &Domain);
	if (option != ADDNEW_GROUP) {
#ifdef CLUSTERED_SITE
		if (sqlOpen_user(group, 0))
#else
		if (iopen((char *) 0))
#endif
			return (1);
	}
	/* Do this so that users do not get added in a alias domain */
	real_domain = (char *) 0;
	if (Domain.len && !(real_domain = get_real_domain(Domain.s))) {
		strerr_warn2(Domain.s, ": No such domain", 0);
		iclose();
		return (1);
	}
	switch (option)
	{
		case ADDNEW_GROUP:
			if (Quota && *Quota) {
				if (str_diffn(Quota, "NOQUOTA", 8)) {
					if ((q = parse_quota(Quota, 0)) == -1) {
						strerr_warn3("vgroup: parse_quota: ", Quota, ": ", &strerr_sys);
						iclose();
						return (1);
					}
					strnum[i = fmt_ulong(strnum, (unsigned int) q)] = 0;
					if (!stralloc_copyb(&quotaVal, strnum, i))
						die_nomem();
					i = str_chr(Quota, ',');
					if (Quota[i]) {
						if (!stralloc_cats(&quotaVal, Quota + i))
							die_nomem();
					}
					if (!stralloc_0(&quotaVal))
						die_nomem();
					quotaVal.len--;
				}
			} else
				quotaVal.len = 0;
#ifdef CLUSTERED_SITE
			if (!mdahost && hostid) {
				if (!(ptr = sql_getip(hostid))) {
					strerr_warn4("vgroup: failed to obtain mdahost for host ", hostid, " domain ", real_domain, 0);
					iclose();
					return (1);
				} else
					mdahost = ptr;
			}
			/* add the user */
			if (mdahost) {
				iclose();
				if (!(ptr = SqlServer(mdahost, real_domain))) {
					strerr_warn4("vgroup: failed to obtain sqlserver for host ", mdahost, " domain ", real_domain, 0);
					return (1);
				} else
				if (iopen(ptr)) {
					strerr_warn2("vgroup: failed to connect to ", ptr, 0);
					return (1);
				}
				if (verbose)
					strmsg_out5("Connected to MDAhost ", mdahost, " SqlServer ", ptr, "\n");
			}
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
			switch (scram)
			{
			case 1: /*- SCRAM-SHA-1 */
				if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-1", iter, b64salt, docram, passwd, &result)) != NO_ERR)
					strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
				break;
			case 2: /*- SCRAM-SHA-256 */
				if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-256", iter, b64salt, docram, passwd, &result)) != NO_ERR)
					strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
				break;
			/*- more cases will get below when devils come up with a new RFC */
			}
			ptr = scram ? result.s : 0;
#else
			ptr = 0;
#endif
#else
			ptr = 0;
#endif
			ret = addGroup(User.s, real_domain, mdahost, gecos, passwd, quotaVal.s, encrypt_flag, ptr);
			if (!ret && random) {
				subprintfe(subfdout, "vgroup", "Password is %s\n", passwd);
				flush("vgroup");
			}
			break;
		case INSERT_MEMBER:
			if (*member == '.') {
				strerr_warn2("vgroup: Illegal member ", member, 0);
				iclose();
				return (1);
			}
			if (*member != '&' && *member != '/' && *member != '|') {
				if (!stralloc_copyb(&alias_line, "&", 1) ||
						!stralloc_cats(&alias_line, member) ||
						!stralloc_0(&alias_line))
					die_nomem();
			} else {
				if (!stralloc_copys(&alias_line, member) ||
						!stralloc_0(&alias_line))
					die_nomem();
			}
			alias_line.len--;
			ret = valias_insert(User.s, real_domain, alias_line.s, ignore);
			break;
		case DELETE_MEMBER:
			if (*member == '.') {
				strerr_warn2("vgroup: Illegal member ", member, 0);
				iclose();
				return (1);
			}
			if (*member != '&' && *member != '/' && *member != '|') {
				if (!stralloc_copyb(&alias_line, "&", 1) ||
						!stralloc_cats(&alias_line, member) ||
						!stralloc_0(&alias_line))
					die_nomem();
			} else {
				if (!stralloc_copys(&alias_line, member) ||
						!stralloc_0(&alias_line))
					die_nomem();
			}
			alias_line.len--;
			ret = valias_delete(User.s, real_domain, alias_line.s);
			break;
		case UPDATE_MEMBER:
			if (*member == '.') {
				strerr_warn2("vgroup: Illegal member ", member, 0);
				iclose();
				return (1);
			} else
			if (*old_member == '.') {
				strerr_warn2("vgroup: Illegal member ", old_member, 0);
				iclose();
				return (1);
			}
			if (*member != '&' && *member != '/' && *member != '|') {
				if (!stralloc_copyb(&alias_line, "&", 1) ||
						!stralloc_cats(&alias_line, member) ||
						!stralloc_0(&alias_line))
					die_nomem();
			} else {
				if (!stralloc_copys(&alias_line, member) ||
						!stralloc_0(&alias_line))
					die_nomem();
			}
			alias_line.len--;
			if (*old_member != '&' && *old_member != '/' && *old_member != '|') {
				if (!stralloc_copyb(&old_alias, "&", 1) ||
						!stralloc_cats(&old_alias, member) ||
						!stralloc_0(&old_alias))
					die_nomem();
			} else {
				if (!stralloc_copys(&old_alias, member) ||
						!stralloc_0(&old_alias))
					die_nomem();
			}
			old_alias.len--;
			ret = valias_update(User.s, real_domain, old_alias.s, alias_line.s);
			break;
	}
	iclose();
	return (ret);
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
