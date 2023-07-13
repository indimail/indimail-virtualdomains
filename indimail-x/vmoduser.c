/*
 * $Log: vmoduser.c,v $
 * Revision 1.17  2023-03-23 22:20:51+05:30  Cprogrammer
 * removed incorrect call to vdeldomain
 *
 * Revision 1.16  2023-03-22 10:43:02+05:30  Cprogrammer
 * run POST_HANDLE program (if set) with domain user uid/gid
 *
 * Revision 1.15  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.14  2022-11-08 17:17:56+05:30  Cprogrammer
 * removed compiler warning for unused variable
 *
 * Revision 1.13  2022-11-02 12:45:15+05:30  Cprogrammer
 * set usage string depeding on gsasl availability
 *
 * Revision 1.12  2022-10-20 11:59:07+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.11  2022-09-14 13:41:26+05:30  Cprogrammer
 * extract encrypted password from pw->pw_passwd starting with {SCRAM-SHA.*}
 *
 * Revision 1.10  2022-08-28 15:11:23+05:30  Cprogrammer
 * fix compilation error for non gsasl build
 *
 * Revision 1.9  2022-08-25 18:07:49+05:30  Cprogrammer
 * Make password compatible with CRAM and SCRAM
 * 1. store hex-encoded salted password for setting GSASL_SCRAM_SALTED_PASSWORD property in libgsasl
 * 2. store clear text password for CRAM authentication methods
 *
 * Revision 1.8  2022-08-24 18:35:17+05:30  Cprogrammer
 * made setting hash method and scram method independent
 *
 * Revision 1.7  2022-08-07 20:41:18+05:30  Cprogrammer
 * check return value of gsasl_mkpasswd() function
 *
 * Revision 1.6  2022-08-07 13:06:42+05:30  Cprogrammer
 * added option to set scram password
 *
 * Revision 1.5  2022-08-05 21:20:58+05:30  Cprogrammer
 * reversed encrypt_flag setting for mkpasswd() change in encrypt_flag
 *
 * Revision 1.4  2022-01-31 17:36:17+05:30  Cprogrammer
 * fixed args passed to post handle script
 *
 * Revision 1.3  2020-04-01 18:58:53+05:30  Cprogrammer
 * added encrypt flag argument to mkpasswd()
 *
 * Revision 1.2  2019-06-07 15:44:54+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-14 22:41:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define XOPEN_SOURCE = 600
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <sgetopt.h>
#include <env.h>
#include <str.h>
#include <scan.h>
#include <mkpasswd.h>
#include <hashmethods.h>
#include <get_scram_secrets.h>
#include <subfd.h>
#include <setuserid.h>
#endif
#ifdef HAVE_GSASL_H
#include <gsasl.h>
#include "gsasl_mkpasswd.h"
#endif
#include "getpeer.h"
#include "check_group.h"
#include "add_vacation.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "is_distributed_domain.h"
#include "findhost.h"
#include "sql_getpw.h"
#include "sql_setpw.h"
#include "sql_active.h"
#include "vlimits.h"
#include "parse_quota.h"
#include "recalc_quota.h"
#include "check_quota.h"
#include "vset_lastauth.h"
#include "vmake_maildir.h"
#include "deluser.h"
#include "iclose.h"
#include "variables.h"
#include "post_handle.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: vmoduser.c,v 1.17 2023-03-23 22:20:51+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vmoduser: fatal: "
#define WARN    "vmoduser: warning: "

#ifdef ENABLE_AUTH_LOGGING
int             active_inactive = 0;
#endif
static char    *usage =
	"usage: vmoduser [options] email_addr\n"
	"options: -v                       (verbose)\n"
#ifdef ENABLE_AUTH_LOGGING
	"         -n                       (Inactive<->Active Toggle)\n"
#endif
	"         -q quota                 (set quota to quota bytes, +/- to increase/decrease curr value)\n"
	"         -c comment               (set the comment/gecos field)\n"
	"         -P passwd                (set the password field)\n"
	"         -e                       (set the encrypted password field as given by -P option)\n"
	"         -h hash                  (use one of DES, MD5, SHA256, SHA512 hash)\n"
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	"         -m SCRAM method          (use one of SCRAM-SHA-1, SCRAM-SHA-256 SCRAM method)\n"
	"         -C                       (Store clear txt and SCRAM hex salted passowrd in database\n"
	"         -S salt                  (Use a fixed base64 encoded salt)\n"
	"         -I iter_count            (Use iter_count instead of default 4096 for generationg SCRAM password\n"
#else
	"         -C                       (Store clear txt passowrd in database\n"
#endif
#else
	"         -C                       (Store clear txt passowrd in database\n"
#endif
	"         -D date format           (Delivery to a Date Folder)\n"
	"         -l vacation_message_file (sets up Auto Responder)\n"
	"                                  (some special values for vacation_message_file)\n"
	"                                  ('-' to remove vacation, '+' to take mesage from stdin)\n"
	"         -H                       (display this usage)\n\n"
	"the following options are bit flags in the gid int field\n"
	"         -t ( toggle bit flags in the gid int field for below operations )\n"
	"         -u ( set no dialup flag )\n"
	"         -d ( set no password changing flag )\n"
	"         -p ( set no pop access flag )\n"
	"         -w ( set no web mail access flag )\n"
	"         -s ( set no smtp access flag )\n"
	"         -i ( set no imap access flag )\n"
	"         -b ( set bounce mail flag )\n"
	"         -r ( set no external relay flag )\n"
	"         -o ( set domain limit override privileges)\n"
	"         -a ( grant administrator privileges)\n"
	"         -0 ( set V_USER0 flag )\n"
	"         -1 ( set V_USER1 flag )\n"
	"         -2 ( set V_USER2 flag )\n"
	"         -3 ( set V_USER3 flag )\n"
	"         -x ( clear all flags )"
	;

static void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

int
get_options(int argc, char **argv, stralloc *User, stralloc *Email, stralloc *Gecos,
		stralloc *enc_pass, stralloc *DateFormat, stralloc *Quota,
		stralloc *vacation_file, int *toggle, int *GidFlag, int *ClearFlags,
		int *QuotaFlag, int *set_vacation, int *docram, char **clear_text,
		char **salt, int *iter, int *scram)
{
	int             c, i, encrypt_flag = -1;
	char            optstr[56], strnum[FMT_ULONG];

	*toggle = *ClearFlags = 0;
	*QuotaFlag = 0;
	*clear_text = 0;
	if (salt)
		*salt = 0;
	if (docram)
		*docram = 0;
	if (scram)
		*scram = 0;
	if (iter)
		*iter = 4096;
	/*- make sure optstr has enough size to hold all options + 1 */
	i = 0;
	i += fmt_strn(optstr + i, "avutxHD:c:q:dpwisobr0123h:e:l:P:0123456789", 42);
