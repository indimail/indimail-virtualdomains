/*
 * $Log: Login_Tasks.c,v $
 * Revision 1.2  2019-07-04 09:17:29+05:30  Cprogrammer
 * collapsed multiple if statements
 *
 * Revision 1.1  2019-04-20 08:14:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <case.h>
#include <scan.h>
#include <fmt.h>
#include <str.h>
#include <strerr.h>
#include <env.h>
#include <getEnvConfig.h>
#endif
#include "indimail.h"
#include "vmake_maildir.h"
#include "get_indimailuidgid.h"
#include "vget_lastauth.h"
#include "vset_lastauth.h"
#include "variables.h"
#include "recalc_quota.h"
#include "parse_quota.h"
#include "check_quota.h"
#include "SendWelcomeMail.h"
#include "open_smtp_relay.h"
#include "update_rules.h"
#include "getpeer.h"
#include "sql_active.h"
#include "runcmmd.h"
#include "bulk_mail.h"
#include "RemoteBulkMail.h"
#include "MailQuotaWarn.h"
#include "user_over_quota.h"
#include "vset_lastdeliver.h"

#ifndef	lint
static char     sccsid[] = "$Id: Login_Tasks.c,v 1.2 2019-07-04 09:17:29+05:30 Cprogrammer Exp mbhangui $";
#endif

int
Login_Tasks(pw, User, ServiceType)
	struct passwd  *pw;
	const char     *User;
	char           *ServiceType;
{
	char           *domain, *ptr, *migrateflag, *migrateuser, *postauth;
	static stralloc fqemail = {0}, Maildir = {0}, tmpbuf = {0}, pwbuf = {0},
					user = {0}, Subject = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	struct stat     statbuf;
	int             i, j, status, flag = 0;
#ifdef ENABLE_AUTH_LOGGING
	time_t          inact_time, tmval1, tmval2, min_login_interval;
#ifdef USE_MAILDIRQUOTA	
	mdir_t          size_limit, count_limit, quota;
#endif
#endif

	if (!pw)
		return (1);
	if (!stralloc_copys(&user, (char *) User))
		return (1);
	case_lowerb(user.s, user.len);
	case_lowers(pw->pw_name);
	i = str_chr((char *) User, '@');
	if (User[i]) {
		if (!stralloc_copy(&fqemail, &user))
			return (1);
		domain = user.s + i + 1;
	} else {
		getEnvConfigStr(&domain, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		case_lowers(domain);
		if (!stralloc_copy(&fqemail, &user) ||
				!stralloc_append(&fqemail, "@") ||
				!stralloc_cats(&fqemail, domain))
			return (1);
	}
	if (!stralloc_0(&user) || !stralloc_0(&fqemail))
		return (1);
	user.len--;
	fqemail.len--;
	create_flag = 1;
	if (access(pw->pw_dir, F_OK)) {
		get_indimailuidgid(&indimailuid, &indimailgid);
		if (vmake_maildir(pw->pw_dir, indimailuid, indimailgid, domain) == -1) {
			strnum1[fmt_ulong(strnum1, indimailuid)] = 0;
			strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
			strerr_warn7("Login_Tasks: vmake_maildir(", pw->pw_dir, ", ", strnum1, ", ", strnum2, "): ", &strerr_sys);
			return (1);
		}
#ifdef ENABLE_AUTH_LOGGING
		if ((inact_time = vget_lastauth(pw, domain, INACT_TIME, 0))) {
			struct tm *tm;
			int    year;
			tm = localtime(&inact_time);
			year = tm->tm_year + 1900;
			if (!stralloc_copyb(&Subject, "Your IndiMail Account was de-activated on ", 42) ||
					!stralloc_catb(&Subject, strnum1, fmt_uint(strnum1, tm->tm_mday)) ||
					!stralloc_append(&Subject, "-") ||
					!stralloc_catb(&Subject, strnum1, fmt_uint(strnum1, tm->tm_mon)) ||
					!stralloc_append(&Subject, "-") ||
					!stralloc_catb(&Subject, strnum1, fmt_uint(strnum1, year)) ||
					!stralloc_0(&Subject))
				return (1);
		} else
			Subject.len = 0;
		SendWelcomeMail(pw->pw_dir, user.s, domain, (is_inactive && inact_time) ? 1 : 0, Subject.s);
#else
		SendWelcomeMail(pw->pw_dir, user.s, domain, 0, "");
#endif
	}
	if (!stralloc_copys(&Maildir, pw->pw_dir) ||
			!stralloc_catb(&Maildir, "/Maildir", 8) ||
			!stralloc_0(&Maildir))
		return (1);
#ifdef ENABLE_AUTH_LOGGING
	if (is_inactive) {
		sql_active(pw, domain, FROM_INACTIVE_TO_ACTIVE);
#ifdef USE_MAILDIRQUOTA	
		if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1) {
			strerr_warn3("Login_Tasks: parse_quota: ", pw->pw_shell, ": ", &strerr_sys);
			return (1);
		}
		(void) recalc_quota(Maildir.s, 0, size_limit, count_limit, 2);
#else
		(void) recalc_quota(Maildir.s, 2);
#endif
	} else
	if (env_get("NOLASTAUTHLOGGING") || env_get("NOLASTAUTH"))
		return (0);
#endif /*- ENABLE_AUTH_LOGGING */
#ifdef POP_AUTH_OPEN_RELAY
	/*- open the relay to pop3/imap users */
	if (!env_get("NORELAY") && (pw->pw_gid & NO_RELAY) == 0 && env_get("OPEN_SMTP")
			&& open_smtp_relay(pw->pw_name, domain))
		update_rules(1); /*- update tcp.smtp.cdb */
