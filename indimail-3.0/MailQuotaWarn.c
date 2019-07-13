/*
 * $Log: MailQuotaWarn.c,v $
 * Revision 1.2  2019-04-21 16:14:08+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-20 08:15:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <fmt.h>
#include <str.h>
#include <scan.h>
#endif
#include "getEnvConfig.h"
#include "parse_quota.h"
#include "recalc_quota.h"
#include "runcmmd.h"

#ifndef	lint
static char     sccsid[] = "$Id: MailQuotaWarn.c,v 1.2 2019-04-21 16:14:08+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("MailQuotaWarn: out of memory", 0);
	_exit(111);
}

int
MailQuotaWarn(char *username, char *domain, char *Maildir, char *QuotaAlloted)
{
	static stralloc maildir = {0}, quotawarn = {0}, quota_cmd = {0}, tmpbuf = {0};
	char           *ptr;
	int             i, percent_warn, percent_usage_disk, warn_usage, warn_mail;
	struct stat     statbuf;
	char            strnum[FMT_ULONG];
	mdir_t          Quota, total_usage;
#ifdef USE_MAILDIRQUOTA
	mdir_t          QuotaCount, mailcount;
	int             percent_usage_mail;
#endif

	/*-
	 * If the age of file QuotaWarn is more than a week send warning
	 * to user if overquota
	 */
	if (!str_diffn(QuotaAlloted, "NOQUOTA", 8))
		return (0);
	i = str_len(Maildir);
	if (Maildir[i - 1] == '/')
		i--;
	if (!stralloc_copyb(&tmpbuf, Maildir, i) ||
			!stralloc_catb(&tmpbuf, "/QuotaWarn", 10) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (!((stat(tmpbuf.s, &statbuf) ? time(0) : time(0) - statbuf.st_mtime) > 7 * 86400))
		return (0);
	if (!stralloc_copys(&maildir, Maildir) || !stralloc_0(&maildir))
		die_nomem();
	maildir.len--;
	if ((ptr = str_str(maildir.s, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
#ifdef USE_MAILDIRQUOTA
	if ((Quota = parse_quota(QuotaAlloted, &QuotaCount)) == -1) {
		strerr_warn2("MailQuotaWarn: parse_quota: ", QuotaAlloted, &strerr_sys);
		return (-1);
	} else
	if (!Quota)
		return (0);
	total_usage = recalc_quota(maildir.s, &mailcount, Quota, QuotaCount, 0);
#else
	scan_ulong(QuotaAlloted, &Quota);
	if (!Quota)
		return (0);
	total_usage = recalc_quota(maildir.s, 0);
#endif
	if (total_usage == -1)
		return (-1);
	warn_usage = warn_mail = 0;
	for (i = 10; i; i--) {
		if (!stralloc_copyb(&quotawarn, "QUOTAWARN", 9) ||
				!stralloc_catb(&quotawarn, strnum, fmt_uint(strnum, i)) ||
				!stralloc_0(&quotawarn))
			die_nomem();
		if ((ptr = env_get(quotawarn.s))) {
			scan_uint(ptr, (unsigned int *) &percent_warn);
			if (percent_warn < 0 || percent_warn > 100)
				continue;
			else {
				if ((percent_usage_disk = (total_usage * 100) / Quota) > percent_warn)
					warn_usage = 1;
#ifdef USE_MAILDIRQUOTA
				if (QuotaCount && (percent_usage_mail = (mailcount * 100) / QuotaCount) > percent_warn)
					warn_mail = 1;
#endif
				if (warn_usage || warn_mail) {
					close(open(tmpbuf.s, O_CREAT | O_TRUNC , 0644));
					getEnvConfigStr(&ptr, "OVERQUOTA_CMD", LIBEXECDIR"/overquota.sh");
					if (!access(ptr, X_OK)) {
						/*-
						 * Call overquota command with 7 arguments
						 * email maildir total_usage mailcount Quota QuotaCount percent_warn
						 */
						if (!stralloc_copys(&quota_cmd, ptr) ||
								!stralloc_cats(&quota_cmd, username) ||
								!stralloc_append(&quota_cmd, "@") ||
								!stralloc_cats(&quota_cmd, domain) ||
								!stralloc_append(&quota_cmd, " ") ||
								!stralloc_cats(&quota_cmd, maildir.s) ||
								!stralloc_append(&quota_cmd, " "))
							die_nomem();
						if (!stralloc_catb(&quota_cmd, strnum, fmt_ulong(strnum, total_usage)) ||
								!stralloc_append(&quota_cmd, " "))
							die_nomem();
#ifdef USE_MAILDIRQUOTA
						if (!stralloc_catb(&quota_cmd, strnum, fmt_ulong(strnum, mailcount)) ||
								!stralloc_append(&quota_cmd, " "))
							die_nomem();
#else
						if (!stralloc_catb(&quota_cmd, "-1 ", 3))
							die_nomem();
#endif
						if (!stralloc_catb(&quota_cmd, strnum, fmt_ulong(strnum, Quota)) ||
								!stralloc_append(&quota_cmd, " "))
							die_nomem();
#ifdef USE_MAILDIRQUOTA
						if (!stralloc_catb(&quota_cmd, strnum, fmt_ulong(strnum, QuotaCount)) ||
								!stralloc_append(&quota_cmd, " "))
							die_nomem();
#else
						if (!stralloc_catb(&quota_cmd, "-1 ", 3))
							die_nomem();
#endif
						if (!stralloc_catb(&quota_cmd, strnum, fmt_uint(strnum, percent_warn)) ||
								!stralloc_0(&quota_cmd))
							die_nomem();
						runcmmd(quota_cmd.s, 0);
					}
					return (1);
				}
			}
		}
	}
	return (0);
}
