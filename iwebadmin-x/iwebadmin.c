/*
 * $Id: iwebadmin.c,v 1.12 2019-07-15 21:15:20+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <indimail.h>
#include <indimail_compat.h>
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <alloc.h>
#include <getln.h>
#include <case.h>
#include <byte.h>
#include <open.h>
#include <stralloc.h>
#include <str.h>
#include <env.h>
#include <scan.h>
#include <fmt.h>
#include <strerr.h>
#endif
#define _USE_XOPEN
#define _XOPEN_SOURCE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <pwd.h>
#include <dirent.h>
#include "alias.h"
#include "auth.h"
#include "autorespond.h"
#include "cgi.h"
#include "command.h"
#include "limits.h"
#include "mailinglist.h"
#include "printh.h"
#include "iwebadmin.h"
#include "show.h"
#include "template.h"
#include "user.h"
#include "util.h"
#include "common.h"

stralloc        Username = {0}, Domain = {0}, Password = {0}, Gecos = {0},
				Quota = {0}, Time = {0}, ActionUser = {0}, Newu = {0},
				Password1 = {0}, Password2 = {0}, Crypted = {0}, Alias = {0},
				LineData = {0}, Action = {0}, Message = {0}, SearchUser = {0},
				TmpBuf = {0}, RealDir = {0}, line = {0}, StatusMessage = {0};
int             CGIValues[256];
int             num_of_mailinglist;
time_t          mytime;
char           *TmpCGI = NULL;
int             Compressed;
int             actout = 1, lang_fd = -1, color_table = -1;
char           *html_text[MAX_LANG_STR + 1];

struct vlimits  Limits;
int             AdminType;
int             MaxPopAccounts;
int             MaxAliases;
int             MaxForwards;
int             MaxAutoResponders;
int             MaxMailingLists;

int             CurPopAccounts;
int             CurForwards;
int             CurBlackholes;
int             CurAutoResponders;
int             CurMailingLists;
uid_t           Uid;
gid_t           Gid;
char            Lang[40], Pagenumber[FMT_ULONG];

extern char *crypt(const char *, const char *);

void
iwebadmin_suid(gid_t Gid, uid_t Uid)
{
#ifdef HAVE_SETREGID
	if (setregid(Gid, Gid) != 0)
#else
	if (setgid(Gid) != 0)
#endif
	{
		out("<h2>");
		out(html_text[318]);
		out("</h2>\n");
		flush();
		strerr_warn1("iwebadmin: setgid: ", &strerr_sys);
		iclose();
		exit(EXIT_FAILURE);
	}
#ifdef HAVE_SETREGID
	if (setreuid(Uid, Uid) != 0)
#else
	if (setuid(Uid) != 0)
#endif
	{
		out("<h2>");
		out(html_text[319]);
		out("</h2>\n");
		flush();
		strerr_warn1("iwebadmin: setuid: ", &strerr_sys);
		iclose();
		exit(EXIT_FAILURE);
	}
}

int
auth_user(struct passwd *pw, char *password)
{
	if (!str_diff(pw->pw_passwd, (char *) crypt(password, pw->pw_passwd)))
		return (0);
	/*- 
	 * Authtenticate case where you have CRAM-MD5 but clients do
	 * not have cram-md5 authentication mechanism 
	 */
	if (!access(".trivial_passwords", F_OK) && !str_diff(pw->pw_passwd, password))
		return (0);
	copy_status_mesg(html_text[198]);
	return (1);
}

