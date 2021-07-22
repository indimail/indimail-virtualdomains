/*
 * $Log: vlimit.c,v $
 * Revision 1.6  2021-07-22 15:17:39+05:30  Cprogrammer
 * conditional define of _XOPEN_SOURCE
 *
 * Revision 1.5  2020-10-13 18:35:44+05:30  Cprogrammer
 * initialize struct tm for strptime() value too big error
 *
 * Revision 1.4  2019-06-07 15:46:15+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.3  2019-05-02 14:39:12+05:30  Cprogrammer
 * fixed SIGSEGV
 *
 * Revision 1.2  2019-04-22 23:19:46+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-18 07:59:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vlimit.c,v 1.6 2021-07-22 15:17:39+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef ENABLE_DOMAIN_LIMITS
#ifdef HAVE_UNISTD_H
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <scan.h>
#include <fmt.h>
#include <str.h>
#endif
#include "vlimits.h"
#include "get_assign.h"
#include "common.h"
#include "parse_quota.h"

#define FATAL   "vlimits: fatal: "
#define WARN    "vlimits: warning: "

static char    *usage =
	"usage: vlimit [options] domain\n"
	"options: -v ( display the indimail version number )\n"
	"         -s ( show current settings )\n"
	"         -D ( delete limits for this domain, i.e. switch to default limits)\n"
	"         -e edate ( set domain   expiry date (ddmmyyyyHHMMSS) | -n no of days\n"
	"         -t edate ( set password expiry date (ddmmyyyyHHMMSS) | -N no of days\n"
	"         -Q quota ( set domain quota )\n"
	"         -M count ( set domain max msg count )\n"
	"         -q quota ( set default user quota )\n"
	"         -m count ( set default user max msg count )\n"
	"         -P count ( set max amount of pop accounts )\n"
	"         -A count ( set max amount of aliases )\n"
	"         -F count ( set max amount of forwards )\n"
	"         -R count ( set max amount of autoresponders )\n"
	"         -L count ( set max amount of mailing lists )\n"
	"the following options are bit flags in the gid int field\n"
	"         -g \"flags\"  (set flags, see below)\n"
	"          gid flags:\n"
	"            u ( set no dialup flag )\n"
	"            d ( set no password changing flag )\n"
	"            p ( set no pop access flag )\n"
	"            s ( set no smtp access flag )\n"
	"            w ( set no web mail access flag )\n"
	"            i ( set no imap access flag )\n"
	"            r ( set no external relay flag )\n"
	"            x ( clear all flags )\n"
	"the following options are bit flags for non postmaster admins\n"
	"         -p \"flags\"  (set pop account flags)\n"
	"         -a \"flags\"  (set alias flags)\n"
	"         -f \"flags\"  (set forward flags)\n"
	"         -r \"flags\"  (set autoresponder flags)\n"
	"         -l \"flags\"  (set mailinglist flags)\n"
	"         -u \"flags\"  (set mailinglist users flags)\n"
	"         -o \"flags\"  (set mailinglist moderators flags)\n"
	"         -x \"flags\"  (set domain  quota flags)\n"
	"         -z \"flags\"  (set default quota flags)\n"
	"         perm flags:\n"
	"            a ( set deny all flag )\n"
	"            c ( set deny create flag )\n"
	"            m ( set deny modify flag )\n"
	"            d ( set deny delete flag )"
	;

int
get_options(int argc, char **argv, char **Domain, char **DomainQuota, char **DefaultUserQuota,
		char **DomainMaxMsgCount, char **DefaultUserMaxMsgCount, char **MaxPopAccounts,
		char **MaxAliases, char **MaxForwards, char **MaxAutoresponders, char **MaxMailinglists,
		char **GidFlagString, char **PermAccountFlagString, char **PermAliasFlagString,
		char **PermForwardFlagString, char **PermAutoresponderFlagString, char **PermMaillistFlagString,
		char **PermMaillistUsersFlagString, char **PermMaillistModeratorsFlagString,
		char **PermQuotaFlagString, char **PermDefaultQuotaFlagString, int *QuotaFlag,
		int *GidFlag, int *PermAccountFlag, int *PermAliasFlag, int *PermForwardFlag,
		int *PermAutoresponderFlag, int *PermMaillistFlag, int *PermMaillistUsersFlag,
		int *PermMaillistModeratorsFlag, int *PermQuotaFlag, int *PermDefaultQuotaFlag,
		int *ShowLimits, int *DeleteLimits, long *domain_expiry, long *passwd_expiry)
{
	int             c;
	extern char    *optarg;
	extern int      optind;
	char            flag[4];
	struct tm       tm = {0};

	*Domain = *DomainQuota = *DefaultUserQuota = *DomainMaxMsgCount =
		*DefaultUserMaxMsgCount = *MaxPopAccounts = *MaxAliases =
		*MaxForwards = *MaxAutoresponders = *MaxMailinglists =
		*GidFlagString = 0;

	*PermAccountFlagString = *PermAliasFlagString = *PermForwardFlagString =
	*PermAutoresponderFlagString = *PermMaillistFlagString =
	*PermMaillistUsersFlagString = *PermMaillistModeratorsFlagString =
	*PermQuotaFlagString = *PermDefaultQuotaFlagString = 0;

	*QuotaFlag = *GidFlag = *PermAccountFlag = *PermAliasFlag =
		*PermForwardFlag = *PermAutoresponderFlag = *PermMaillistFlag =
		*PermMaillistUsersFlag = *PermMaillistModeratorsFlag =
		*PermQuotaFlag = *PermDefaultQuotaFlag = *ShowLimits =
		*DeleteLimits = 0;
	*domain_expiry = *passwd_expiry = 0l;

	flag[0] = flag[1] = flag[2] = flag[3] = 0;
	while ((c = getopt(argc, argv, "vst:e:n:N:DQ:q:M:m:P:A:F:R:L:g:p:a:f:r:l:u:o:x:z:h")) != opteof) {
		switch (c)
		{
		case 'v':
			out("vlimits", "version: ");
			out("vlimits", VERSION);
			out("vlimits", "\n");
			flush("vlimits");
			break;
		case 's':
			*ShowLimits = 1;
			break;
		case 'e':
			*ShowLimits = 0;
			flag[0] = 1;
			if (str_diffn(optarg, "-1", 3)) {
				if (str_len(optarg) != 14 || !strptime(optarg, "%d%m%Y%H%M%S", &tm)) {
					strerr_warn3("Invalid domain expiry date [", optarg, "]", 0);
					strerr_warn2(WARN, usage, 0);
					return(1);
				} else
				if ((*domain_expiry = mktime(&tm)) == -1) {
					strerr_warn3("Invalid start date [", optarg, "]", 0);
					strerr_warn2(WARN, usage, 0);
					return(1);
				}
			} else
				*domain_expiry = -1; /* Disable check on expiry date*/
			break;
		case 'n':
			*ShowLimits = 0;
			flag[1] = 1;
			scan_ulong(optarg, (unsigned long *) domain_expiry);
			*domain_expiry *= 86400;
			*domain_expiry += time(0);
			break;
		case 't':
			*ShowLimits = 0;
			flag[2] = 1;
			if (str_diffn(optarg, "-1", 3)) {
				if (str_len(optarg) != 14 || !strptime(optarg, "%d%m%Y%H%M%S", &tm)) {
					strerr_warn3("Invalid password expiry date [", optarg, "]", 0);
					strerr_warn2(WARN, usage, 0);
					return(1);
				} else
				if ((*passwd_expiry = mktime(&tm)) == -1) {
					strerr_warn3("Invalid start date [", optarg, "]", 0);
					strerr_warn2(WARN, usage, 0);
					return(1);
				}
			} else
				*passwd_expiry = -1; /* Disable check on expiry date*/
			break;
		case 'N':
			*ShowLimits = 0;
			flag[3] = 1;
			scan_ulong(optarg, (unsigned long *) passwd_expiry);
			*passwd_expiry *= 86400;
			*passwd_expiry += time(0);
			break;
		case 'D':
			*ShowLimits = 0;
			*DeleteLimits = 1;
			break;
		case 'Q':
			*ShowLimits = 0;
			*DomainQuota = optarg;
			break;
		case 'q':
			*ShowLimits = 0;
			*DefaultUserQuota = optarg;
			break;
		case 'M':
			*ShowLimits = 0;
			*DomainMaxMsgCount = optarg;
			break;
		case 'm':
			*ShowLimits = 0;
			*DefaultUserMaxMsgCount = optarg;
			break;
		case 'P':
			*ShowLimits = 0;
			*MaxPopAccounts = optarg;
			break;
		case 'A':
			*ShowLimits = 0;
			*MaxAliases = optarg;
			break;
		case 'F':
			*ShowLimits = 0;
			*MaxForwards = optarg;
			break;
		case 'R':
			*ShowLimits = 0;
			*MaxAutoresponders = optarg;
			break;
		case 'L':
			*ShowLimits = 0;
			*MaxMailinglists = optarg;
			break;
		case 'g':
			*ShowLimits = 0;
			*GidFlagString = optarg;
			*GidFlag = 1;
			break;
		case 'p':
			*ShowLimits = 0;
			*PermAccountFlagString = optarg;
			*PermAccountFlag = 1;
			break;
		case 'a':
			*ShowLimits = 0;
			*PermAliasFlagString = optarg;
			*PermAliasFlag = 1;
			break;
		case 'f':
			*ShowLimits = 0;
			*PermForwardFlagString = optarg;
			*PermForwardFlag = 1;
			break;
		case 'r':
			*ShowLimits = 0;
			*PermAutoresponderFlagString = optarg;
			*PermAutoresponderFlag = 1;
			break;
		case 'l':
			*ShowLimits = 0;
			*PermMaillistFlagString = optarg;
			*PermMaillistFlag = 1;
			break;
		case 'u':
			*ShowLimits = 0;
			*PermMaillistUsersFlagString = optarg;
			*PermMaillistUsersFlag = 1;
			break;
		case 'o':
			*ShowLimits = 0;
			*PermMaillistModeratorsFlagString = optarg;
			*PermMaillistModeratorsFlag = 1;
			break;
		case 'x':
			*ShowLimits = 0;
			*PermQuotaFlagString = optarg;
			*PermQuotaFlag = 1;
			break;
		case 'z':
			*ShowLimits = 0;
			*PermDefaultQuotaFlagString = optarg;
			*PermDefaultQuotaFlag = 1;
			break;
		case 'h':
			*ShowLimits = 0;
			strerr_warn2(WARN, usage, 0);
			return (1);
		default:
			*ShowLimits = 0;
			break;
		}
	}
	if (flag[0] && flag[1]) {
		strerr_warn1("must supply either Domain expiry date or Number of days", 0);
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
	if (flag[2] && flag[3]) {
		strerr_warn1("must supply either Password expiry date or Number of days", 0);
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
	if (optind < argc)
		*Domain = argv[optind++];
	if (!*Domain) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

int
main(int argc, char *argv[])
{
	int             i;
	struct vlimits  limits;
	char          *Domain, *DomainQuota, *DefaultUserQuota, *DomainMaxMsgCount,
				  *DefaultUserMaxMsgCount, *MaxPopAccounts, *MaxAliases,
				  *MaxForwards, *MaxAutoresponders, *MaxMailinglists,
				  *GidFlagString, *PermAccountFlagString, *PermAliasFlagString,
				  *PermForwardFlagString, *PermAutoresponderFlagString,
				  *PermMaillistFlagString, *PermMaillistUsersFlagString,
				  *PermMaillistModeratorsFlagString, *PermQuotaFlagString,
				  *PermDefaultQuotaFlagString;
	int             QuotaFlag, GidFlag, PermAccountFlag, PermAliasFlag,
					PermForwardFlag, PermAutoresponderFlag, PermMaillistFlag,
					PermMaillistUsersFlag, PermMaillistModeratorsFlag,
					PermQuotaFlag, PermDefaultQuotaFlag, ShowLimits,
					DeleteLimits;
	long            domain_expiry = 0, passwd_expiry = 0;
	char            strnum[FMT_ULONG];


	if (get_options(argc, argv, &Domain, &DomainQuota, &DefaultUserQuota, &DomainMaxMsgCount,
				&DefaultUserMaxMsgCount, &MaxPopAccounts, &MaxAliases, &MaxForwards,
				&MaxAutoresponders, &MaxMailinglists, &GidFlagString, &PermAccountFlagString,
				&PermAliasFlagString, &PermForwardFlagString, &PermAutoresponderFlagString,
				&PermMaillistFlagString, &PermMaillistUsersFlagString,
				&PermMaillistModeratorsFlagString, &PermQuotaFlagString,
				&PermDefaultQuotaFlagString, &QuotaFlag, &GidFlag, &PermAccountFlag,
				&PermAliasFlag, &PermForwardFlag, &PermAutoresponderFlag, &PermMaillistFlag,
				&PermMaillistUsersFlag, &PermMaillistModeratorsFlag, &PermQuotaFlag,
				&PermDefaultQuotaFlag, &ShowLimits, &DeleteLimits, &domain_expiry, &passwd_expiry))
		return (1);
	if (!get_assign(Domain, 0, 0, 0)) {
		strerr_warn2(Domain, ": No such domain\n", 0);
		return (1);
	}
	if (vget_limits(Domain, &limits)) {
		strerr_warn1("vlimits: vget_limits: Failed to get limits: ", 0);
		return (-1);
	}
	if (DeleteLimits) {
		if (vdel_limits(Domain) == 0) {
			out("vlimits", "Limits deleted\n");
			flush("vlimits");
			return (0);
		} else {
			strerr_warn1("vlimits: failed to delete limits", 0);
			return (-1);
		}
	}
	if (domain_expiry)
		limits.domain_expiry = domain_expiry;
	if (passwd_expiry)
		limits.passwd_expiry = passwd_expiry;
	if (MaxPopAccounts)
		scan_int(MaxPopAccounts, &limits.maxpopaccounts);
	if (MaxAliases)
		scan_int(MaxAliases, &limits.maxaliases);
	if (MaxForwards)
		scan_int(MaxForwards, &limits.maxforwards);
	if (MaxAutoresponders)
		scan_int(MaxAutoresponders, &limits.maxautoresponders);
	if (MaxMailinglists)
		scan_int(MaxMailinglists, &limits.maxmailinglists);
	/*- quota & message count limits */
	if (DomainQuota && (limits.diskquota = parse_quota(DomainQuota, 0)) == -1) {
		strerr_warn1("vlimits: diskquota: ", &strerr_sys);
		return (-1);
	}
	if (DomainMaxMsgCount &&
		(limits.maxmsgcount = strtoll(DomainMaxMsgCount, 0, 0)) == -1)
	{
		strerr_warn1("vlimits: maxmsgcount: ", &strerr_sys);
		return (-1);
	}
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (limits.maxmsgcount == LLONG_MIN || limits.maxmsgcount == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
	{
		strerr_warn1("vlimits: maxmsgcount: ", &strerr_sys);
		return (-1);
	}

	if (DefaultUserQuota) {
		if (!str_diffn(DefaultUserQuota, "NOQUOTA", 8))
			limits.defaultquota = -1;
		else
		if ((limits.defaultquota = parse_quota(DefaultUserQuota, 0)) == -1) {
			strerr_warn1("vlimits: defaultquota: ", &strerr_sys);
			return (-1);
		}
	}
	if (DefaultUserMaxMsgCount &&
			(limits.defaultmaxmsgcount = strtoll(DefaultUserMaxMsgCount, 0, 0)) == -1)
	{
		strerr_warn1("vlimits: defaultmaxmsgcount: ", &strerr_sys);
		return (-1);
	}
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (limits.defaultmaxmsgcount == LLONG_MIN || limits.defaultmaxmsgcount == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
	{
		strerr_warn1("vlimits: defaultmaxmsgcount: ", &strerr_sys);
		return (-1);
	}
	if (GidFlag == 1) {
		GidFlag = 0;
		limits.disable_dialup = 0;
		limits.disable_passwordchanging = 0;
		limits.disable_pop = 0;
		limits.disable_smtp = 0;
		limits.disable_webmail = 0;
		limits.disable_imap = 0;
		limits.disable_relay = 0;
		for (i = 0; i < str_len(GidFlagString); i++) {
			switch (GidFlagString[i])
			{
			case 'u':
				limits.disable_dialup = 1;
				break;
			case 'd':
				limits.disable_passwordchanging = 1;
				break;
			case 'p':
				limits.disable_pop = 1;
				break;
			case 's':
				limits.disable_smtp = 1;
				break;
			case 'w':
				limits.disable_webmail = 1;
				break;
			case 'i':
				limits.disable_imap = 1;
				break;
			case 'r':
				limits.disable_relay = 1;
				break;
			case 'x':
				limits.disable_dialup = 0;
				limits.disable_passwordchanging = 0;
				limits.disable_pop = 0;
				limits.disable_smtp = 0;
				limits.disable_webmail = 0;
				limits.disable_imap = 0;
				limits.disable_relay = 0;
				break;
			}
		}
	}
	if (PermAccountFlag == 1) {
		limits.perm_account = 0;
		for (i = 0; i < str_len(PermAccountFlagString); i++) {
			switch (PermAccountFlagString[i])
			{
			case 'a':
				limits.perm_account |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_account |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_account |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_account |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermAliasFlag == 1) {
		limits.perm_alias = 0;
		for (i = 0; i < str_len(PermAliasFlagString); i++) {
			switch (PermAliasFlagString[i])
			{
			case 'a':
				limits.perm_alias |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_alias |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_alias |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_alias |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermForwardFlag == 1) {
		limits.perm_forward = 0;
		for (i = 0; i < str_len(PermForwardFlagString); i++) {
			switch (PermForwardFlagString[i])
			{
			case 'a':
				limits.perm_forward |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_forward |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_forward |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_forward |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermAutoresponderFlag == 1) {
		limits.perm_autoresponder = 0;
		for (i = 0; i < str_len(PermAutoresponderFlagString); i++) {
			switch (PermAutoresponderFlagString[i])
			{
			case 'a':
				limits.perm_autoresponder |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_autoresponder |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_autoresponder |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_autoresponder |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermMaillistFlag == 1) {
		limits.perm_maillist = 0;
		for (i = 0; i < str_len(PermMaillistFlagString); i++) {
			switch (PermMaillistFlagString[i])
			{
			case 'a':
				limits.perm_maillist |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_maillist |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_maillist |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_maillist |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermMaillistUsersFlag == 1) {
		limits.perm_maillist_users = 0;
		for (i = 0; i < str_len(PermMaillistUsersFlagString); i++) {
			switch (PermMaillistUsersFlagString[i])
			{
			case 'a':
				limits.perm_maillist_users |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_maillist_users |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_maillist_users |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_maillist_users |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermMaillistModeratorsFlag == 1) {
		limits.perm_maillist_moderators = 0;
		for (i = 0; i < str_len(PermMaillistModeratorsFlagString); i++) {
			switch (PermMaillistModeratorsFlagString[i])
			{
			case 'a':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_maillist_moderators |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermQuotaFlag == 1) {
		limits.perm_quota = 0;
		for (i = 0; i < str_len(PermQuotaFlagString); i++) {
			switch (PermQuotaFlagString[i])
			{
			case 'a':
				limits.perm_quota |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_quota |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_quota |= VLIMIT_DISABLE_MODIFY;
				break;
			case 'd':
				limits.perm_quota |= VLIMIT_DISABLE_DELETE;
				break;
			}
		}
	}
	if (PermDefaultQuotaFlag == 1) {
		limits.perm_defaultquota = 0;
		for (i = 0; i < str_len(PermDefaultQuotaFlagString); i++) {
			switch (PermDefaultQuotaFlagString[i])
			{
			case 'a':
				limits.perm_defaultquota |= VLIMIT_DISABLE_ALL;
				break;
			case 'c':
				limits.perm_defaultquota |= VLIMIT_DISABLE_CREATE;
				break;
			case 'm':
				limits.perm_defaultquota |= VLIMIT_DISABLE_MODIFY;
				break;
			}
		}
	}
	if (!ShowLimits && vset_limits(Domain, &limits) != 0) {
		strerr_warn1("vset_limits: Failed to set limits", 0);
		return (-1);
	}
	if (ShowLimits) {
		out("vlimits", "Domain Expiry Date   : ");
		out("vlimits", limits.domain_expiry == -1 ? "Never Expires\n" : ctime(&limits.domain_expiry));
		out("vlimits", "Password Expiry Date : ");
		out("vlimits", limits.passwd_expiry == -1 ? "Never Expires\n" : ctime(&limits.passwd_expiry));
		out("vlimits", "Max Domain Quota     : ");
		strnum[fmt_ulong(strnum, (unsigned long) limits.diskquota)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		out("vlimits", "Max Domain Messages  : ");
		strnum[fmt_ulong(strnum, (unsigned long) limits.maxmsgcount)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		if (limits.defaultquota == -1)
			out("vlimits", "Default User Quota   : unlimited\n");
		else {
			out("vlimits", "Default User Quota   : ");
			strnum[fmt_ulong(strnum, (unsigned long) limits.defaultquota)] = 0;
			out("vlimits", strnum);
			out("vlimits", "\n");
		}
		out("vlimits", "Default User Messages: ");
		strnum[fmt_ulong(strnum, (unsigned long) limits.defaultmaxmsgcount)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		out("vlimits", "Max Pop Accounts     : ");
		strnum[fmt_int(strnum, limits.maxpopaccounts)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		out("vlimits", "Max Aliases          : ");
		strnum[fmt_int(strnum, limits.maxaliases)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		out("vlimits", "Max Forwards         : ");
		strnum[fmt_int(strnum, limits.maxforwards)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		out("vlimits", "Max Autoresponders   : ");
		strnum[fmt_int(strnum, limits.maxautoresponders)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		out("vlimits", "Max Mailinglists     : ");
		strnum[fmt_int(strnum, limits.maxmailinglists)] = 0;
		out("vlimits", strnum);
		out("vlimits", "\n");
		out("vlimits", "GID Flags:\n");
		if (limits.disable_imap != 0)
			out("vlimits", "  NO_IMAP\n");
		if (limits.disable_smtp != 0)
			out("vlimits", "  NO_SMTP\n");
		if (limits.disable_pop != 0)
			out("vlimits", "  NO_POP\n");
		if (limits.disable_webmail != 0)
			out("vlimits", "  NO_WEBMAIL\n");
		if (limits.disable_passwordchanging != 0)
			out("vlimits", "  NO_PASSWD_CHNG\n");
		if (limits.disable_relay != 0)
			out("vlimits", "  NO_RELAY\n");
		if (limits.disable_dialup != 0)
			out("vlimits", "  NO_DIALUP\n");
		out("vlimits", "Flags for non postmaster accounts:\n");
		out("vlimits", "  pop account           : ");
		out("vlimits", limits.perm_account & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_account & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_account & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  alias                 : ");
		out("vlimits", limits.perm_alias & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_alias & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_alias & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  forward               : ");
		out("vlimits", limits.perm_forward & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_forward & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_forward & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  autoresponder         : ");
		out("vlimits", limits.perm_autoresponder & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_autoresponder & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_autoresponder & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  mailinglist           : ");
		out("vlimits", limits.perm_maillist & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_maillist & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_maillist & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  mailinglist users     : ");
		out("vlimits", limits.perm_maillist_users & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_maillist_users & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_maillist_users & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  mailinglist moderators: ");
		out("vlimits", limits.perm_maillist_moderators & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_maillist_moderators & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_maillist_moderators & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  domain quota          : ");
		out("vlimits", limits.perm_quota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_quota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", limits.perm_quota & VLIMIT_DISABLE_DELETE ? "DENY_DELETE  " : "ALLOW_DELETE ");
		out("vlimits", "\n");
		out("vlimits", "  default quota         : ");
		out("vlimits", limits.perm_defaultquota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE  " : "ALLOW_CREATE ");
		out("vlimits", limits.perm_defaultquota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY  " : "ALLOW_MODIFY ");
		out("vlimits", "\n");
		flush("vlimits");
		return (0);
	}
	return (0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-domain-limits=y", 0);
	return(0);
}
#endif
