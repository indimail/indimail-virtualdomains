/*
 * $Log: vmoduser.c,v $
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
#include <mkpasswd.h>
#endif
#include "getpeer.h"
#include "get_indimailuidgid.h"
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
#include "deldomain.h"
#include "iclose.h"
#include "variables.h"
#include "post_handle.h"

#ifndef	lint
static char     sccsid[] = "$Id: vmoduser.c,v 1.3 2020-04-01 18:58:53+05:30 Cprogrammer Exp mbhangui $";
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
	"         -e encrypted_passwd      (set the encrypted password field)\n"
	"         -D date format           (Delivery to a Date Folder)\n"
	"         -l vacation_message_file (sets up Auto Responder)\n"
	"                                  (some special values for vacation_message_file)\n"
	"                                  ('-' to remove vacation, '+' to take mesage from stdin)\n"
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
	strerr_warn1("vmoduser: out of memory", 0);
	_exit(111);
}

int
get_options(int argc, char **argv, stralloc *User, stralloc *Email, stralloc *Domain, stralloc *Gecos,
		stralloc *Passwd, stralloc *DateFormat, stralloc *Quota, stralloc *vacation_file, 
		int *toggle, int *GidFlag, int *ClearFlags, int *QuotaFlag, int *set_vacation)
{
	int             c;

	*toggle = *ClearFlags = 0;
	*QuotaFlag = 0;
#ifdef ENABLE_AUTH_LOGGING
	while ((c = getopt(argc, argv, "avutnxD:c:q:dpwisobr0123he:l:P:")) != opteof) 
#else
	while ((c = getopt(argc, argv, "avuxD:c:q:dpwisobr0123he:l:P:")) != opteof) 
#endif
	{
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
			encrypt_flag = 1;
			/*- flow through */
		case 'P':
			mkpasswd(optarg, Passwd, encrypt_flag);
			break;
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
		case 'h':
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc) {
		if (!stralloc_copys(Email, argv[optind++]) ||
				!stralloc_0(Email))
			die_nomem();
		Email->len--;
	}
	if (!Email->len) {
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
	static stralloc Email = {0}, User = {0}, Domain = {0}, Gecos = {0},
					Passwd = {0}, DateFormat = {0}, Quota = {0}, vacation_file = {0},
					tmpbuf = {0}, tmpQuota = {0};
	int             GidFlag = 0, QuotaFlag = 0, toggle = 0, ClearFlags,
					set_vacation = 0, err, fd, i;
	uid_t           uid;
	gid_t           gid;
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
	int             domain_limits;
#endif

	if (get_options(argc, argv, &User, &Email, &Domain, &Gecos, &Passwd,
			&DateFormat, &Quota, &vacation_file, &toggle, &GidFlag, &ClearFlags, &QuotaFlag, &set_vacation))
		return (1);
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
		strerr_warn5("vmoduser: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	if (QuotaFlag == 1 || set_vacation) {
		if (uid && setuid(0)) {
			strerr_warn1("vmoduser: setuid-root: ", &strerr_sys);
			return (1);
		}
	} else
	if (setuid(uid)) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strerr_warn3("vmoduser: setuid-", strnum1, ": ", &strerr_sys);
		return (1);
	}
	if (set_vacation)
		return (add_vacation(Email.s, vacation_file.s));
	parse_email(Email.s, &User, &Domain);
	if (!(real_domain = get_real_domain(Domain.s))) {
		strerr_warn3("vmoduser: ", Domain.s, ": No such domain", 0);
		return (1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (!get_assign(real_domain, &TheDir, 0, 0)) {
		strerr_warn3("vmoduser: ", real_domain, ": domain does not exist", 0);
		return (1);
	}
	if (!stralloc_copy(&tmpbuf, &TheDir) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	domain_limits = ((access(tmpbuf.s, F_OK) && !env_get("DOMAIN_LIMITS")) ? 0 : 1);
#endif
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1) {
		strerr_warn4(WARN, "vmoduserr: Unable to verify ", real_domain, " as a distributed domain", 0);
		return (1);
	} else
	if (err == 1) {
		if (!stralloc_copy(&tmpbuf, &User) ||
				!stralloc_append(&tmpbuf, "@") ||
				!stralloc_cats(&tmpbuf, real_domain) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (!findhost(tmpbuf.s, 1)) {
			strerr_warn4("vmoduser: no such user ", User.s, "@", real_domain, 0);
			return (1);
		}
	}
#endif
	if (!(pw = sql_getpw(User.s, real_domain))) {
		strerr_warn4("vmoduser: no such user ", User.s, "@", real_domain, 0);
		return (1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (!(pw->pw_gid & V_OVERRIDE) && domain_limits) {
		if (vget_limits(real_domain, &limits)) {
			strerr_warn2("vmoduser: Unable to get domain limits for ", real_domain, 0);
			return (1);
		}
		if (QuotaFlag && (limits.perm_defaultquota & VLIMIT_DISABLE_MODIFY)) {
			strerr_warn2("vmoduser: quota modification not allowed for ", Email.s, 0);
			return (1);
		}
	}
#endif
	PwTmp = *pw; /*- structure copy */
	pw = &PwTmp;
	if (Gecos.len)
		pw->pw_gecos = Gecos.s;
	if (Passwd.len)
		pw->pw_passwd = Passwd.s;
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
				if ((ul = parse_quota(pw->pw_shell, 0)) == -1) {
					strerr_warn3("vmoduser: invalid quota specification [", pw->pw_shell, "]", 0);
					return (1);
				}
				quota += ul;
				if ((ul = parse_quota(Quota.s, 0)) == -1) {
					strerr_warn3("vmoduser: invalid quota specification [", pw->pw_shell, "]", 0);
					return (1);
				}
				quota += ul;
				strnum1[i = fmt_ulong(strnum1, quota)] = 0;
				if (!stralloc_copyb(&tmpQuota, strnum1, i))
					die_nomem();
			} else {
				if ((ul = parse_quota(Quota.s, 0)) == -1) {
					strerr_warn3("vmoduser: invalid quota specification [", pw->pw_shell, "]", 0);
					return (1);
				}
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
			if (!get_assign(real_domain, 0, &uid, &gid)) {
				if (indimailuid == -1 || indimailgid == -1)
					get_indimailuidgid(&indimailuid, &indimailgid);
				uid = indimailuid;
				gid = indimailgid;
			}
			vmake_maildir(pw->pw_dir, uid, gid, real_domain);
		} else
			err = deluser(User.s, real_domain, 2);
	}
#endif
	if ((Gecos.len || Passwd.len || ClearFlags || GidFlag || QuotaFlag) && (err = sql_setpw(pw, real_domain))) {
		strerr_warn1("vmoduser: sql_setpw failed", 0);
		return (1);
	}
	if (!err && QuotaFlag == 1) {
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
#ifdef USE_MAILDIRQUOTA
		if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1) {
			strerr_warn3("vmoduser: parse_quota: ", pw->pw_shell, ": ", &strerr_sys);
			return (1);
		}
		quota = recalc_quota(tmpbuf.s, &mailcount, size_limit, count_limit, 2);
#else
		quota = recalc_quota(tmpbuf.s, 2);
#endif
	}
	if (Passwd.len) {
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
		if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, INDIMAIL_QMAIL_MODE)) == -1) {
			strerr_warn3("vmoduser: open", tmpbuf.s, ": ", &strerr_sys);
			return (1);
		}
		if (!get_assign(real_domain, 0, &uid, &gid)) {
			if (indimailuid == -1 || indimailgid == -1)
				get_indimailuidgid(&indimailuid, &indimailgid);
			uid = indimailuid;
			gid = indimailgid;
		}
		if (fchown(fd, uid, gid)) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn7("vmoduser: fchown: ", tmpbuf.s, ": (uid ", strnum1, ", gid ", strnum2, "): ", &strerr_sys);
			deldomain(Domain.s);
			iclose();
			close(fd);
			return (1);
		}
		if (write(fd, DateFormat.s, DateFormat.len) == -1) {
			strerr_warn3("vmoduser: write: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return (1);
		}
		close(fd);
	}
	iclose();
	if (!err) {
		for (i = 1, tmpbuf.len = 0; i < argc; i++) {
			if (!stralloc_append(&tmpbuf, " ") ||
					!stralloc_cats(&tmpbuf, argv[i]) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
		}
		if (!(ptr = env_get("POST_HANDLE"))) {
			char           *base_argv0;

			i = str_rchr(argv[0], '/');
			if (!*(base_argv0 = (argv[0] + i)))
				base_argv0 = argv[0];
			else
				base_argv0++;
			return (post_handle("%s/%s%s", LIBEXECDIR, base_argv0, tmpbuf.s));
		} else
			return (post_handle("%s%s", ptr, tmpbuf.s));
	}
	return (err);
}