void
del_id_files(stralloc *dirname)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	int             len;

	if (!(mydir = opendir(dirname->s))) {
		strerr_warn3("iwebadmin: opendir: ", dirname->s, ": ", &strerr_sys);
		return;
	}
	for (len = dirname->len; (mydirent = readdir(mydir));) {
		if (str_str(mydirent->d_name, ".qw") != 0) {
			if (!stralloc_append(dirname, "/") ||
					!stralloc_cats(dirname, mydirent->d_name) ||
					!stralloc_0(dirname))
			{
				iclose();
				exit(EXIT_FAILURE);
			}
			if (unlink(dirname->s)) {
				strerr_warn3("iwebadmin: unlink: ", dirname->s, ": ", &strerr_sys);
				iclose();
				exit(EXIT_FAILURE);
			}
			dirname->len = len;
		}
	}
	closedir(mydir);
	return;
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	const char     *ip_addr = env_get("REMOTE_ADDR");
	const char     *x_forward = env_get("HTTP_X_FORWARDED_FOR");
	static stralloc tmp = {0}, returnhttp = {0}, returntext = {0};
	char           *pi, *rm;
	int             i, fd;
	struct passwd  *pw;
	extern int      encrypt_flag;
	char            strnum[FMT_ULONG], outbuf[2048];
	struct substdio ssout;

	while (!access("/tmp/gdb.wait", F_OK))
			sleep(1);
	init_globals();
	if (x_forward)
		ip_addr = x_forward;
	if (!ip_addr)
		ip_addr = "127.0.0.1";
	if ((pi = env_get("PATH_INFO"))) {
		if (!stralloc_copys(&tmp, pi + 5) || !stralloc_0(&tmp))
			die_nomem();
	}
	if (!(rm = env_get("REQUEST_METHOD")))
		rm = "";
	if (rm && !str_diffn(rm, "POST", 4))
		get_cgi(); /*- sets TmpCGI */
	else {
		if (!(TmpCGI = env_get("QUERY_STRING")))
			TmpCGI = "";
	}
	if (pi && !str_diffn(pi, "/com/", 5)) {
		GetValue(TmpCGI, &Username, "user=");
		GetValue(TmpCGI, &Domain, "dom=");
		GetValue(TmpCGI, &Time, "time=");
		scan_ulong(Time.s, (unsigned long *) &mytime);
		if (!(pw = sql_getpw(Username.s, Domain.s))) {
			copy_status_mesg(html_text[198]);
			show_login();
			iclose();
			exit(0);
		}
		/*- get the real uid and gid and change to that user */
		if (!get_assign(Domain.s, &RealDir, &Uid, &Gid)) {
			out("<h2>coudldn't get domain details for ");
			out(Domain.s);
			out(" from assign file</h2>\n");
			flush();
			copy_status_mesg(html_text[19]);
			show_login();
			iclose();
			exit(0);
		}
		iwebadmin_suid(Gid, Uid);
		if (chdir(RealDir.s) < 0) {
			out("<h2>");
			out(html_text[171]);
			out(" ");
			out(RealDir.s);
			out("</h2>\n");
			flush();
		}
		load_limits();
		set_admin_type();
		if (AdminType == USER_ADMIN || AdminType == DOMAIN_ADMIN)
			auth_user_domain(ip_addr, pw);
		else
			auth_system(ip_addr, pw);
		process_commands(tmp.s);
	} else
	if (pi && str_diffn(pi, "/passwd/", 7) == 0) {
		GetValue(TmpCGI, &Username, "address=");
		GetValue(TmpCGI, &Password, "oldpass=");
		GetValue(TmpCGI, &Password1, "newpass1=");
		GetValue(TmpCGI, &Password2, "newpass2=");
		if (Username.len && !Password.len && (Password1.len || Password2.len)) {
			/*- username entered, but no password */
			copy_status_mesg(html_text[198]);
			errout("iwebadmin: ");
			errout(Username.s);
			errout(": No password\n");
			errflush();
		} else
		if (Username.len && Password.len) {
			/*- attempt to authenticate user */
			if (!get_assign(Domain.s, &RealDir, &Uid, &Gid)) {
				out("<h2>coudldn't get domain details for ");
				out(Domain.s);
				out(" from assign file</h2>\n");
				flush();
				copy_status_mesg(html_text[19]);
				show_login();
				iclose();
				exit(0);
			}
			iwebadmin_suid(Gid, Uid);
			if (!Domain.len) {
				copy_status_mesg(html_text[198]);
				errout("iwebadmin: ");
				errout(Username.s);
				out(": No domain given\n");
				errflush();
			} else {
				if (chdir(RealDir.s) < 0) {
					out("<h2>");
					out(html_text[171]);
					out(" ");
					out(RealDir.s);
					out("</h2>\n");
					flush();
				}
				load_limits();
				if (!access(".trivial_passwords", F_OK))
					encrypt_flag = 1;
				if (!(pw = sql_getpw(Username.s, Domain.s))) {
					copy_status_mesg(html_text[198]);
					errout("iwebadmin: ");
					errout(Username.s);
					out(": No such user\n");
					errflush();
				} else
				if (pw->pw_gid & NO_PASSWD_CHNG) {
					copy_status_mesg(html_text[20]);
					errout("iwebadmin: ");
					errout(Username.s);
					out(": password change denied\n");
					errflush();
				} else
				if (auth_user(pw, Password.s)) {
					copy_status_mesg(html_text[198]);
					errout("iwebadmin: ");
					errout(Username.s);
					out(": incorrect password\n");
					errflush();
				} else
				if (str_diffn(Password1.s, Password2.s, Password1.len > Password2.len ? Password1.len : Password2.len) != 0)
					copy_status_mesg(html_text[200]);
				else
				if (!Password1.len)
					copy_status_mesg(html_text[234]);
				else
				if ((i = ipasswd(Username.s, Domain.s, Password1.s, USE_POP)) != 1)
					copy_status_mesg(html_text[140]);
#ifndef TRIVIAL_PASSWORD_ENABLED
				else
				if (str_str(Username.s, Password1.s) != NULL)
					copy_status_mesg(html_text[320]);
#endif
				else {
					/* success */
					copy_status_mesg(html_text[139]);
					Password.len = 0;
					send_template("change_password_success.html");
					errout("iwebadmin: ");
					errout(Username.s);
					out(": password changed\n");
					errflush();
					return 0;
				}
			}
		}
		send_template("change_password.html");
		return 0;
	} else
	if (rm && *rm) {
		GetValue(TmpCGI, &Username, "username=");
		GetValue(TmpCGI, &Domain, "domain=");
		GetValue(TmpCGI, &Password, "password=");
		if (Username.len) {
			i = str_chr(Username.s, '@');
			if (Username.s[i]) {
				if (!stralloc_copyb(&Domain, Username.s + i + 1, Username.len - (i + 1)) ||
						!stralloc_0(&Domain))
					die_nomem();
				Domain.len--;
			}
		}
		get_assign(Domain.s, &RealDir, &Uid, &Gid);
		iwebadmin_suid(Gid, Uid);
		/*- Authenticate a user and domain admin */
		if (Domain.len && Username.len) {
			if (chdir(RealDir.s)) {
				copy_status_mesg(html_text[171]);
				errout("iwebadmin: ");
				errout(RealDir.s);
				errout(": unable to change dir\n");
				errflush();
				iclose();
				exit(0);
			}
			load_limits();
			if (!(pw = sql_getpw(Username.s, Domain.s))) {
				copy_status_mesg(html_text[198]);
				errout("iwebadmin: ");
				errout(Username.s);
				errout(": No such user\n");
				errflush();
				show_login();
				iclose();
				exit(0);
			}
			if (auth_user(pw, Password.s)) {
				copy_status_mesg(html_text[198]);
				errout("iwebadmin: ");
				errout(Username.s);
				out(": incorrect password\n");
				errflush();
				show_login();
				iclose();
				exit(0);
			}
			if (!stralloc_copys(&tmp, pw->pw_dir) ||
					!stralloc_catb(&tmp, "/Maildir", 8) ||
					!stralloc_0(&tmp))
				die_nomem();
			tmp.len--;
			del_id_files(&tmp);
			mytime = time(NULL);
			if (!stralloc_append(&tmp, "/") ||
					!stralloc_catb(&tmp, strnum, fmt_ulong(strnum, (unsigned long) mytime)) ||
					!stralloc_catb(&tmp, ".qw", 3) ||
					!stralloc_0(&tmp))
				die_nomem();
			if ((fd = open_trunc(tmp.s)) == -1) {
				out(html_text[144]);
				out(" ");
				out(tmp.s);
				out("<br>\n");
				flush();
				iclose();
				exit(0);
			}
			substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
			/*- set session vars */
			GetValue(TmpCGI, &returntext, "returntext=");
			GetValue(TmpCGI, &returnhttp, "returnhttp=");
			if (substdio_put(&ssout, "ip_addr=", 8) ||
					substdio_puts(&ssout, (char *) ip_addr) ||
					substdio_put(&ssout, "&returntext=", 12) ||
					substdio_put(&ssout, returntext.s, returntext.len) ||
					substdio_put(&ssout, "&returnhttp=", 12) ||
					substdio_put(&ssout, returnhttp.s, returnhttp.len) ||
					substdio_put(&ssout, "\n", 1) || substdio_flush(&ssout)) {
				out(html_text[144]);
				out(" ");
				out(tmp.s);
				out("<br>\n");
				flush();
				iclose();
				exit(0);
			} else
			if (close(fd)) {
				out(html_text[144]);
				out(" ");
				out(tmp.s);
				out("<br>\n");
				flush();
				iclose();
				exit(0);
			}
			set_admin_type();
			/*-
			 * show the main menu for domain admins, modify user page
			 * for regular users 
			 */
			if (AdminType == DOMAIN_ADMIN)
				show_menu();
			else {
				if (!stralloc_copy(&ActionUser, &Username) || !stralloc_0(&ActionUser))
					die_nomem();
				ActionUser.len--;
				moduser();
			}
			iclose();
			exit(0);
		}
	}
	show_login();
	iclose();
	return 0;
}

