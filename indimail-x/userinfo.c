/*
 * $Log: userinfo.c,v $
 * Revision 1.10  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.9  2022-08-28 12:03:43+05:30  Cprogrammer
 * fixed display string for DES/un-encrypted password
 *
 * Revision 1.8  2022-08-04 14:42:22+05:30  Cprogrammer
 * display scram password if existing
 *
 * Revision 1.7  2021-08-21 12:52:33+05:30  Cprogrammer
 * moved no_of_days.h to libqmail
 *
 * Revision 1.6  2021-07-22 15:17:31+05:30  Cprogrammer
 * conditional define of _XOPEN_SOURCE
 *
 * Revision 1.5  2020-10-13 18:35:14+05:30  Cprogrammer
 * initialize struct tm for strptime() value too big error
 *
 * Revision 1.4  2020-04-01 18:58:19+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.3  2019-06-07 16:10:48+05:30  mbhangui
 * fix for missing mysql_get_option() in new versions of libmariadb
 *
 * Revision 1.2  2019-04-17 17:54:35+05:30  Cprogrammer
 * display db server info only for root
 *
 * Revision 1.1  2019-04-14 18:30:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIME_H
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#define XOPEN_SOURCE = 600
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <mysql.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <scan.h>
#include <error.h>
#include <open.h>
#include <getln.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <getEnvConfig.h>
#include <no_of_days.h>
#endif
#include "valiasinfo.h"
#include "relay_select.h"
#ifdef VFILTER
#include "vfilter_display.h"
#endif
#include "SqlServer.h"
#include "vget_lastauth.h"
#include "common.h"
#include "parse_quota.h"
#include "recalc_quota.h"
#include "check_quota.h"
#include "islocalif.h"
#include "sql_getpw.h"
#include "sql_gethostid.h"
#include "get_local_ip.h"
#include "iopen.h"
#include "findhost.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "is_distributed_domain.h"
#include "isvirtualdomain.h"
#include "variables.h"
#include "userinfo.h"

#ifndef	lint
static char     sccsid[] = "$Id: userinfo.c,v 1.10 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
#endif

extern char *strptime(const char *, const char *, struct tm *);

static void
die_nomem()
{
	strerr_warn1("userinfo: out of memory", 0);
	_exit(111);
}

int
userinfo(Email, User, Domain, DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir, 
		  DisplayQuota, DisplayLastAuth, DisplayFilter, DisplayAll)
	char           *Email, *User, *Domain;
	int             DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir, DisplayQuota, 
	                DisplayLastAuth, DisplayFilter, DisplayAll;
{
	struct passwd  *mypw;
	char           *ptr, *real_domain, *mailstore, *sysconfdir, *controldir, *passwd_hash;
	MYSQL          *mptr;
#ifdef CLUSTERED_SITE
	int             is_dist = 0;
#endif
	struct substdio ssin;
	char            inbuf[4096];
	int             i, j, fd, use_ssl = 0, islocal = 1, match;
	static stralloc maildir = {0}, line = {0}, tmpbuf = {0};
	uid_t           uid;
	gid_t           gid;
	mdir_t          cur_size, mcount;
#ifdef ENABLE_AUTH_LOGGING
	static stralloc timeBuf = {0};
	mdir_t          delivery_count, delivery_size;
	char            ipaddr[7][18];
	time_t          add_time, auth_time, pwdchg_time, inact_time, act_time, pop3_time, imap_time, delivery_time;
	struct tm       tm = {0};
#endif

	real_domain = (char *) 0;
	if ((Domain && *Domain) && !(real_domain = get_real_domain(Domain))) {
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		if (*controldir == '/') {
			if (!stralloc_copys(&tmpbuf, controldir) ||
					!stralloc_catb(&tmpbuf, "/rcpthosts", 10) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
		}
		else {
			getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
			if (!stralloc_copys(&tmpbuf, sysconfdir) ||
					!stralloc_append(&tmpbuf, "/") ||
					!stralloc_cats(&tmpbuf, controldir) ||
					!stralloc_catb(&tmpbuf, "/rcpthosts", 10) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
		}
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno == error_noent) {
				strerr_warn1("userinfo: no domains found. rcpthosts missing", 0);
				return (1);
			}
			strerr_warn3("userinfo: ", tmpbuf.s, ": ", &strerr_sys);
			return (1);
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for(;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("userinfo: read: ", tmpbuf.s, ": ", &strerr_sys);
				break;
			}
			if (!match && line.len == 0)
				break;
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
			match = str_chr(line.s, '#');
			if (line.s[match])
				line.s[match] = 0;
			for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			line.s[line.len - 1] = 0;
			if (!str_diffn(line.s, Domain, line.len + 1)) {
				real_domain = Domain;
				break;
			}
		}
		close(fd);
	}
#ifdef CLUSTERED_SITE
	if ((is_dist = is_distributed_domain(real_domain)) == -1) {
		strerr_warn3("Unable to verify ", Domain, " as a distributed domain", 0);
		return (1);
	}
	if (real_domain && *real_domain) {
		if (is_dist == 1) {
			if (!stralloc_copys(&tmpbuf, User) ||
					!stralloc_append(&tmpbuf, "@") ||
					!stralloc_cats(&tmpbuf, real_domain) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			if ((mailstore = findhost(tmpbuf.s, 1)) != (char *) 0) {
				i = str_rchr(mailstore, ':');
				if (mailstore[i]) {
					mailstore[i] = 0;
					ptr = mailstore;
					for(;*mailstore && *mailstore != ':';mailstore++);
					if (*mailstore == ':')
						mailstore++;
					else {
						mailstore[i] = ':';
						strerr_warn3("findhost: invalid route spec [", ptr, "]", 0);
						return (1);
					}
				} else {
					strerr_warn3("findhost: invalid route spec [", mailstore, "]", 0);
					return (1);
				}
			} else {
				if (userNotFound)
					strerr_warn4(User, "@", Domain, ": No such user", 0);
				else
					strerr_warn1("internal system error", 0);
				return (1);
			}
		} else {
			if (iopen((char *) 0)) {
				strerr_warn1("iopen failed: failed to connect to db", 0);
				return (1);
			}
			mailstore = MdaServer(mysql_host.s, real_domain);
			if (!mailstore && (!str_diffn(mysql_host.s, "localhost", 10) || !str_diffn(mysql_host.s, "127.0.0.1", 10))) {
				if ((ptr = get_local_ip(PF_INET))) {
					if (!(mailstore = MdaServer(ptr, real_domain)))
						mailstore = ptr;
				}
			}
			if (!mailstore)
				mailstore = "unknown";
		}
	} else
		mailstore = "localhost";
	islocal = islocalif(mailstore);
#else
	mailstore = "localhost";
#endif
	if (!(mypw = sql_getpw(User, real_domain))) {
		if (!real_domain && !isvirtualdomain(User) && get_assign(User, &tmpbuf, &uid, &gid)) {
			if (DisplayName || DisplayAll)
				subprintfe(subfdout, "userinfo", "name          : %s\n", User);
			if (DisplayUid || DisplayAll)
				subprintfe(subfdout, "userinfo", "uid           : %u\n", uid);
			if (DisplayGid || DisplayAll)
				subprintfe(subfdout, "userinfo", "uid           : %u\n", gid);
			if (DisplayDir || DisplayAll)
				subprintfe(subfdout, "userinfo", "dir           : %s %s", tmpbuf.s,
						access(tmpbuf.s, F_OK) && errno == ENOENT ? (islocal ? " (missing)\n" : " (remote)\n") : "\n");
			flush("userinfo");
			return (0);
		} else
		if (valiasinfo(User, real_domain)) {
			flush("userinfo");
			return (0);
		}
		if (Domain && *Domain)
			strerr_warn4(User, "@", Domain, ": No such user", 0);
		else
			strerr_warn2(User, ": No such user", 0);
		flush("userinfo");
		return (1);
	}
	if (!str_diffn(mypw->pw_passwd, "{SCRAM-SHA-1}", 13))
		passwd_hash = "SCRAM-SHA-1";
	else
	if (!str_diffn(mypw->pw_passwd, "{SCRAM-SHA-256}", 15))
		passwd_hash = "SCRAM-SHA-256";
	else
	if (mypw->pw_passwd[0] == '$' && mypw->pw_passwd[2] == '$') {
		switch(mypw->pw_passwd[1])
		{
			case '1':
				passwd_hash = "MD5";
				break;
			case '5':
				passwd_hash = "SHA256";
				break;
			case '6':
				passwd_hash = "SHA512";
				break;
			default:
				passwd_hash = "$?$";
				break;
		}
	} else {
		i = str_len(mypw->pw_passwd);
		passwd_hash = i == 13 ? "DES/cleartxt" : "un-encrypted";
	}
	if (DisplayName || DisplayAll) {
		subprintfe(subfdout, "userinfo", "name          : %s@%s\n", mypw->pw_name,
				(Domain && *Domain) ? real_domain : "localhost");
	}
	if (DisplayPasswd || DisplayAll)
		subprintfe(subfdout, "userinfo", "passwd        : %s (%s)\n", mypw->pw_passwd, passwd_hash);
	if (DisplayUid || DisplayAll)
		subprintfe(subfdout, "userinfo", "uid           : %u\n", mypw->pw_uid);
	if (DisplayGid || DisplayAll) {
		subprintfe(subfdout, "userinfo", "gid           : %u\n", mypw->pw_gid);
		if (mypw->pw_gid == 0)
			subprintfe(subfdout, "userinfo", "                -all services available\n");
		if (mypw->pw_gid & NO_PASSWD_CHNG)
			subprintfe(subfdout, "userinfo", "                -password can not be changed by user\n");
		if (mypw->pw_gid & NO_POP)
			subprintfe(subfdout, "userinfo", "                -pop access closed\n");
		if (mypw->pw_gid & NO_WEBMAIL)
			subprintfe(subfdout, "userinfo", "                -webmail access closed\n");
		if (mypw->pw_gid & NO_IMAP)
			subprintfe(subfdout, "userinfo", "                -imap access closed\n");
		if (mypw->pw_gid & NO_SMTP)
			subprintfe(subfdout, "userinfo", "                -smtp access closed\n");
		if (mypw->pw_gid & BOUNCE_MAIL)
			subprintfe(subfdout, "userinfo", "                -mail will be bounced back to sender\n");
		if (mypw->pw_gid & NO_RELAY)
			subprintfe(subfdout, "userinfo", "                -user not allowed to relay mail\n");
		if (mypw->pw_gid & NO_DIALUP)
			subprintfe(subfdout, "userinfo", "                -no dialup flag has been set\n");
		if (mypw->pw_gid & QA_ADMIN)
			subprintfe(subfdout, "userinfo", "                -has qmailadmin administrator privileges\n");
		if (mypw->pw_gid & V_OVERRIDE)
			subprintfe(subfdout, "userinfo", "                -has domain limit skip privileges\n");
		if (mypw->pw_gid & V_USER0)
			subprintfe(subfdout, "userinfo", "                -user flag 0 is set\n");
		if (mypw->pw_gid & V_USER1)
			subprintfe(subfdout, "userinfo", "                -user flag 1 is set\n");
		if (mypw->pw_gid & V_USER2)
			subprintfe(subfdout, "userinfo", "                -user flag 2 is set\n");
		if (mypw->pw_gid & V_USER3)
			subprintfe(subfdout, "userinfo", "                -user flag 3 is set\n");
	}
	if (DisplayComment || DisplayAll)
		subprintfe(subfdout, "userinfo", "gecos         : %s\n", mypw->pw_gecos);
	if (DisplayDir || DisplayAll)
		subprintfe(subfdout, "userinfo", "dir           : %s %s", mypw->pw_dir,
				access(mypw->pw_dir, F_OK) && errno == error_noent ? (islocal ? " (missing)\n" : " (remote)\n") : "\n");
	if (DisplayQuota || DisplayAll) {
		mdir_t          dquota;
#ifdef USE_MAILDIRQUOTA	
		mdir_t          size_limit, count_limit;
#endif

		if (!str_diffn(mypw->pw_shell, "NOQUOTA", 8))
			subprintfe(subfdout, "userinfo", "quota         : unlimited\n");
		else {
			dquota = parse_quota(mypw->pw_shell, 0)/1048576;
			subprintfe(subfdout, "userinfo", "quota         : %s [%.2f] %s\n",
					mypw->pw_shell,
					dquota < 1024 ? (float) dquota : (float) (dquota/1024),
					dquota < 1024 ? "MiB" : "GiB");
		}
		if (islocal) {
			if (!stralloc_copys(&maildir, mypw->pw_dir) ||
					!stralloc_catb(&maildir, "/Maildir", 8) ||
					!stralloc_0(&maildir))
				die_nomem();
#ifdef USE_MAILDIRQUOTA	
			if ((size_limit = parse_quota(mypw->pw_shell, &count_limit)) == -1)
				cur_size = mcount = -1;
			else
			if ((cur_size = recalc_quota(maildir.s, &mcount, size_limit, count_limit, 2)) == -1) {
				if (errno != error_acces) {
					strerr_warn1("userinfo: recalc_quota: ", &strerr_sys);
					return (1);
				}
				cur_size = check_quota(maildir.s, &mcount);
			}
			subprintfe(subfdout, "userinfo", "curr quota    : %ldS,%ldC\n",
					cur_size == -1 ? 0 : cur_size, mcount == -1 ? 0 : mcount);
#else
			if ((cur_size = recalc_quota(maildir, 2)) == -1) {
				strerr_warn1("userinfo: recalc_quota: ", &strerr_sys);
				flush("userinfo");
				return (1);
			}
			subprintfe(subfdout, "userinfo", "curr quota    : %ld\n", cur_size == -1 ? 0 : cur_size);
#endif
		} else
			subprintfe(subfdout, "userinfo", "curr quota    : unknown (remote)\n");
	}
	if (DisplayAll) {
#ifdef CLUSTERED_SITE
		subprintfe(subfdout, "userinfo", "Mail Store IP : %s (%s - %s)\n", mailstore,
				is_dist ? "Clustered" : "NonClustered",
				islocal ? "local" : "remote");
		if (is_dist)
			ptr = sql_gethostid(mailstore);
		else
			ptr = "non-clustered domain";
		subprintfe(subfdout, "userinfo", "Mail Store ID : %s\n", ptr ? ptr : "no id present");
#else
		subprintfe(subfdout, "userinfo", "Mail store    : %s\n", mailstore);
#endif
#ifdef CLUSTERED_SITE
		if (!(ptr = SqlServer(mailstore, real_domain))) {
			if (!is_dist)
				ptr = mysql_host.s;
			else
				ptr = "unknown";
		}
		subprintfe(subfdout, "userinfo", "Sql Database  : ");
		if (!getuid() || !geteuid())
			out("userinfo",  ptr);
		else {
			for (; *ptr && *ptr != ':'; ptr++)
				if (substdio_put(subfdout, ptr, 1))
					strerr_die1sys(111, "userinfo: write: ");
		}
		subprintfe(subfdout, "userinfo", "\n");
#else
		subprintfe(subfdout, "userinfo", "Sql Database  : %s\n", mysql_host.s);
#endif
		mptr = &mysql[1];
		if (mptr->unix_socket)
			subprintfe(subfdout, "userinfo", "Unix   Socket : %s\n", mptr->unix_socket);
		else {
			subprintfe(subfdout, "userinfo", "TCP/IP Port   : %u\n", mptr->port);
#if MYSQL_VERSION_ID >= 50703 && !defined(MARIADB_BASE_VERSION) && defined(HAVE_MYSQL_OPT_SSL_MODE)
			if (in_mysql_get_option(mptr, MYSQL_OPT_SSL_MODE, &use_ssl)) {
				strerr_warn2("userinfo: mysql_get_option: MYSQL_OPT_SSL_MODE: ", (char *) in_mysql_error(mptr), 0);
				flush("userinfo");
				return (1);
			}
			if (use_ssl)
				ptr = (char *) in_mysql_get_ssl_cipher(mptr);
#else
			use_ssl = ((ptr = (char *) in_mysql_get_ssl_cipher(mptr)) ? 1 : 0);
#endif
			subprintfe(subfdout, "userinfo", "Use SSL       : %s\n", use_ssl ? "Yes" : "No");
			if (use_ssl)
				subprintfe(subfdout, "userinfo", "SSL Cipher    : %s\n", ptr);
		}
	}
#ifdef ENABLE_AUTH_LOGGING
	if (Domain && *Domain && DisplayAll)
		subprintfe(subfdout, "userinfo", "Table Name    : %s\n", is_inactive ? inactive_table : default_table);
	if (Domain && *Domain && (DisplayLastAuth || DisplayAll)) {
		auth_time = vget_lastauth(mypw, Domain, AUTH_TIME, ipaddr[0]);
		add_time  = vget_lastauth(mypw, Domain, CREAT_TIME, ipaddr[1]);
		pwdchg_time = vget_lastauth(mypw, Domain, PASS_TIME, ipaddr[2]);
		act_time = vget_lastauth(mypw, Domain, ACTIV_TIME, ipaddr[3]);
		inact_time = vget_lastauth(mypw, Domain, INACT_TIME, ipaddr[4]);
		pop3_time = vget_lastauth(mypw, Domain, POP3_TIME, ipaddr[5]);
		imap_time = vget_lastauth(mypw, Domain, IMAP_TIME, ipaddr[6]);
		delivery_count = 0;
		delivery_time = 0;
		delivery_size = 0;
		if (!stralloc_copys(&tmpbuf, mypw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir/deliveryCount", 22) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if ((fd = open_read(tmpbuf.s)) != -1) {
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			for(;;) {
				if (getln(&ssin, &line, &match, '\n') == -1) {
					strerr_warn3("userinfo: read: ", tmpbuf.s, ": ", &strerr_sys);
					break;
				}
				if (!match && line.len == 0)
					break;
				line.len--;
				line.s[line.len] = 0; /*- remove newline */
				for (ptr = line.s, i = 0; i < line.len; i++, ptr++) {
					if (isspace(*ptr))
						break;
				}
				if (!stralloc_copyb(&timeBuf, line.s, i) || !stralloc_0(&timeBuf))
					die_nomem();
				strptime(timeBuf.s, "%d-%m-%Y:%H:%M:%S", &tm);
				delivery_time = mktime(&tm);
				if (i + 1 < line.len) {
					ptr = line.s + i + 1;
					i += 1;
					j = scan_ulong(ptr, (unsigned long *) &delivery_count);
					i += j;
				}
				if (i + 1 < line.len) {
					ptr = line.s + i + 1;
					i += 1;
					scan_ulong(ptr, (unsigned long *) &delivery_size);
				}
			}
			close(fd);
		}
		if (auth_time > 0 || add_time > 0) {
			ptr = (relay_select(Email, ipaddr[5]) || relay_select(Email, ipaddr[6]) ? "YES" : "NO");
			subprintfe(subfdout, "userinfo", "Relay Allowed : %s\n", ptr);
			subprintfe(subfdout, "userinfo", "Days inact    : %s\n",
					no_of_days((time(0) - (auth_time ? auth_time : add_time))));
		} else {
			subprintfe(subfdout, "userinfo", "Relay Allowed : No\n");
			if (add_time) {
				subprintfe(subfdout, "userinfo", "Days inact    : %s\n", 
						no_of_days((time(0) - (auth_time ? auth_time : add_time))));
			} else
				subprintfe(subfdout, "userinfo", "Days inact    : Unknown\n");
		}
		if (!add_time)
			subprintfe(subfdout, "userinfo", "Added   On    : Unknown\n");
		else
		if (add_time > 0)
			subprintfe(subfdout, "userinfo", "Added On      : (%s) %s",
					ipaddr[1], asctime(localtime(&add_time)));
		else
			subprintfe(subfdout, "userinfo", "Added   On    : ????\n");
		if (!auth_time) {
			subprintfe(subfdout, "userinfo", "last  auth    : Not yet logged in\n");
			subprintfe(subfdout, "userinfo", "last  IMAP    : Not yet logged in\n");
			subprintfe(subfdout, "userinfo", "last  POP3    : Not yet logged in\n");
		} else
		if (pop3_time > 0 || imap_time > 0) {
			subprintfe(subfdout, "userinfo", "last  auth    : (%s) %s",
					ipaddr[0], asctime(localtime(&auth_time)));
			if (pop3_time > 0)
				subprintfe(subfdout, "userinfo", "last  POP3    : (%s) %s",
						ipaddr[5], asctime(localtime(&pop3_time)));
			else
				subprintfe(subfdout, "userinfo", "last  POP3    : Not yet logged in\n");
			if (imap_time > 0)
				subprintfe(subfdout, "userinfo", "last  IMAP    : (%s) %s",
						ipaddr[6], asctime(localtime(&imap_time)));
			else
				subprintfe(subfdout, "userinfo", "last  IMAP    : Not yet logged in\n");
		} else
		if (auth_time < 0)
			subprintfe(subfdout, "userinfo", "last  auth    : ????\n");
		if (!pwdchg_time)
			subprintfe(subfdout, "userinfo", "PassChange    : Not yet Changed\n");
		else
		if (pwdchg_time > 0)
			subprintfe(subfdout, "userinfo", "PassChange    : (%s) %s",
					ipaddr[2], asctime(localtime(&pwdchg_time)));
		else
			subprintfe(subfdout, "userinfo", "PassChange    : ????\n");
		if (!inact_time)
			subprintfe(subfdout, "userinfo", "Inact Date    : Not yet Inactivated\n");
		else
		if (inact_time > 0)
			subprintfe(subfdout, "userinfo", "Inact Date    : (%s) %s",
					ipaddr[4], asctime(localtime(&inact_time)));
		else
			subprintfe(subfdout, "userinfo", "Inact Date    : ????\n");
		if (inact_time > 0) {
			if (!act_time)
				subprintfe(subfdout, "userinfo", "Activ Date    : Not yet Activated\n");
			else
			if (act_time > 0)
				subprintfe(subfdout, "userinfo", "Activ Date    : (%s) %s",
						ipaddr[3], asctime(localtime(&act_time)));
			else
				subprintfe(subfdout, "userinfo", "Activ Date    : ????\n");
		} else {
			if (is_inactive)
				subprintfe(subfdout, "userinfo", "Activ Date    : Not yet Activated\n");
			else {
				if (!add_time)
					subprintfe(subfdout, "userinfo", "Activ Date    : Unknown\n");
				else
				if (add_time > 0)
					subprintfe(subfdout, "userinfo", "Activ Date    : (%s) %s",
							ipaddr[1], asctime(localtime(&add_time)));
				else
					subprintfe(subfdout, "userinfo", "Activ Date    : ????\n");
			}
		}
		if (delivery_time > 0) {
			subprintfe(subfdout, "userinfo", "Delivery Time : %s",
					asctime(localtime(&delivery_time)));
			subprintfe(subfdout, "userinfo", "       latest : (%ld) Mails, %ld Bytes [%ld Kb]\n",
					delivery_count, delivery_size, delivery_size/1024);
		} else
			subprintfe(subfdout, "userinfo", "Delivery Time : No Mails Delivered yet / Per Day Limit not configured\n");
	}
#endif
	if (DisplayFilter || DisplayAll) {
		valiasinfo(mypw->pw_name, Domain);
#ifdef VFILTER
		if (!stralloc_copys(&maildir, mypw->pw_dir) ||
				!stralloc_catb(&maildir, "/Maildir/vfilter", 16) ||
				!stralloc_0(&maildir))
			die_nomem();
		if (!access(maildir.s, F_OK)) {
			subprintfe(subfdout, "userinfo", "Filters       :\n");
			vfilter_display(Email, 0);
		}
#endif
	}
	flush("userinfo");
	return (0);
}