#ifdef ENABLE_AUTH_LOGGING
	i += fmt_strn(optstr + i, "nC", 2);
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	i += fmt_strn(optstr + i, "m:S:I:", 6);
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
		case 't':
			*toggle = 1;
			break;
#ifdef ENABLE_AUTH_LOGGING
		case 'n':
			active_inactive = 1;
#endif
			break;
		case 'x':
			*ClearFlags = 1;
			break;
		case 'e':
			/*- ignore encrypt flag option if -h option is provided */
			if (encrypt_flag == -1)
				encrypt_flag = 0;
			/*- flow through */
		case 'P':
			mkpasswd(*clear_text = optarg, enc_pass, encrypt_flag);
			if (verbose) {
				subprintfe(subfderr, "vmoduser", "encrypted password set as %s\n", enc_pass->s);
				errflush("vmoduser");
			}
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
			else
				strerr_die5x(100, WARN, "wrong hash method ", optarg, ". Supported HASH Methods: DES MD5 SHA-256 SHA-512\n", usage);
			if (!env_put2("PASSWORD_HASH", strnum))
				strerr_die1x(111, "out of memory");
			encrypt_flag = 1;
			break;
		case 'C':
			if (docram)
				*docram = 1;
			break;
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
		case 'm':
			if (!scram)
				break;
			if (!str_diffn(optarg, "SCRAM-SHA-1", 11))
				*scram = 1;
			else
			if (!str_diffn(optarg, "SCRAM-SHA-256", 13))
				*scram = 2;
			else
				strerr_die5x(100, WARN, "wrong SCRAM method ", optarg, ". Supported SCRAM Methods: SCRAM-SHA1 SCRAM-SHA-256\n", usage);
			break;
		case 'S':
			if (!salt)
				break;
			i = str_chr(optarg, ',');
			if (optarg[i])
				strerr_die3x(100, WARN, optarg, ": salt cannot have a comma character");
			*salt = optarg;
			break;
		case 'I':
			if (!iter)
				break;
			scan_int(optarg, iter);
			break;