#endif
#ifdef ENABLE_AUTH_LOGGING
	getEnvConfigStr(&ptr, "MIN_LOGIN_INTERVAL", "0");
	scan_ulong(ptr, (unsigned long *) &min_login_interval);
	if (min_login_interval) {
		if ((tmval2 = time(0)) - (tmval1 = vget_lastauth(pw, domain, AUTH_TIME, 0)) < min_login_interval) {
			strnum1[fmt_ulong(strnum1, (unsigned long) (tmval2 - tmval1))] = 0;
			strnum2[fmt_ulong(strnum2, (unsigned long) min_login_interval)] = 0;
			strerr_warn8("ERR: Login interval ", strnum1, " < ", strnum2, ", user=", pw->pw_name, ", domain=", domain, 0);
			return (2);
		}
	}
	tmval1 = vget_lastauth(pw, domain, PASS_TIME, 0);
	strnum1[fmt_ulong(strnum1, tmval1)] = 0;
	if (!env_put2("LAST_PASSWORD_CHANGE", strnum1))
		return (1);
#ifdef USE_MAILDIRQUOTA
	if ((quota = check_quota(Maildir.s, 0)) == -1) {
		strerr_warn1("Login_Tasks: check_quota: unable to get quota usage: ", &strerr_sys);
	}
	if ((ptr = (char *) env_get("TCPREMOTEIP")) || (ptr = GetPeerIPaddr()))
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, ptr, pw->pw_gecos, quota);
	else
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, "unknown", pw->pw_gecos, quota);
#else
	quota = check_quota(Maildir.s);
	if ((ptr = (char *) env_get("TCPREMOTEIP")) || (ptr = GetPeerIPaddr()))
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, ptr, pw->pw_gecos, quota);
	else
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, "unknown", pw->pw_gecos, quota);
#endif /*- USE_MAILDIRQUOTA */
#endif /*- ENABLE_AUTH_LOGGING */
	if ((postauth = (char *) env_get("POSTAUTH")) && !access(postauth, X_OK)) {
		if (!stralloc_copy(&pwbuf, &fqemail) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_cats(&pwbuf, pw->pw_passwd) ||
				!stralloc_append(&pwbuf, ":"))
			return (1);
		strnum1[i = fmt_uint(strnum1, pw->pw_uid)] = 0;
		strnum2[j = fmt_uint(strnum2, pw->pw_gid)] = 0;
		if (!stralloc_catb(&pwbuf, strnum1, i) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_catb(&pwbuf, strnum2, j) ||
				!stralloc_append(&pwbuf, ":"))
			return (1);
		strnum1[i = fmt_uint(strnum1, is_inactive)] = 0;
		if (!stralloc_cats(&pwbuf, pw->pw_gecos) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_cats(&pwbuf, pw->pw_dir) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_cats(&pwbuf, pw->pw_shell) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_catb(&pwbuf, strnum1, i) ||
				!stralloc_0(&pwbuf))
			return (1);
		if (!env_put2("PWSTRUCT", pwbuf.s))
			return (1);
		if (!stralloc_copys(&tmpbuf, postauth) ||
				!stralloc_append(&tmpbuf, " ") ||
				!stralloc_cat(&tmpbuf, &fqemail) ||
				!stralloc_append(&tmpbuf, " ") ||
				!stralloc_cats(&tmpbuf, pw->pw_dir) ||
				!stralloc_0(&tmpbuf))
			return (1);
		if ((status = runcmmd(tmpbuf.s, 0))) {
			strnum1[fmt_uint(strnum1, status)] = 0;
			strerr_warn9("Login_Tasks: runncmmd: ", postauth, " ", fqemail.s, " ", pw->pw_dir, " [status =", strnum1, "]", 0);
			return (status);
		}
	}
	getEnvConfigStr(&migrateflag, "MIGRATEFLAG", MIGRATEFLAG);
	if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
			!stralloc_append(&tmpbuf, "/") ||
			!stralloc_cats(&tmpbuf, migrateflag))
		return (1);
	getEnvConfigStr(&migrateuser, "MIGRATEUSER", MIGRATEUSER);
	if (access(tmpbuf.s, F_OK) && !access(migrateuser, X_OK)) {
		if (!stralloc_copy(&pwbuf, &fqemail) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_cats(&pwbuf, pw->pw_passwd) ||
				!stralloc_append(&pwbuf, ":"))
			return (1);
		strnum1[i = fmt_uint(strnum1, pw->pw_uid)] = 0;
		strnum2[j = fmt_uint(strnum2, pw->pw_gid)] = 0;
		if (!stralloc_catb(&pwbuf, strnum1, i) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_catb(&pwbuf, strnum2, j) ||
				!stralloc_append(&pwbuf, ":"))
			return (1);
		strnum1[i = fmt_uint(strnum1, is_inactive)] = 0;
		if (!stralloc_cats(&pwbuf, pw->pw_gecos) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_cats(&pwbuf, pw->pw_dir) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_cats(&pwbuf, pw->pw_shell) ||
				!stralloc_append(&pwbuf, ":") ||
				!stralloc_catb(&pwbuf, strnum1, i) ||
				!stralloc_0(&pwbuf))
			return (1);
		if (!env_put2("PWSTRUCT", pwbuf.s))
			return (1);
		if (!stralloc_copys(&tmpbuf, migrateuser) ||
				!stralloc_append(&tmpbuf, " ") ||
				!stralloc_cat(&tmpbuf, &fqemail) ||
				!stralloc_append(&tmpbuf, " ") ||
				!stralloc_cats(&tmpbuf, pw->pw_dir) ||
				!stralloc_0(&tmpbuf))
			return (1);
		if ((status = runcmmd(tmpbuf.s, 0))) {
			strnum1[fmt_uint(strnum1, status)] = 0;
			strerr_warn9("Login_Tasks: runncmmd: ", migrateuser, " ", fqemail.s, " ", pw->pw_dir, " [status =", strnum1, "]", 0);
			return (status);
		}
	}
	/* - Copy Bulk Mails from Local BULK_MAILDIR directory */
	if (!bulk_mail(fqemail.s, domain, pw->pw_dir))
		flag = 1;
	if (!bulk_mail(fqemail.s, pw->pw_gecos, pw->pw_dir) || flag) {
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir/BulkMail", 17) ||
				!stralloc_0(&tmpbuf))
			return (1);
		close(open(tmpbuf.s, O_CREAT | O_TRUNC , 0644));
	}
	RemoteBulkMail(fqemail.s, domain, pw->pw_dir);
	/*- 
	 * If the age of file QuotaWarn is more than a week send warning
	 * to user if overquota
	 */
	if (str_diffn(pw->pw_shell, "NOQUOTA", 8)) {
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/QuotaWarn", 10) ||
				!stralloc_0(&tmpbuf))
			return (1);
		if ((stat(tmpbuf.s, &statbuf) ? time(0) : time(0) - statbuf.st_mtime) > 7 * 86400)
			MailQuotaWarn(pw->pw_name, domain, Maildir.s, pw->pw_shell);
	}
	/*- Remove Bounce Flag if user is under qutoa */
	if (pw->pw_gid & BOUNCE_MAIL && !user_over_quota(Maildir.s, pw->pw_shell, 0))
		vset_lastdeliver(pw->pw_name, domain, 0);
	return (0);
}
