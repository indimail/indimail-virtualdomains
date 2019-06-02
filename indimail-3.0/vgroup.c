/*
 * $Log: vgroup.c,v $
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
static char     sccsid[] = "$Id: vgroup.c,v 1.2 2019-04-22 23:19:29+05:30 Cprogrammer Exp mbhangui $";
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
	"options: -V (print version number)\n"
	"         -v (verbose)\n"
	"         -a (Add new group)\n"
#ifdef CLUSTERED_SITE
	"         -h hostid   (host on which the group needs to be created - specify hostid)\n"
	"         -m mdahost  (host on which the group needs to be created - specify mdahost)\n"
	"         -I Ignore requirement of of groupAddress to be local\n"
#endif
	"         -c comment (sets the gecos comment field)\n"
	"         -q quota_in_bytes (sets the users quota)\n"
	"         -i member_email_address (insert member to group)\n"
	"         -d member_email_address (delete member from group)\n"
	"         -u newMemberEmail -o oldMemberEmail (update member with a new address)"
	;

static int
get_options(int argc, char **argv, int *option, char **group, char **gecos, char **member, char **old_member, char **passwd,
	char **hostid, char **mdahost, char **quota, int *ignore)
{
	int             c;

	*group = *gecos = *member = *old_member = *passwd = *hostid = *mdahost = *quota = 0;
	*option = -1;
	*ignore = 0;
	while ((c = getopt(argc, argv, "vaIc:i:d:o:u:h:m:q:")) != -1) {
		switch (c)
		{
		case 'v':
			verbose = 1;
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
		case 'I':
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
			break;
#ifdef CLUSTERED_SITE
		case 'h':
			*hostid = optarg;
			break;
		case 'm':
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
	if (!*passwd)
		*passwd = vgetpasswd(*gecos ? *gecos : *group);
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vgroup: out of memory", 0);
	_exit(111);
}

int
addGroup(char *user, char *domain, char *mdahost, char *gecos, char *passwd, char *quota)
{
	struct passwd  *pw;
	static stralloc tmpbuf = {0};
	int             i;

	if (!stralloc_copyb(&tmpbuf, "MailGroup ", 10) ||
			!stralloc_cats(&tmpbuf, gecos ? gecos : user) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((i = iadduser(user, domain, mdahost, passwd, tmpbuf.s, quota, 0, USE_POP, 1)) < 0)
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
				   *mdahost, *Quota, *real_domain;
	char            strnum[FMT_ULONG];
	mdir_t          q;
#ifdef CLUSTERED_SITE
	char           *ptr;
#endif
	int             i, option, ignore = 0, ret = -1;

	if (get_options(argc, argv, &option, &group, &gecos, &member, &old_member, &passwd,
		&hostid, &mdahost, &Quota, &ignore))
		return (1);
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
			ret = addGroup(User.s, real_domain, mdahost, gecos, passwd, quotaVal.s);
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