#endif
#endif
		case 'D':
			if (!stralloc_copys(DateFormat, optarg) ||
					!stralloc_0(DateFormat))
				die_nomem();
			DateFormat->len--;
			break;
		case 'l':
			if (!stralloc_copys(vacation_file, optarg) ||
					!stralloc_0(vacation_file))
				die_nomem();
			vacation_file->len--;
			*set_vacation = 1;
			break;
		case 'c':
			if (!stralloc_copys(Gecos, optarg) ||
					!stralloc_0(Gecos))
				die_nomem();
			Gecos->len--;
			break;
		case 'q':
			*QuotaFlag = 1;
			if (!stralloc_copys(Quota, optarg) ||
					!stralloc_0(Quota))
				die_nomem();
			Quota->len--;
			break;
		case 'd':
			*GidFlag |= NO_PASSWD_CHNG;
			break;
		case 'p':
			*GidFlag |= NO_POP;
			break;
		case 's':
			*GidFlag |= NO_SMTP;
			break;
		case 'o':
			*GidFlag |= V_OVERRIDE;
			break;
		case 'w':
			*GidFlag |= NO_WEBMAIL;
			break;
		case 'i':
			*GidFlag |= NO_IMAP;
			break;
		case 'b':
			*GidFlag |= BOUNCE_MAIL;
			break;
		case 'r':
			*GidFlag |= NO_RELAY;
			break;
		case 'u':
			*GidFlag |= NO_DIALUP;
			break;
		case '0':
			*GidFlag |= V_USER0;
			break;
		case '1':
			*GidFlag |= V_USER1;
			break;
		case '2':
			*GidFlag |= V_USER2;
			break;
		case '3':
			*GidFlag |= V_USER3;
			break;
		case 'a':
			*GidFlag |= QA_ADMIN;
			break;
		case 'H':
		default:
			strerr_die2x(100, WARN, usage);
		}
	}
	if (optind < argc) {
		if (!stralloc_copys(Email, argv[optind++]) ||
				!stralloc_0(Email))
			die_nomem();
		Email->len--;
	}
	if (!Email->len)
		strerr_die2x(100, WARN, usage);
	if (encrypt_flag == -1)
		encrypt_flag = 1;
	return (0);
}

int
main(int argc, char **argv)
{
	static stralloc Email = {0}, User = {0}, Domain = {0}, Gecos = {0},
					enc_pass = {0}, DateFormat = {0}, Quota = {0}, vacation_file = {0},
					tmpbuf = {0}, tmpQuota = {0};
	int             GidFlag = 0, QuotaFlag = 0, toggle = 0, ClearFlags,
					set_vacation = 0, err, fd, i;
	uid_t           uid, domainuid;
	gid_t           gid, domaingid;
	struct passwd   PwTmp;
	struct passwd  *pw;
	char           *real_domain, *ptr;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	mdir_t          quota = 0, ul;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit, mailcount;
#endif
#ifdef ENABLE_DOMAIN_LIMITS
	static stralloc TheDir = {0};
	struct vlimits  limits;
	int             domain_limits, docram;
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	static stralloc result = {0};
	char           *b64salt, *clear_text;
	int             scram, iter;
#endif
#endif

#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	if (get_options(argc, argv, &User, &Email, &Gecos, &enc_pass,
			&DateFormat, &Quota, &vacation_file, &toggle, &GidFlag, &ClearFlags,
			&QuotaFlag, &set_vacation, &docram, &clear_text, &b64salt, &iter, &scram))
		return (1);
#else
	if (get_options(argc, argv, &User, &Email, &Gecos, &enc_pass,
			&DateFormat, &Quota, &vacation_file, &toggle, &GidFlag, &ClearFlags,
			&QuotaFlag, &set_vacation, &docram, 0, 0, 0, 0))
		return (1);
#endif
#else
	if (get_options(argc, argv, &User, &Email, &Gecos, &enc_pass,
			&DateFormat, &Quota, &vacation_file, &toggle, &GidFlag, &ClearFlags,
			&QuotaFlag, &set_vacation, docram, 0, 0, 0, 0))
		return (1);
