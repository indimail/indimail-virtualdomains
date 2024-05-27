/*
 * $Id: vlimit.c,v 1.8 2024-05-28 00:21:32+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vlimit.c,v 1.8 2024-05-28 00:21:32+05:30 Cprogrammer Exp mbhangui $";
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
#include <str.h>
#include <subfd.h>
#include <fmt.h>
#endif
#include "vlimits.h"
#include "get_assign.h"
#include "common.h"
#include "parse_quota.h"
#include "check_group.h"

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
	"            s ( set no smtp access flag )\n"
	"            i ( set no imap access flag )\n"
	"            p ( set no pop access flag )\n"
	"            w ( set no web mail access flag )\n"
	"            r ( set no external relay flag )\n"
	"            x ( clear all flags )\n"
	"         -T toggle bit flag for -g option\n"
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
static int      toggle;

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
	while ((c = getopt(argc, argv, "vst:e:n:N:DQ:q:M:m:P:A:F:R:L:Tg:p:a:f:r:l:u:o:x:z:h")) != opteof) {
		switch (c)
		{
		case 'v':
			subprintfe(subfdout, "vlimits", "version: %s\n", VERSION);
			flush("vlimits");
			break;
		case 's':
			*ShowLimits = 1;
			break;
		case 'e': /*- domain expiry */
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
		case 'n': /*- domain expiry */
			*ShowLimits = 0;
			flag[1] = 1;
			scan_long(optarg, domain_expiry);
			*domain_expiry *= 86400;
			*domain_expiry += time(0);
			break;
		case 't': /*- password expiry */
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
		case 'N': /*- password expiry */
			*ShowLimits = 0;
			flag[3] = 1;
			scan_long(optarg, passwd_expiry);
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
		case 'T':
			toggle = 1;
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
	struct vlimits  limits = { 0 };
	char           *Domain, *DomainQuota, *DefaultUserQuota, *DomainMaxMsgCount,
				   *DefaultUserMaxMsgCount, *MaxPopAccounts, *MaxAliases,
				   *MaxForwards, *MaxAutoresponders, *MaxMailinglists,
				   *GidFlagString, *PermAccountFlagString, *PermAliasFlagString,
				   *PermForwardFlagString, *PermAutoresponderFlagString,
				   *PermMaillistFlagString, *PermMaillistUsersFlagString,
				   *PermMaillistModeratorsFlagString, *PermQuotaFlagString,
				   *PermDefaultQuotaFlagString;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             QuotaFlag, GidFlag, PermAccountFlag, PermAliasFlag,
					PermForwardFlag, PermAutoresponderFlag, PermMaillistFlag,
					PermMaillistUsersFlag, PermMaillistModeratorsFlag,
					PermQuotaFlag, PermDefaultQuotaFlag, ShowLimits,
					DeleteLimits, i;
	uid_t           Uid, myuid;
	gid_t           Gid, mygid;
	long            domain_expiry = 0, passwd_expiry = 0;


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
	myuid = getuid();
	mygid = getgid();
	if (!get_assign(Domain, 0, &Uid, &Gid)) {
		strerr_warn2(Domain, ": No such domain", 0);
		return (1);
	}
	if (!Uid)
		strerr_die4x(100, FATAL, "domain ", Domain, " with uid 0");
	if (myuid != Uid && mygid != Gid && myuid != 0 && check_group(Gid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, Uid)] = 0;
		strnum2[fmt_ulong(strnum2, Gid)] = 0;
		strerr_warn6(WARN, "you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return(1);
	}
	if (vget_limits(Domain, &limits)) {
		strerr_warn2(FATAL, "Failed to get limits: ", 0);
		return (-1);
	}
	if (DeleteLimits) {
		if (vdel_limits(Domain) == 0) {
			out("vlimits", "Limits deleted\n");
			flush("vlimits");
			return (0);
		} else {
			strerr_warn2(FATAL, "failed to delete limits", 0);
			return (-1);
		}
	}
	if (domain_expiry)
		limits.domain_expiry = domain_expiry;
	if (passwd_expiry)
		limits.passwd_expiry = passwd_expiry;
	if (MaxPopAccounts)
		scan_long(MaxPopAccounts, &limits.maxpopaccounts);
	if (MaxAliases)
		scan_long(MaxAliases, &limits.maxaliases);
	if (MaxForwards)
		scan_long(MaxForwards, &limits.maxforwards);
	if (MaxAutoresponders)
		scan_long(MaxAutoresponders, &limits.maxautoresponders);
	if (MaxMailinglists)
		scan_long(MaxMailinglists, &limits.maxmailinglists);
	/*- quota & message count limits */
	if (DomainQuota && (limits.diskquota = parse_quota(DomainQuota, 0)) == -1) {
		strerr_warn2(FATAL, "diskquota: ", &strerr_sys);
		return (-1);
	}
	if (DomainMaxMsgCount &&
		(limits.maxmsgcount = strtoll(DomainMaxMsgCount, 0, 0)) == -1)
	{
		strerr_warn2(FATAL, "maxmsgcount: ", &strerr_sys);
		return (-1);
	}
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (limits.maxmsgcount == LLONG_MIN || limits.maxmsgcount == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
	{
		strerr_warn2(FATAL, "maxmsgcount: ", &strerr_sys);
		return (-1);
	}

	if (DefaultUserQuota) {
		if (!str_diffn(DefaultUserQuota, "NOQUOTA", 8))
			limits.defaultquota = -1;
		else
		if ((limits.defaultquota = parse_quota(DefaultUserQuota, 0)) == -1) {
			strerr_warn2(FATAL, "defaultquota: ", &strerr_sys);
			return (-1);
		}
	}
	if (DefaultUserMaxMsgCount &&
			(limits.defaultmaxmsgcount = strtoll(DefaultUserMaxMsgCount, 0, 0)) == -1)
	{
		strerr_warn2(FATAL, "defaultmaxmsgcount: ", &strerr_sys);
		return (-1);
	}
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (limits.defaultmaxmsgcount == LLONG_MIN || limits.defaultmaxmsgcount == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
	{
		strerr_warn2(FATAL, "defaultmaxmsgcount: ", &strerr_sys);
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
				limits.disable_dialup = toggle ? 0 : 1;
				break;
			case 'd':
				limits.disable_passwordchanging = toggle ? 0 : 1;
				break;
			case 'p':
				limits.disable_pop = toggle ? 0 : 1;
				break;
			case 's':
				limits.disable_smtp = toggle ? 0 : 1;
				break;
			case 'w':
				limits.disable_webmail = toggle ? 0 : 1;
				break;
			case 'i':
				limits.disable_imap = toggle ? 0 : 1;
				break;
			case 'r':
				limits.disable_relay = toggle ? 0 : 1;
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
		subprintfe(subfdout, "vlimits", "Domain Expiry Date   : %s",
				limits.domain_expiry == -1 ? "Never Expires\n" : ctime(&limits.domain_expiry));
		subprintfe(subfdout, "vlimits", "Password Expiry Date : %s",
				limits.passwd_expiry == -1 ? "Never Expires\n" : ctime(&limits.passwd_expiry));
		subprintfe(subfdout, "vlimits", "Max Domain Quota     : %13lu\n", (unsigned long) limits.diskquota);
		subprintfe(subfdout, "vlimits", "Max Domain Messages  : %13lu\n", (unsigned long) limits.maxmsgcount);
		if (limits.defaultquota == -1)
			subprintfe(subfdout, "vlimits", "Default User Quota   : unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Default User Quota   : %13lu\n", (unsigned long) limits.defaultquota);
		subprintfe(subfdout, "vlimits", "Default User Messages: %13lu\n", (unsigned long) limits.defaultmaxmsgcount);
		subprintfe(subfdout, "vlimits", "Max Pop Accounts     : %13ld\n", limits.maxpopaccounts);
		subprintfe(subfdout, "vlimits", "Max Aliases          : %13ld\n", limits.maxaliases);
		subprintfe(subfdout, "vlimits", "Max Forwards         : %13ld\n", limits.maxforwards);
		subprintfe(subfdout, "vlimits", "Max Autoresponders   : %13ld\n", limits.maxautoresponders);
		subprintfe(subfdout, "vlimits", "Max Mailinglists     : %13ld\n", limits.maxmailinglists);
		out("vlimits", "\n");
		subprintfe(subfdout, "vlimits", "GID Flags:\n");
		subprintfe(subfdout, "vlimits", "  %s\n", limits.disable_dialup ? "NO_DIALUP" : "DIALUP");
		subprintfe(subfdout, "vlimits", "  %s\n", limits.disable_passwordchanging ? "NO_PASSWD_CHNG" : "PASSWD CHNG");
		subprintfe(subfdout, "vlimits", "  %s\n", limits.disable_smtp ? "NO_SMTP" : "SMTP");
		subprintfe(subfdout, "vlimits", "  %s\n", limits.disable_imap ? "NO_IMAP" : "IMAP");
		subprintfe(subfdout, "vlimits", "  %s\n", limits.disable_pop ? "NO_POP" : "POP3");
		subprintfe(subfdout, "vlimits", "  %s\n", limits.disable_webmail ? "NO_WEBMAIL" : "WEBMAIL");
		subprintfe(subfdout, "vlimits", "  %s\n", limits.disable_relay ? "NO_RELAY" : "RELAY");
		out("vlimits", "\n");
		subprintfe(subfdout, "vlimits", "Flags for non postmaster accounts:\n");
		subprintfe(subfdout, "vlimits", "  pop account           : %12s %12s %12s\n",
				limits.perm_account & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_account & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_account & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  alias                 : %12s %12s %12s\n",
				limits.perm_alias & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_alias & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_alias & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  forward               : %12s %12s %12s\n",
				limits.perm_forward & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_forward & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_forward & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  autoresponder         : %12s %12s %12s\n",
				limits.perm_autoresponder & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_autoresponder & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_autoresponder & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  mailinglist           : %12s %12s %12s\n",
				limits.perm_maillist & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_maillist & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_maillist & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  mailinglist users     : %12s %12s %12s\n",
				limits.perm_maillist_users & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_maillist_users & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_maillist_users & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  mailinglist moderators: %12s %12s %12s\n",
				limits.perm_maillist_moderators & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_maillist_moderators & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_maillist_moderators & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  domain quota          : %12s %12s %12s\n",
				limits.perm_quota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_quota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
				limits.perm_quota & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  default quota         : %12s %12s\n",
				limits.perm_defaultquota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
				limits.perm_defaultquota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY");
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
/*
 * $Log: vlimit.c,v $
 * Revision 1.8  2024-05-28 00:21:32+05:30  Cprogrammer
 * fixed data types
 * added -T option to toggle gid flags
 * initialize struct vlimits
 *
 * Revision 1.7  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
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
