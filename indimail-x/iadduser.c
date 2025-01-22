/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <stralloc.h>
#include <strerr.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <error.h>
#include <scan.h>
#include <fmt.h>
#include <mkpasswd.h>
#include <getEnvConfig.h>
#include <env.h>
#endif
#include "indimail.h"
#include "addusercntrl.h"
#include "is_distributed_domain.h"
#include "open_master.h"
#include "is_user_present.h"
#include "add_user_assign.h"
#include "get_assign.h"
#include "get_local_hostid.h"
#include "get_indimailuidgid.h"
#include "make_user_dir.h"
#include "sql_adduser.h"
#include "sql_gethostid.h"
#include "sql_updateflag.h"
#include "sql_getpw.h"
#include "variables.h"
#include "get_hashmethod.h"

#define ALLOWCHARS              " .!#$%&'*+-/=?^_`{|}~\""

#ifndef	lint
static char     sccsid[] = "$Id: iadduser.c,v 1.10 2024-05-17 16:24:31+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("iadduser: out of memory", 0);
	_exit(111);
}

/*
 * E-mail addresses are formally defined in RFC 5322 (mostly section 3.4.1) and to a lesser degree RFC 5321. An e-mail address
 * is a string of a subset of ASCII characters separated into 2 parts by an "@" (at sign), a "local-part" and a domain, that is,
 * local-part@domain.
 *
 * The "local-part" of an e-mail address can be up to 64 characters and the domain name a maximum of 255 characters. Clients may
 * attempt to use larger objects, but they must be prepared for the server to reject them if they cannot be handled by it.
 *
 * The local-part of the e-mail address may use any of these ASCII characters:
 *
 *   * Uppercase and lowercase English letters (a-z, A-Z)
 *   * Digits 0 through 9
 *   * Characters ! # $ % & ' * + - / = ? ^ _ ` { | } ~
 *   * Character . (dot, period, full stop) provided that it is not the first or last character, and provided also that it does
 *     not appear two or more times consecutively.
 *
 * Additionally, quoted-strings (ie: "John Doe"@example.com) are permitted, thus allowing characters that would otherwise be
 * prohibited, however they do not appear in common practice. RFC 5321 also warns that "a host that expects to receive mail SHOULD
 * avoid defining mailboxes where the Local-part requires (or uses) the Quoted-string form".
 *
 * The local-part is case sensitive, so "jsmith@example.com" and "JSmith@example.com" may be delivered to different people. This
 * practice is discouraged by RFC 5321. However, only the authoritative mail servers for a domain may make that decision. The only
 * exception is for a local-part value of "postmaster" which is case insensitive, and should be forwarded to the server's administrator.
 *
 * Notwithstanding the addresses permitted by these standards, some systems impose more restrictions on e-mail addresses, both in
 * e-mail addresses created on the system and in e-mail addresses to which messages can be sent. Hotmail, for example, only allows
 * creation of e-mail addresses using alphanumerics, dot (.), underscore (_) and hyphen (-), and will not allow sending mail to any
 * e-mail address containing ! # $ % * / ? | ^ { } ` ~[2]. The domain name is much more restricted: it must match the requirements
 * for a hostname, consisting of letters, digits, hyphens and dots. In addition, the domain may be an IP address literal, surrounded
 * by square braces, such as jsmith@[192.168.2.1] (this is rarely seen, except in spam).
 *
 * The informational RFC 3696 written by the author of RFC 5321 explains the details in a readable way, with a few minor errors noted
 * in the 3696 errata.
 */