/*- populate html_text */
void
load_lang(char *lang)
{
	unsigned long   id;
	int             match, len;
	char           *ptr, *s;
	char            inbuf[1024];
	static stralloc lang_str = {0};
	struct substdio ssin;

	lang_str.len = 0;
	if ((lang_fd = open_lang(lang)) == -1) {
		/*-
		 * Rare error likely caused by improper installation, should probably be 
		 * handled by regular error system, but this is a quick band-aid.
		 */
		out("Content-Type: text/html\r\n\r\n");
		out("<html> <head>\r\n");
		out("<title>Failed to open lang file:");
		out(lang);
		out("</title>\r\n");
		out("</head>\r\n<body>\r\n");
		out("<h1>iwebadmin error</h1>\r\n");
		out("<p>Failed to open lang file: ");
		out(lang);
		out(". Please check your lang directory.\r\n");
		out("</body></html>\r\n");
		flush();
		exit(-1);
	}
	substdio_fdbuf(&ssin, read, lang_fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("load_lang: read: ", lang, ": ", &strerr_sys);
			close(lang_fd);
			return;
		}
		if (line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!stralloc_catb(&lang_str, ptr, line.len - (ptr - line.s)) ||
				!stralloc_0(&lang_str))
			die_nomem();
	}
	close(lang_fd);
	for (len = 0, ptr = lang_str.s;len < lang_str.len;) {
		for (s = ptr + scan_ulong(ptr, &id); *s && isspace(*s); s++);
		if (!isspace(*ptr))
			html_text[id] = s;
		len += str_len(ptr) + 1;
		if (len < lang_str.len)
			ptr = lang_str.s + len;
	}
	return;
}