#endif
	parse_email(Email.s, &User, &Domain);
	if (!(real_domain = get_real_domain(Domain.s)))
		strerr_die3x(1, WARN, Domain.s, ": No such domain");
	if (!get_assign(real_domain, &TheDir, &domainuid, &domaingid))
		strerr_die3x(1, WARN, real_domain, ": domain does not exist");
	if (!domainuid)
		strerr_die4x(100, WARN, "domain ", real_domain, " with uid 0");
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != domainuid && gid != domaingid && check_group(domaingid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, domainuid)] = 0;
		strnum2[fmt_ulong(strnum2, domaingid)] = 0;
		strerr_die6x(100, WARN, "you must be root or domain user (uid=", strnum1, ", gid=", strnum2, ") to run this program");
	}
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	if (clear_text) {
		switch (scram)
		{
		case 1: /*- SCRAM-SHA-1 */
			if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-1", iter, b64salt, docram, clear_text, &result)) != NO_ERR)
				strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
			break;
		case 2: /*- SCRAM-SHA-256 */
			if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-256", iter, b64salt, docram, clear_text, &result)) != NO_ERR)
				strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
			break;
		}
	}
#endif
#endif
	if (QuotaFlag == 1 || set_vacation) {
		if (uid && setuid(0))
			strerr_die2sys(111, FATAL, "setuid-root: ");
	} else
	if (uid && setuid(uid)) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strerr_die4sys(111, FATAL, "setuid-", strnum1, ": ");
	}
	if (set_vacation)
		return (add_vacation(Email.s, vacation_file.s));