int
iadduser(const char *username, const char *domain, const char *mdahost, const char *password,
		 const char *gecos, char *quota, int max_users_per_level, int actFlag,
		 int encrypt_flag, const char *scram_passwd)
{
	static stralloc Dir = {0}, Crypted = {0}, tmpbuf = {0}, line = {0};
	char            estr[2], inbuf[512], strnum[FMT_ULONG];
	char           *tmpstr, *dir, *ptr, *allow_chars;
	uid_t           uid;
	gid_t           gid;
	int             uid_flag = 0, fd, match, ulen, u_level = 0, i;
	struct substdio ssin;
#ifdef CLUSTERED_SITE
	static stralloc SqlBuf = {0};
	int             err, is_dist = 0;
#endif

	if (!username || !*username || !isalnum((int) *username))
		strerr_die1x(100, "iadduser: illegal username");
	if ((ulen = str_len(username)) > MAX_PW_NAME || str_len(domain) > MAX_PW_DOMAIN ||
			str_len(gecos) > MAX_PW_GECOS || str_len(password) > MAX_PW_PASS)
		strerr_die1x(100, "iadduser: Name too long");
	if (*username == '.' || username[ulen - 1] == '.')
		strerr_die1x(100, "iadduser: Trailing/Leading periods not allowed");
	getEnvConfigStr(&allow_chars, "ALLOWCHARS", ALLOWCHARS);
	for (ptr = (char *) username; *ptr; ptr++) {
		if (*ptr == ':')
			strerr_die1x(100, "iadduser: ':' not allowed in names");
		if (*ptr == '.' && *(ptr + 1) == '.')
			strerr_die1x(100, "iadduser: successive periods not allowed in local-part See RFC-5322");
		i = str_chr(allow_chars, *ptr);
		if (allow_chars[i])
			continue;
		if (!isalnum((int) *ptr)) {
			estr[0] = *ptr;
			estr[1] = 0;
			strerr_die3x(100, "iadduser: [", estr, "] not allowed in local-part See RFC-5322");
		}
		if (isupper((int) *ptr))
			strerr_die3x(100, "iadduser: user [", username, "] has an uppercase character");
	}
	if (domain && *domain) {
		for (ptr = (char *) domain; *ptr; ptr++) {
			if (*ptr == ':') {
				strerr_die1x(100, "iadduser: ':' not allowed in names");
				return (-1);
			} else
			if (isupper((int) *ptr))
				strerr_die3x(100, "iadduser: domain [", domain, "] has an uppercase character");
		}
		uid_flag = 1;
#ifdef CLUSTERED_SITE
		if ((is_dist = is_distributed_domain(domain)) == -1)
			strerr_die3x(111, "iadduser: unable to verify ", domain, " as a distributed domain");
		if (is_dist == 1) { /*- distributed domain */
			if (open_master())
				strerr_die1x(111, "iadduser: Failed to open Master Db");
			uid_flag = ADD_FLAG;
			if ((err = is_user_present(username, domain)) == 1)
				strerr_die5x(100, "iadduser: username ", username, "@", domain, " exists");
			else
			if (err == -1) {
				strerr_die1x(111, "iadduser: auth db Error");
				return (-1);
			}
		} else
		if (sql_getpw(username, domain))
			strerr_die5x(100, "iadduser: username ", username, "@", domain, " exists");
#else
		if (sql_getpw(username, domain))
			strerr_die5x(100, "iadduser: username ", username, "@", domain, " exists");
#endif
		if (!(tmpstr = get_assign(domain, &Dir, &uid, &gid)))
			strerr_die3x(100, "iadduser: Domain ", domain, " does not exist");
	} else { /*- if domain is null */
		if (get_assign(username, &Dir, &uid, &gid))
			strerr_die3x(100, "iadduser: username ", username, "  exists");
		get_indimailuidgid(&uid, &gid);
	}
	if (!stralloc_copy(&tmpbuf, &Dir) ||
		!stralloc_catb(&tmpbuf, "/.users_per_level", 17) || !stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1 && errno != error_noent)
		strerr_die3sys(111, "iadduser: ", tmpbuf.s, ": ");
	if (fd == -1)
		u_level = 0;
	else {
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die3sys(111, "iadduser: read: ", tmpbuf.s, ": ");
		close(fd);
		if (!line.len)
			strerr_die3x(100, "iadduser: ", tmpbuf.s, ": incomplete line");
		if (match) {
			line.len--;
			if (!line.len)
				strerr_die3x(111, "iadduser: ", tmpbuf.s, ": incomplete line");
			line.s[line.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		scan_int(line.s, &u_level);
	}
	/*- check gecos for : characters - bad */
	if (gecos && *gecos) {
		i = str_chr(gecos, ':');
		if (gecos[i])
			strerr_die1x(100, "iadduser: ':' not allowed in names");
	}
	if (!(dir = make_user_dir(username, domain, uid, gid,
			!max_users_per_level ? u_level : max_users_per_level))) {
		strerr_die1x(111, "iadduser: make user dir failed");
	}
	if (!*dir)
		dir = (char *) 0;
	if (domain && *domain) {
		if (!(ptr = env_get("PASSWORD_HASH"))) {
			if ((i = get_hashmethod(domain)) == -1)
				strerr_die1sys(111, "iadduser: get_hashmethod: ");
			strnum[fmt_int(strnum, i)] = 0;
			if (!env_put2("PASSWORD_HASH", strnum))
				strerr_die1x(111, "iadduser: out of memory");
		}
		if (mkpasswd(password, &Crypted, encrypt_flag) == -1)
			strerr_die1sys(111, "iadduser: crypt: ");
		ptr = sql_adduser(username, domain, Crypted.s, gecos, dir, quota,
				uid_flag, actFlag, scram_passwd);
		if (!ptr || !*ptr)
			return (-1);
#ifdef CLUSTERED_SITE
		if (is_dist == 1 && uid_flag == ADD_FLAG) {
			/*- Get hostid of the Local machine */
			if (mdahost && *mdahost) {
				if (!(ptr = sql_gethostid(mdahost)))
					strerr_die2x(111, "iadduser-sql_gethostid: Unable to get hostid for ", mdahost);
			} else /* avoid looping of mails */
			if (!(ptr = get_local_hostid()))
				strerr_die1x(111, "iadduser-sql_gethostid: unable to get local hostid");
			/*-
			 * This can happen under a race condition where some other process
			 * adds the same user to hostcntrl. In this case the entries added
			 * locally should be removed.
			 */
			if ((err = addusercntrl(username, domain, ptr, Crypted.s, 0)) == 2) {
				if (!stralloc_catb(&SqlBuf, "delete low_priority from ", 25) ||
					!stralloc_cats(&SqlBuf, default_table) ||
					!stralloc_catb(&SqlBuf, "where pw_name = \"", 17) ||
					!stralloc_cats(&SqlBuf, username) ||
					!stralloc_catb(&SqlBuf, "\" and pw_domain = \"", 19) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
					die_nomem();
				if (mysql_query(&mysql[1], SqlBuf.s))
					strerr_die2x(111, "iadduser-addusercntrl: ", (char *) in_mysql_error(&mysql[1]));
#ifdef ENABLE_AUTH_LOGGING
				if (!stralloc_catb(&SqlBuf, "delete low_priority from lastauth where user=\"", 46) ||
					!stralloc_cats(&SqlBuf, username) ||
					!stralloc_catb(&SqlBuf, "\" and domain = \"", 16) ||
					!stralloc_cats(&SqlBuf, domain) ||
					!stralloc_append(&SqlBuf, "\"") || !stralloc_0(&SqlBuf))
					die_nomem();
				if (mysql_query(&mysql[1], SqlBuf.s))
					strerr_die2x(111, "iadduser-lastauth: ", (char *) in_mysql_error(&mysql[1]));
#endif
				strerr_die5x(111, "iadduser: username ", username, "@", domain, " exists");
			} else
			if (!err)
				sql_updateflag(username, domain, 1); /*- Reset the pw_uid flag to 1 */
			/* - don't bother for if (err) as hostsync should take care */
		} /*- if (is_dist == 1 && uid_flag == ADD_FLAG) */
#endif
	} else
	if (add_user_assign(username, dir))
		return (-1);
	return (0);
}
/*
 * $Log: iadduser.c,v $
 * Revision 1.10  2024-05-17 16:24:31+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2023-07-17 11:44:10+05:30  Cprogrammer
 * set hash method from hash_method control file in controldir, domaindir
 *
 * Revision 1.8  2023-07-16 13:56:59+05:30  Cprogrammer
 * check mkpasswd for error
 *
 * Revision 1.7  2023-03-20 10:03:03+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.6  2022-11-02 12:43:44+05:30  Cprogrammer
 * added feature to add scram password during user addition
 *
 * Revision 1.5  2022-10-27 17:06:18+05:30  Cprogrammer
 * add to hostcntrl only if domain is distrbuted
 *
 * Revision 1.4  2022-08-05 22:40:35+05:30  Cprogrammer
 * removed apop argument to iadduser()
 *
 * Revision 1.3  2022-08-05 21:02:18+05:30  Cprogrammer
 * added encrypt_flag argument
 *
 * Revision 1.2  2020-04-01 18:55:09+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-14 18:31:35+05:30  Cprogrammer
 * Initial revision
 *
 */