void
init_globals()
{
	char           *accept_lang, *langptr, *qptr, *charset;
	int             lang_err, i;
	float           maxq, thisq;

	byte_zero((char *) CGIValues, sizeof (CGIValues));
	CGIValues['0'] = 0;
	CGIValues['1'] = 1;
	CGIValues['2'] = 2;
	CGIValues['3'] = 3;
	CGIValues['4'] = 4;
	CGIValues['5'] = 5;
	CGIValues['6'] = 6;
	CGIValues['7'] = 7;
	CGIValues['8'] = 8;
	CGIValues['9'] = 9;
	CGIValues['A'] = 10;
	CGIValues['B'] = 11;
	CGIValues['C'] = 12;
	CGIValues['D'] = 13;
	CGIValues['E'] = 14;
	CGIValues['F'] = 15;
	CGIValues['a'] = 10;
	CGIValues['b'] = 11;
	CGIValues['c'] = 12;
	CGIValues['d'] = 13;
	CGIValues['e'] = 14;
	CGIValues['f'] = 15;
	actout = 1;
	AdminType = NO_ADMIN;
	lang_fd = -1;

	/*
	 * Parse HTTP_ACCEPT_LANGUAGE to find highest preferred language
	 * that we have a translation for.  Example setting:
	 * de-de, ja;q=0.25, en;q=0.50, de;q=0.75
	 * The q= lines determine which is most preferred, defaults to 1.
	 * Our routine starts with en at 0.0, and then would try de-de (1.00),
	 * de (1.00), ja (0.25), en (0.50), and then de (0.75).
	 *
	 * default to English at 0.00 preference 
	 */
	maxq = 0.0;
	str_copy(Lang, "en");
	/* read in preferred languages */
	if ((langptr = env_get("HTTP_ACCEPT_LANGUAGE"))) {
		accept_lang = alloc(str_len(langptr) + 1);
		str_copy(accept_lang, langptr);
		langptr = str_tok(accept_lang, " ,\n");
		while (langptr) {
			qptr = str_str(langptr, ";q=");
			if (qptr) {
				*qptr = '\0';	/* convert semicolon to NULL */
				thisq = (float) atof(qptr + 3);
			} else
				thisq = 1.0;
			/*- if this is a better match than our previous best, try it */
			if (thisq > maxq) {
				lang_err = open_lang(langptr);

				/*
				 * Remove this next section for strict interpretation of
				 * HTTP_ACCEPT_LANGUAGE.  It will try language xx (with the
				 * same q value) if xx-yy fails.
				 */
				if ((lang_err == -1) && (langptr[2] == '-')) {
					langptr[2] = '\0';
					lang_err = open_lang(langptr);
				}
				if (lang_err == 0) {
					maxq = thisq;
					str_copy(Lang, langptr);
				}
			}
			langptr = str_tok(NULL, " ,\n");
		}
		alloc_free(accept_lang);
	}
	/*- best language choice is now in 'Lang' */

	/*- build table of html_text entries */
	for (i = 0; i <= MAX_LANG_STR; i++)
		html_text[i] = "";

	/*- read English first as defaults for incomplete language files */
	if (str_diffn(Lang, "en", 3))
		load_lang("en");
	/*- load the preferred language */
	load_lang(Lang);

	/*- open the color table */
	open_colortable();
	umask(INDIMAIL_UMASK);
	charset = html_text[0];
	out("Content-Type: text/html; charset=");
	out(!*charset ? "iso-8859-1" : charset);
	out("\n");
#ifdef NO_CACHE
	out("Cache-Control: no-cache\n");
	out("Cache-Control: no-store\n");
	out("Pragma: no-cache\n");
	out("Expires: Thu, 01 Dec 1994 16:00:00 GMT\n");
#endif
	out("\n");
	if (!stralloc_ready(&SearchUser, 1) || !stralloc_0(&SearchUser))
		die_nomem();
	SearchUser.len--;
}

	/*
	 * This feature sponsored by PinkRoccade Public Sector, Sept 2004 
	 */