#ifdef ENABLE_DOMAIN_LIMITS
	if (!stralloc_copy(&tmpbuf, &TheDir) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	domain_limits = ((access(tmpbuf.s, F_OK) && !env_get("DOMAIN_LIMITS")) ? 0 : 1);
#endif
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1)
		strerr_die4x(1, WARN, "Unable to verify ", real_domain, " as a distributed domain");
	else
	if (err == 1) {
		if (!stralloc_copy(&tmpbuf, &User) ||
				!stralloc_append(&tmpbuf, "@") ||
				!stralloc_cats(&tmpbuf, real_domain) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (!findhost(tmpbuf.s, 1))
			strerr_die5x(1, WARN, "no such user ", User.s, "@", real_domain);
	}
#endif
	if (!(pw = sql_getpw(User.s, real_domain)))
		strerr_die5x(1, WARN, "no such user ", User.s, "@", real_domain);
#ifdef ENABLE_DOMAIN_LIMITS
	if (!(pw->pw_gid & V_OVERRIDE) && domain_limits) {
		if (vget_limits(real_domain, &limits))
			strerr_die3x(1, WARN, "Unable to get domain limits for ", real_domain);
		if (QuotaFlag && (limits.perm_defaultquota & VLIMIT_DISABLE_MODIFY))
			strerr_die3x(1, WARN, "quota modification not allowed for ", Email.s);
	}
#endif
	PwTmp = *pw; /*- structure copy */
	pw = &PwTmp;
	if (Gecos.len)
		pw->pw_gecos = Gecos.s;
	if (enc_pass.len)
		pw->pw_passwd = enc_pass.s;
	if (!str_diffn(pw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(pw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(pw->pw_passwd, 0, 0, 0, 0, 0, 0, 0, &ptr);
		if (i != 6 && i != 8)
			strerr_die2x(1, WARN, "unable to get secrets");
		pw->pw_passwd = ptr;
	}
	if (ClearFlags == 1)
		pw->pw_gid = 0;
	else
	if (GidFlag != 0) {
		if (toggle)
			pw->pw_gid ^= GidFlag;
		else
			pw->pw_gid |= GidFlag;
	}
	if (QuotaFlag == 1) {
		if (!str_diffn(Quota.s, "NOQUOTA", 8))
			pw->pw_shell = "NOQUOTA";
		else {
			if ((*Quota.s == '+') || (*Quota.s == '-')) {
				if ((ul = parse_quota(pw->pw_shell, 0)) == -1)
					strerr_die4x(1, WARN, "invalid quota specification [", pw->pw_shell, "]");
				quota += ul;
				if ((ul = parse_quota(Quota.s, 0)) == -1)
					strerr_die4x(1, WARN, "invalid quota specification [", pw->pw_shell, "]");
				quota += ul;
				strnum1[i = fmt_ulong(strnum1, quota)] = 0;
				if (!stralloc_copyb(&tmpQuota, strnum1, i))
					die_nomem();
			} else {
				if ((ul = parse_quota(Quota.s, 0)) == -1)
					strerr_die4x(1, WARN, "invalid quota specification [", pw->pw_shell, "]");
				strnum1[i = fmt_ulong(strnum1, ul)] = 0;
				if (!stralloc_copyb(&tmpQuota, strnum1, i))
					die_nomem();
			}
			i = str_rchr(pw->pw_shell, ',');
			if (pw->pw_shell[i]) {
				if (!stralloc_cats(&tmpQuota, pw->pw_shell + i + 1))
					die_nomem();
			}
			if (!stralloc_0(&tmpQuota))
				die_nomem();
			pw->pw_shell = tmpQuota.s;
		}
	}
	err = 0;
#ifdef ENABLE_AUTH_LOGGING
	if (active_inactive == 1) {
		if (is_inactive) {
			err = sql_active(pw, real_domain, FROM_INACTIVE_TO_ACTIVE);
			vmake_maildir(pw->pw_dir, uid, gid, real_domain);
		} else
			err = deluser(User.s, real_domain, 2);
	}
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	ptr = scram ? result.s : 0;
#else
	ptr = (char *) NULL;
#endif
#else
	ptr = (char *) NULL;
#endif
	if ((Gecos.len || enc_pass.len || ClearFlags || GidFlag || QuotaFlag) &&
			(err = sql_setpw(pw, real_domain, ptr)))
		strerr_die2x(1, FATAL, "sql_setpw failed");
	if (!err && QuotaFlag == 1) {
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
#ifdef USE_MAILDIRQUOTA
		if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1)
			strerr_die4sys(111, WARN, "parse_quota: ", pw->pw_shell, ": ");
		quota = recalc_quota(tmpbuf.s, &mailcount, size_limit, count_limit, 2);
#else
		quota = recalc_quota(tmpbuf.s, 2);
#endif
	}
	if (enc_pass.len) {
		if (!quota) {
			if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
					!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
#ifdef USE_MAILDIRQUOTA
			quota = check_quota(tmpbuf.s, 0);
#else
			quota = check_quota(tmpbuf.s);
#endif
		}
#ifdef ENABLE_AUTH_LOGGING
		vset_lastauth(pw->pw_name, real_domain, "pass", GetPeerIPaddr(), pw->pw_gecos, quota);
#endif
	}
	if (DateFormat.len) {
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir/folder.dateformat", 26) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, INDIMAIL_QMAIL_MODE)) == -1)
			strerr_die4sys(111, FATAL, "open", tmpbuf.s, ": ");
		if (fchown(fd, uid, gid)) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn8(FATAL, "fchown: ", tmpbuf.s, ": (uid ", strnum1, ", gid ", strnum2, "): ", &strerr_sys);
			close(fd);
			iclose();
			return (1);
		}
		if (write(fd, DateFormat.s, DateFormat.len) == -1) {
			strerr_warn4(FATAL, "write: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			iclose();
			return (1);
		}
		close(fd);
	}
	iclose();
	if (!err) {
		for (i = 1, tmpbuf.len = 0; i < argc; i++) {
			if (!stralloc_append(&tmpbuf, " ") ||
					!stralloc_cats(&tmpbuf, argv[i]))
				die_nomem();
		}
		if (!stralloc_0(&tmpbuf))
			die_nomem();
		if (!(ptr = env_get("POST_HANDLE"))) {
			char           *base_argv0;

			i = str_rchr(argv[0], '/');
			if (!*(base_argv0 = (argv[0] + i)))
				base_argv0 = argv[0];
			else
				base_argv0++;
			return (post_handle("%s/%s%s", LIBEXECDIR, base_argv0, tmpbuf.s));
		} else {
			if (setuser_privileges(domainuid, domaingid, "indimail")) {
				strnum1[fmt_ulong(strnum1, domainuid)] = 0;
				strnum2[fmt_ulong(strnum2, domaingid)] = 0;
				strerr_die6sys(111, FATAL, "setuser_privilege: (", strnum1, "/", strnum2, "): ");
			}
			return (post_handle("%s%s", ptr, tmpbuf.s));
		}
	}
	return (err);
}