void
quickAction(char *username, int action)
{
	char           *space, *ar, *ez;
	char           *aliasline;
	int             len, plen;

	/*
	 * Note that all of the functions called from quickAction() assume
	 * that the username to modify is in a global called "ActionUser"
	 * It would be better to pass this information as a parameter, but
	 * that's how it was originally done.  The code in command.c that
	 * calls quickAction() passes ActionUser as the username parameter
	 * in hopes that someday we'll remove the globals and pass parameters.
	 */

	/*
	 * first check for alias/forward, autorepsonder (or even mailing list) 
	 */
	if ((aliasline = valias_select(username, Domain.s))) {
		/*
		 * Autoresponder/Mailing List detection algorithm:
		 * We're looking for either '/autorespond ' or '/ezmlm-reject ' to
		 * appear in the first line, before a space appears
		 */
		space = str_str(aliasline, " ");
		ar = str_str(aliasline, "/autoresponder ");
		ez = str_str(aliasline, "/ezmlm-reject ");
		if (ar && space && (ar < space)) {
			/* autorepsonder */
			if (action == ACTION_MODIFY)
				modautorespond();
			else
			if (action == ACTION_DELETE)
				delautorespond();
		} else
		if (ez && space && (ez < space)) {
			/* mailing list (cdb-backend only) */
			if (action == ACTION_MODIFY)
				modmailinglist();
			else if (action == ACTION_DELETE)
				delmailinglist();
		} else {
			/*- it's just a forward/alias of some sort */
			if (action == ACTION_MODIFY)
				moddotqmail();
			else if (action == ACTION_DELETE)
				deldotqmail();
		}
	} else
	if (sql_getpw(username, Domain.s)) {
		/*- POP/IMAP account */
		if (action == ACTION_MODIFY)
			moduser();
		else
		if (action == ACTION_DELETE) {
			// don't allow deletion of postmaster account
			if (!case_diffs(username, "postmaster") || !case_diffs(username, "prefilt") ||
					!case_diffs(username, "postfilt")) {
				copy_status_mesg(html_text[317]);
				show_menu();
				iclose();
			} else
				ideluser();
		}
	} else {
		/*
		 * check for mailing list on SQL backend (not in valias_select) 
		 */
		if (!stralloc_copyb(&TmpBuf, ".qmail-", 7) ||
				!stralloc_cats(&TmpBuf, username) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		str_replace(TmpBuf.s + 7, '.', ':');
		if (!access(TmpBuf.s, F_OK)) {
			/*- mailing list (MySQL backend) */
			if (action == ACTION_MODIFY)
				modmailinglist();
			else
			if (action == ACTION_DELETE)
				delmailinglist();
		} else { /*- user does not exist */
			len = str_len(html_text[153] + str_len(username) + Domain.len + 50);
			plen = 0;
			for (;;) {
				set_status_mesg_size(len);
				plen = snprinth(StatusMessage.s, len, "%s (%H@%H)", html_text[153], username, Domain.s);
				if (plen < len) {
					StatusMessage.len = plen;
					break;
				}
				len = plen + 28;
			}
			show_menu();
			iclose();
		}
	}
}
