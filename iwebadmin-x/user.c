/*
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
 *
 * $Id: user.c,v 1.36 2023-10-10 18:06:45+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define REMOVE_MYSQL_H
#include <indimail_compat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <alloc.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <substdio.h>
#include <getln.h>
#include <strerr.h>
#include <error.h>
#include <case.h>
#include <get_scram_secrets.h>
#endif
#ifdef HAVE_GSASL_MKPASSWD
#include "gsasl_mkpasswd.h"
#endif
#include "alias.h"
#include "cgi.h"
#include "config.h"
#include "limits.h"
#include "printh.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "show.h"
#include "template.h"
#include "user.h"
#include "util.h"
#include "common.h"

#define HOOKS 1

#ifdef HOOKS
/*
 * note that as of December 2003, only the first three hooks are 
 * implemented 
 */
#define HOOK_ADDUSER     "adduser"
#define HOOK_DELUSER     "deluser"
#define HOOK_MODUSER     "moduser"
#endif

extern int rename(const char *oldpath, const char *newpath);

extern int create_flag;

void
show_users()
{
	if (MaxPopAccounts == 0)
		return;
	send_template("show_users.html");
}


int
show_user_lines(char *user, char *dom, time_t mytime, char *dir)
{
	int             fd, i, k, startnumber, moreusers = 1, totalpages,
					bounced, colspan = 7, allowdelete, bars, match;
	struct passwd  *pw;
	char            qconvert[FMT_DOUBLE], strnum[FMT_ULONG];
	static stralloc line = {0}, dest = {0}, path = {0};
	char            inbuf[1024];
	struct substdio ssin;

	if (MaxPopAccounts == 0)
		return 0;

	/*- Get the default catchall box name */
	if ((fd = open_read(".qmail-default")) == -1) {
		/*- report error opening .qmail-default and exit */
		out("<tr><td colspan=\"");
		strnum[fmt_int(strnum, colspan)] = 0;
		out(strnum);
		out("\">");
		out(html_text[144]);
		out(" .qmail-default</tr></td>");
		flush();
		iclose();
		iweb_exit(SYSTEM_FAILURE);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn1("show_user_lines: read: .qmail-default: ", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default 1<BR>\n");
		flush();
		iclose();
		iweb_exit(READ_FAILURE);
	}
	if (match) {
		line.len--;
		line.s[line.len] = 0;
	} else {
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
	}
	close(fd);

	if (SearchUser.len) {
		pw = sql_getall(dom, 1, 1);
		for (k = 0; pw; k++) {
			if ((!SearchUser.s[1] && pw->pw_name[0] >= SearchUser.s[0]) || !str_diff(SearchUser.s, pw->pw_name))
				break;
			pw = sql_getall(dom, 0, 0);
		}
		if (k == 0)
			str_copy(Pagenumber, "1");
		else
			Pagenumber[fmt_int(Pagenumber, (k / MAXUSERSPERPAGE) + 1)] = 0;
	}

	if (!stralloc_copyb(&TmpBuf, "where pw_domain=\"", 17) ||
			!stralloc_cats(&TmpBuf, dom) ||
			!stralloc_append(&TmpBuf, "\"") ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	k = count_table("indimail", TmpBuf.s) + count_table("indibak", TmpBuf.s);
	/*- Determine number of pages */
	if (k == 0)
		totalpages = 1;
	else
		totalpages = (k / MAXUSERSPERPAGE) + 1;
	scan_int(*Pagenumber ? Pagenumber : "1", &i);
	startnumber = MAXUSERSPERPAGE * (i - 1);
	if (i == 0)
		str_copy(Pagenumber, "1");
	if (str_str(line.s, " bounce-no-mailbox\n"))
		bounced = 1;
	else
	if (str_str(line.s, "@")) {
		bounced = 0;
		i = str_rchr(line.s, ' ');
		if (line.s[i] == ' ') {
			if (!stralloc_copyb(&dest, line.s + i + 1, line.len - (i + 1)) ||
					!stralloc_0(&dest))
				die_nomem();
			dest.len--;
		}
	} else {
		/*- Maildir type catchall */
		bounced = 0;
		if (!stralloc_copy(&dest, &line) ||
				!stralloc_0(&dest))
			die_nomem();
		dest.len--;
	}

	/*
	 * check to see if there are any users to list, 
	 * otherwise repeat previous page
	 */
	pw = sql_getall(dom, 1, 1);
	if (AdminType == DOMAIN_ADMIN || (AdminType == USER_ADMIN && !str_diffn(pw->pw_name, Username.s, Username.len + 1))) {
		for (k = 0; k < startnumber; ++k) {
			pw = sql_getall(dom, 0, 0);
		}
	}
	if (pw == NULL) {
		out("<tr><td colspan=\"");
		strnum[fmt_int(strnum, colspan)] = 0;
		out(strnum);
		out("\" bgcolor=");
		out(get_color_text("000"));
		out(">");
		out(html_text[131]);
		out("</td></tr>\n");
		moreusers = 0;
		flush();
	} else {
		while (pw &&
				((k < MAXUSERSPERPAGE + startnumber) ||
				 (AdminType != DOMAIN_ADMIN || (AdminType == USER_ADMIN && !str_diffn(pw->pw_name, Username.s, Username.len + 1)))))
		{
			if (AdminType == DOMAIN_ADMIN || (AdminType == USER_ADMIN && !str_diffn(pw->pw_name, Username.s, Username.len + 1))) {
				mdir_t          diskquota = 0;
				mdir_t          maxmsg = 0;

				/*- display account name and user name */
				out("<tr bgcolor=");
				out(get_color_text("000"));
				out(">");
				printh("<td align=\"left\">%H</td>", pw->pw_name);
				printh("<td align=\"left\">%H</td>", pw->pw_gecos);

				/*- display user's quota */
				if (!stralloc_copys(&path, pw->pw_dir) ||
						!stralloc_catb(&path, "/Maildir", 8) ||
						!stralloc_0(&path))
					die_nomem();
				diskquota = check_quota(path.s, &maxmsg);
				out("<td align=\"right\">");
				strnum[fmt_double(strnum, ((double) diskquota) / 1048576.0, 2)] = 0; /* Convert to MB */
				out(strnum);
				out("&nbsp;/&nbsp;</td>");
				if (str_diffn(pw->pw_shell, "NOQUOTA", 7)) {
					if (quota_to_megabytes(qconvert, pw->pw_shell)) {
						out("<td align=\"left\">(BAD)</td>");
					} else {
						out("<td align=\"left\">");
						out(qconvert);
						out("</td>");
					}
				} else {
					out("<td align=\"left\">");
					out(html_text[229]);
					out("</td>");
				}

				/*- display button to modify user */
				out("<td align=\"center\">");
				printh("<a href=\"%s&moduser=%C\">", cgiurl("moduser"), pw->pw_name);
				out("<img src=\"");
				out(IMAGEURL);
				out("/modify.png\" border=\"0\"></a>");
				out("</td>");

				/*
				 * if the user has admin privileges and pw->pw_name is not 
				 * the user or postmaster, allow deleting 
				 */
				if (AdminType == DOMAIN_ADMIN && str_diffn(pw->pw_name, Username.s, Username.len) &&
						str_diff(pw->pw_name, "postmaster") &&
						str_diff(pw->pw_name, "prefilt") &&
						str_diff(pw->pw_name, "postfilt")) {
					allowdelete = 1;
				} else /*- else, don't allow deleting */
					allowdelete = 0;

				/*
				 * display trashcan for delete, or nothing if delete not allowed 
				 */
				out("<td align=\"center\">");
				if (allowdelete) {
					printh("<a href=\"%s&deluser=%C\">", cgiurl("deluser"), pw->pw_name);
					out("<img src=\"");
					out(IMAGEURL);
					out("/trash.png\" border=\"0\"></a>");
				} else {
					/*- printf ("<img src=\"%s/disabled.png\" border=\"0\">", IMAGEURL); */
				}
				out("</td>");

				/*- display button in the 'set catchall' column */
				out("<td align=\"center\">");
				if (bounced == 0 && !str_diffn(pw->pw_name, dest.s, dest.len)) {
					out("<img src=\"");
					out(IMAGEURL);
					out("/radio-on.png\" border=\"0\"></a>");
#ifdef CATCHALL_ENABLED
				} else
				if (AdminType == DOMAIN_ADMIN) {
					printh("<a href=\"%s&deluser=%C&page=%s\">", cgiurl("setdefault"), pw->pw_name, Pagenumber);
					out("<img src=\"");
					out(IMAGEURL);
					out("/radio-off.png\" border=\"0\"></a>");
#endif
				} else {
					out("<img src=\"");
					out(IMAGEURL);
					out("/disabled.png\" border=\"0\">");
				}
				out("</td>");
				out("</tr>\n");
				flush();
			}
			pw = sql_getall(dom, 0, 0);
			++k;
		} /*- while */
	}

	if (AdminType == DOMAIN_ADMIN) {
		print_user_index("showusers", colspan, user, dom, mytime);
		out("<tr bgcolor=");
		out(get_color_text("000"));
		out(">");
		out("<td colspan=\"");
		strnum[fmt_int(strnum, colspan)] = 0;
		out(strnum);
		out("\" align=\"right\">");
		out("<font size=\"2\"><b>");
		out("[&nbsp;");
		bars = 0;
#ifdef USER_INDEX
		/*- only display "previous page" if pagenumber > 1 */
		scan_int(Pagenumber, &i);
		if (i > 1) {
			printh("<a href=\"%s&page=%d\">%s</a>", cgiurl("showusers"),
				   i - 1 ? i - 1 : i, html_text[135]);
			bars = 1;
		}
		if (moreusers && i < totalpages) {
			if (bars)
				out("&nbsp;|&nbsp;");
			printh("<a href=\"%s&page=%d\">%s</a>", cgiurl("showusers"), i + 1, html_text[137]);
			bars = 1;
		}
#endif
#ifdef CATCHALL_ENABLED
		if (bars)
			out("&nbsp;|&nbsp;");
		printh("<a href=\"%s\">%s</a>", cgiurl("deleteall"), html_text[235]);
		out("&nbsp;|&nbsp;");
		printh("<a href=\"%s\">%s</a>", cgiurl("bounceall"), html_text[134]);
		out("&nbsp;|&nbsp;");
		printh("<a href=\"%s\">%s</a>", cgiurl("setremotecatchall"), html_text[206]);
#endif
		out("&nbsp;]");
		out("</b></font>");
		out("</td></tr>\n");
	}
	flush();
	return 0;
}

void
adduser()
{
	char            strnum[FMT_ULONG];

	count_users();
	load_limits();
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	if (MaxPopAccounts != -1 && CurPopAccounts >= MaxPopAccounts) {
		if (!stralloc_copys(&StatusMessage, html_text[199]) ||
				!stralloc_append(&StatusMessage, " ") ||
				!stralloc_catb(&StatusMessage, strnum, fmt_int(strnum, MaxPopAccounts)) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		StatusMessage.len--;
		show_menu();
		iclose();
		iweb_exit(LIMIT_FAILURE);
	}
	send_template("add_user.html");
}

void
moduser()
{
	if (!(AdminType == DOMAIN_ADMIN ||
				(AdminType == USER_ADMIN &&
				 !str_diffn(ActionUser.s, Username.s, ActionUser.len > Username.len ? ActionUser.len : Username.len)))) {
		copy_status_mesg(html_text[142]);
		show_menu();
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	send_template("mod_user.html");
}

void
addusernow()
{
	int             cnt = 0, num, pid, error, len, plen, encrypt_flag = 1;
	char          **mailingListNames;
	char           *ptr;
	static stralloc email = {0}, tmp1 = {0}, tmp2 = {0};
#ifdef MODIFY_QUOTA
	char            qconvert[11];
#endif
	char            strnum[FMT_ULONG];
	struct passwd  *mypw;
	static stralloc box = {0};

	count_users();
	load_limits();
	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		show_menu();
		iclose();
		iweb_exit(PERM_FAILURE);
	}

	if (MaxPopAccounts != -1 && CurPopAccounts >= MaxPopAccounts) {
		copy_status_mesg(html_text[199]);
		if (!stralloc_append(&StatusMessage, " ") ||
				!stralloc_catb(&StatusMessage, strnum, fmt_int(strnum, MaxPopAccounts)) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		StatusMessage.len--;
		show_menu();
		iclose();
		iweb_exit(LIMIT_FAILURE);
	}
	GetValue(TmpCGI, &Newu, "newu=");
	if (fixup_local_name(Newu.s)) {
		copy_status_mesg(html_text[148]);
		if (!stralloc_cat(&StatusMessage, &Newu) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		StatusMessage.len--;
		adduser();
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
	if (check_local_user(Newu.s)) {
		len = str_len(html_text[175]) + Newu.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[175], Newu.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		adduser();
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
#ifdef MODIFY_QUOTA
	GetValue(TmpCGI, &Quota, "quota=");
#endif
	GetValue(TmpCGI, &Password1, "password1=");
	GetValue(TmpCGI, &Password2, "password2=");
	if (str_diffn(Password1.s, Password2.s, Password1.len > Password2.len ? Password1.len : Password2.len)) {
		copy_status_mesg(html_text[200]);
		adduser();
		iclose();
		iweb_exit(AUTH_FAILURE);
	}
#ifndef TRIVIAL_PASSWORD_ENABLED
	if (str_str(Newu.s, Password1.s)) {
		copy_status_mesg(html_text[320]);
		adduser();
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
#endif

#ifndef ENABLE_LEARN_PASSWORDS
	if (!Password1.len) {
		copy_status_mesg(html_text[234]);
		adduser();
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
#endif
	if (!stralloc_copy(&email, &Newu) ||
			!stralloc_append(&email, "@") ||
			!stralloc_cat(&email, &Domain) ||
			!stralloc_0(&email))
		die_nomem();
	email.len--;
	GetValue(TmpCGI, &Gecos, "gecos=");
	if (!Gecos.len) {
		if (!stralloc_copy(&Gecos, &Newu) ||
				!stralloc_0(&Gecos))
			die_nomem();
		Gecos.len--;
	}

	GetValue(TmpCGI, &TmpBuf, "number_of_mailinglist=");
	scan_int(TmpBuf.s, &num);
	if (num > 0) {
		if (!(mailingListNames = (char **) alloc(sizeof (char *) * num)))
			die_nomem();
		else {
			for (cnt = 0; cnt < num; cnt++) {
				if (!stralloc_copys(&tmp1, "subscribe") ||
						!stralloc_catb(&tmp1, strnum, fmt_int(strnum, cnt)) ||
						!stralloc_append(&tmp1, "=") ||
						!stralloc_0(&tmp1))
					die_nomem();
				if ((error = GetValue(TmpCGI, &tmp2, tmp1.s)) != -1) {
					if (!(pid = fork())) {
						if (!(mailingListNames[cnt] = (char *) alloc(tmp2.len + 1)))
							die_nomem();
						str_copyb(mailingListNames[cnt], tmp2.s, tmp2.len + 1);
						if (!stralloc_copys(&tmp1, EZMLMDIR) ||
								!stralloc_catb(&tmp1, "/ezmlm-sub", 10) ||
								!stralloc_0(&tmp1))
							die_nomem();
						if (!stralloc_copy(&tmp2, &RealDir) ||
								!stralloc_append(&tmp2, "/") ||
								!stralloc_cats(&tmp2, mailingListNames[cnt]) ||
								!stralloc_0(&tmp2))
							die_nomem();
						execl(tmp1.s, "ezmlm-sub", tmp2.s, email.s, NULL);
						_exit(127);
					} else {
						wait(&pid);
					}
				}
			}
		}
	}
	/*- add the user then get the indimail password structure */
	create_flag = 1;
	/*-----------------------------------------------*/
	if (!stralloc_copy(&tmp1, &RealDir) ||
			!stralloc_catb(&tmp1, "/.trivial_passwords", 19) ||
			!stralloc_0(&tmp1))
		die_nomem();
	if (!access(tmp1.s, F_OK))
		encrypt_flag = 0;
	GetValue(TmpCGI, &box, "cram=");
	cram = !str_diff(box.s, "on") ? 1 : 0;
#ifdef HAVE_GSASL_MKPASSWD
	GetValue(TmpCGI, &box, "scram=");
	u_scram = !str_diff(box.s, "on") ? 1 : 0;
	if (u_scram) {
		switch (scram)
		{
		case 1: /*- SCRAM-SHA-1 */
			gsasl_mkpasswd(0, "SCRAM-SHA-1", iter_count, b64salt.len ? b64salt.s : 0, cram, Password1.s, &result);
			break;
		case 2: /*- SCRAM-SHA-256 */
			gsasl_mkpasswd(0, "SCRAM-SHA-256", iter_count, b64salt.len ? b64salt.s : 0, cram, Password1.s, &result);
			break;
		}
		ptr = scram > 0 ? result.s : 0;
	} else
	if (cram) {
		if (!stralloc_copyb(&result, "{CRAM}", 6) ||
				!stralloc_cats(&result, Password1.s) ||
				!stralloc_0(&result))
			die_nomem();
		ptr = result.s;
	} else
		ptr = 0;
#else
	if (cram) {
		if (!stralloc_copyb(&result, "{CRAM}", 6) ||
				!stralloc_cats(&result, Password1.s) ||
				!stralloc_0(&result))
			die_nomem();
		ptr = result.s;
	} else
		ptr = 0;
#endif
	/*-----------------------------------------------*/
	if (iadduser(Newu.s, Domain.s, 0, Password1.s, Gecos.s, 0, 0, USE_POP, encrypt_flag, ptr) == 0 &&
#ifdef MYSQL_REPLICATION
		!sleep(2) &&
#endif
		(mypw = sql_getpw(Newu.s, Domain.s))) {
		/*
		 * iadduser() in indimail sets the default
		 * quota, so we only need to change it if the user enters
		 * something in the Quota field.
		 */
#ifdef MODIFY_QUOTA
		if (!str_diffn(Quota.s, "NOQUOTA", 8))
			setuserquota(Newu.s, Domain.s, "NOQUOTA");
		else
		if (Quota.len) {
			if (quota_to_bytes(qconvert, Quota.s))
				copy_status_mesg(html_text[314]);
			else
				setuserquota(Newu.s, Domain.s, qconvert);
		}
#endif
		/*- report success */
		len = str_len(html_text[2]) + str_len(html_text[119]) + Newu.len + Domain.len + Gecos.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H@%H (%H) %s", html_text[2], Newu.s, Domain.s, Gecos.s, html_text[119]);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else {
		len = str_len(html_text[2]) + str_len(html_text[120]) + Newu.len + Domain.len + Gecos.len + 28;
		/*- otherwise, report error */
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "<font color=\"red\">%s %H@%H (%H) %s</font>",
				html_text[2], Newu.s, Domain.s, Gecos.s, html_text[120]);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	}
	call_hooks(HOOK_ADDUSER, Newu.s, Domain.s, Password1.s, Gecos.s);
	/*
	 * After we add the user, show the user page
	 * people like to visually verify the results
	 */
	show_users();
}

int
call_hooks(char *hook_type, char *p1, char *p2, char *p3, char *p4)
{
	int             fd, pid, match;
	char           *cmd = 0, *ptr;
	static stralloc hooks_path = {0}, line = {0};
	char            inbuf[1024];
	struct substdio ssin;

	/*- first look in directory for domain */
	if (!stralloc_copy(&hooks_path, &RealDir) ||
			!stralloc_catb(&hooks_path, "/.iwebadmin-hooks", 17) ||
			!stralloc_0(&hooks_path))
		die_nomem();
	if (access(hooks_path.s, F_OK)) {
		/*- then try /etc/indimail */
		if (!stralloc_copys(&hooks_path, SYSCONFDIR) ||
				!stralloc_catb(&hooks_path, "/iwebadmin-hooks", 16) ||
				!stralloc_0(&hooks_path))
			die_nomem();
	}
	if ((fd = open_read(hooks_path.s)) == -1)
		return (0);
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("show_user_lines: read: ", hooks_path.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" .iwebadmin-hooks 1<BR>\n");
			flush();
			return (0);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		match = str_chr(ptr, ' ');
		if (ptr[match] && !str_diffn(ptr, hook_type, match)) {
			ptr += match;
			for (;*ptr && isspace(*ptr); ptr++);
			cmd = ptr;
			if (!*cmd)
				cmd = 0;
			break;
		}
	}
	close(fd);
	if (!cmd)
		return (0); /* don't have a hook for this type */
	pid = fork();
	if (pid == 0) {
		/*
		 * Second param to execl should actually be just the program name,
		 * without the path information.  Add a pointer to point into cmd
		 * at the start of the program name only.    BUG 2003-12 
		 */
		execl(cmd, cmd, p1, p2, p3, p4, NULL);
		out("cmd=[");
		out(cmd);
		out("] ");
		out(html_text[202]);
		out(" ");
		out("type = ");
		out(hook_type);
		out(" ");
		out(p1);
		out(" ");
		out(p2);
		out(" ");
		out(p3);
		out(" ");
		out(p4);
		out(": ");
		out(error_str(errno));
		out("\n");
		flush();
		exit(127);
	} else
		wait(&pid);
	return (0);
}

void
ideluser()
{
	send_template("del_user_confirm.html");
}

void
unsubscribe_user(char *user, char *domain)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	struct stat     st;
	static stralloc email = {0}, tmp1 = {0}, tmp2 = {0};
	pid_t           pid;
	extern int      ezmlm_make;

	if (!ezmlm_make || MaxMailingLists == 0)
		return;
	if (!(mydir = opendir(".")))
		return;
	while ((mydirent = readdir(mydir))) {
		if (stat(mydirent->d_name, &st) == 1)
			continue;
		if (!str_diff(mydirent->d_name, ".") || !str_diff(mydirent->d_name, ".."))
			continue;
		if ((st.st_mode & S_IFMT) != S_IFDIR)
			continue;
		if (!stralloc_copyb(&tmp1, ".qmail-", 7) ||
				!stralloc_copys(&tmp1, mydirent->d_name) ||
				!stralloc_0(&tmp1))
			die_nomem();
		if (access(tmp1.s, F_OK))
			continue;
		if (!stralloc_copys(&tmp1, mydirent->d_name) ||
				!stralloc_catb(&tmp1, "/subscribers", 12) ||
				!stralloc_0(&tmp1))
			die_nomem();
		if (access(tmp1.s, F_OK))
			continue;
		pid = fork();
		if (pid == 0) {
			if (!stralloc_copys(&tmp1, EZMLMDIR) ||
					!stralloc_catb(&tmp1, "/ezmlm-unsub", 12) ||
					!stralloc_0(&tmp1))
				die_nomem();
			if (!stralloc_copy(&tmp2, &RealDir) ||
					!stralloc_append(&tmp2, "/") ||
					!stralloc_cats(&tmp2, mydirent->d_name) ||
					!stralloc_0(&tmp2))
				die_nomem();
			if (!stralloc_copys(&email, user) ||
					!stralloc_append(&email, "@") ||
					!stralloc_cats(&email, domain) ||
					!stralloc_0(&email))
				die_nomem();
			execl(tmp1.s, "ezmlm-unsub", tmp2.s, email.s, (char *) 0);
		} else
			wait(&pid);
	}
	return;
}

void
delusergo()
{
	static stralloc forward = {0}, forwardto = {0};
	int             len, plen;

	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		show_users();
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	if (deluser(ActionUser.s, Domain.s, 1) ) {
		copy_status_mesg(html_text[145]);
		show_users();
		iclose();
		iweb_exit(SYSTEM_FAILURE);
	}
	len = ActionUser.len + str_len(html_text[141]) + 28;
	for (plen = 0;;) {
		if (!stralloc_ready(&StatusMessage, len))
			die_nomem();
		plen = snprinth(StatusMessage.s, len, "%H %s", ActionUser.s, html_text[141]);
		if (plen < len) {
			StatusMessage.len = plen;
			break;
		}
		len = plen + 28;
	}
	/*-
	 * Start create forward when delete - 
	 * Code added by Eugene Teo 6 June 2000 
	 * Modified by Jeff Hedlund (jeff.hedlund@matrixsi.com) 4 June 2003 
	 */

	GetValue(TmpCGI, &forward, "forward=");
	if (!str_diffn(forward.s, "on", 3)) {
		GetValue(TmpCGI, &forwardto, "forwardto=");
		if (adddotqmail_shared(ActionUser.s, &forwardto, -1) != 0) {
			if (!stralloc_copys(&StatusMessage, html_text[315]))
				die_nomem();
			StatusMessage.len -= 5; /*- remove '%s'. */
			if (!stralloc_cat(&StatusMessage, &forwardto) ||
					!stralloc_0(&StatusMessage))
				die_nomem();
		}
	}
	unsubscribe_user(ActionUser.s, Domain.s);
	call_hooks(HOOK_DELUSER, ActionUser.s, Domain.s, forwardto.s, "");
	show_users();
}

void
count_users()
{
	if (!stralloc_copyb(&TmpBuf, "where pw_domain=\"", 17) ||
			!stralloc_cat(&TmpBuf, &Domain) ||
			!stralloc_append(&TmpBuf, "\"") ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	CurPopAccounts = count_table("indimail", TmpBuf.s) + count_table("indibak", TmpBuf.s);
}

void
setremotecatchall()
{
	send_template("setremotecatchall.html");
}

void
set_qmaildefault(char *opt)
{
	int             fd, match, use_vfilter = 0;
	static stralloc line = {0};
	char            inbuf[1024], outbuf[512];
	char           *ptr;
	struct substdio ssin, ssout;

	if ((fd = open_read(".qmail-default")) == -1) {
		out(html_text[144]);
		out(" .qmail-default<br>\n");
		flush();
		iclose();
		iweb_exit(SYSTEM_FAILURE);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn1("set_qmaildefault: read: .qmail-default", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default 1<BR>\n");
		flush();
		close(fd);
		iclose();
		iweb_exit(READ_FAILURE);
	}
	if (match) {
		line.len--;
		line.s[line.len] = 0;
	} else {
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
	}
	match = str_chr(line.s, '#');
	if (line.s[match])
		line.s[match] = 0;
	for (ptr = line.s; *ptr && isspace(*ptr); ptr++);
	if (!*ptr) {
		out(html_text[159]);
		out(" .qmail-default 1<BR>\n");
		flush();
		close(fd);
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
	close(fd);
	use_vfilter = str_str(ptr, "vfilter") ? 1 : 0;

	if ((fd = open_trunc(".qmail-default")) == -1) {
		strerr_warn1("set_qmaildefault: open_trunc: .qmail-default: ", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default<br>\n");
		flush();
		iclose();
		iweb_exit(SYSTEM_FAILURE);
	} else {
		substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
		if (substdio_put(&ssout, "| ", 2) ||
				substdio_puts(&ssout, PREFIX) ||
				substdio_put(&ssout, "/sbin/", 6) ||
				substdio_put(&ssout, use_vfilter ? "vfilter" : "vdelivermail", use_vfilter ? 7 : 12) ||
				substdio_put(&ssout, " '' ", 4) ||
				substdio_puts(&ssout, opt) ||
				substdio_put(&ssout, "\n", 1) ||
				substdio_flush(&ssout))
		{
			strerr_warn1("set_qmaildefault: write: ", &strerr_sys);
			out(html_text[144]);
			out(" .qmail-default<br>\n");
			flush();
			iclose();
			iweb_exit(SYSTEM_FAILURE);
		}
		close(fd);
	}
	show_users();
	iclose();
	iweb_exit(0);
}

void
setremotecatchallnow()
{
	static stralloc fwdaddr = {0};
	int             len, plen;

	GetValue(TmpCGI, &Newu, "newu=");
	if (check_email_addr(Newu.s)) {
		len = str_len(html_text[148]) + Newu.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[148], Newu.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		setremotecatchall();
		iweb_exit(INPUT_FAILURE);
	}
	if (Newu.s[0] == '@') {
		/*- forward all mail to external domain */
		if (!stralloc_copyb(&fwdaddr, "$EXT", 4) ||
				!stralloc_cat(&fwdaddr, &Newu) ||
				!stralloc_0(&fwdaddr))
			die_nomem();
		set_qmaildefault(fwdaddr.s);
	} else
		set_qmaildefault(Newu.s);
}

void
bounceall()
{
	set_qmaildefault("bounce-no-mailbox");
}

void
deleteall()
{
	set_qmaildefault("delete");
}

void
get_catchall()
{
	int             fd, i, match;
	static stralloc line = {0};
	char            inbuf[1024];
	char           *ptr;
	struct substdio ssin;

	/*- Get the default catchall box name */
	if ((fd = open_read(".qmail-default")) == -1) {
		out("<tr><td colspan=\"5\">");
		out(html_text[144]);
		out(" .qmail-default</td><tr>\n");
		flush();
		iclose();
		iweb_exit(SYSTEM_FAILURE);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn1("get_catchall: read: .qmail-default", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default 1<BR>\n");
		close(fd);
		flush();
		iclose();
		iweb_exit(READ_FAILURE);
	}
	if (match) {
		line.len--;
		line.s[line.len] = 0;
	} else {
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
	}
	match = str_chr(line.s, '#');
	if (line.s[match])
		line.s[match] = 0;
	for (ptr = line.s; *ptr && isspace(*ptr); ptr++);
	if (!*ptr) {
		out(html_text[159]);
		out(" .qmail-default 1<BR>\n");
		close(fd);
		flush();
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
	close(fd);
	if (str_str(line.s, " bounce-no-mailbox\n")) {
		out("<b>");
		out(html_text[130]);
		out("</b>");
		flush();
	} else
	if (str_str(line.s, " delete\n")) {
		out("<b>");
		out(html_text[236]);
		out("</b>");
		flush();
	} else
	if (str_str(ptr, "@")) {
		i = str_rchr(ptr, ' ');
		if (ptr[i] && !str_diffn(ptr + 1, "$EXT@", 5))
			printh("<b>%s <I>user</I>%H</b>", html_text[62], ptr + i + 5);
		else
			printh("<b>%s %H</b>", html_text[62], ptr + i + 1);
	} else {
		i = str_rchr(ptr, '/');
		if (ptr[i]) {
			i++;
			if (line.s[line.len - 1] == '/') {
				line.s[line.len - 1] = 0;
				line.len--;
			}
			printh("<b>%s %H</b>", html_text[62], ptr + i);
		} else
			printh("<b>%s invalid</b>", html_text[62]);
	}
	return;
}

int
migrate_vacation(char *dir, char *user)
{
	int             len;

	if (!stralloc_copys(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cats(&TmpBuf, user) ||
			!stralloc_catb(&TmpBuf, "/.vacation.msg", 14) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	len = TmpBuf.len;
	if (!stralloc_cats(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cats(&TmpBuf, user) ||
			!stralloc_catb(&TmpBuf, "/.autoresp.msg", 14) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (!access(TmpBuf.s, F_OK) && rename(TmpBuf.s, TmpBuf.s + len)) {
		copy_status_mesg(html_text[322]);
		strerr_warn5("migratevacation: rename: ", TmpBuf.s, " --> ", TmpBuf.s + len, ": ",  &strerr_sys);
		return 1;
	}
	return 0;
}

int
makeautoresp(substdio *out, char *dir)
{
	static stralloc subject = {0};
	int             fd;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], outbuf[512];
	struct substdio ssout;

	GetValue(TmpCGI, &subject, "vsubject=");
	/*- if no subject, error */
	if (!subject.len || !subject.s[0]) {
		copy_status_mesg(html_text[216]);
		return 1;
	}
	if (migrate_vacation(dir, ActionUser.s))
		return 1;
	/*- make the autoresp directory */
	if (!stralloc_copys(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (r_mkdir(TmpBuf.s, 0750, Uid, Gid)) {
		copy_status_mesg(html_text[143]);
		if (!stralloc_catb(&StatusMessage, " /autoresp/user", 14) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		strnum1[fmt_uint(strnum1, getuid())] = 0;
		strnum2[fmt_uint(strnum2, getgid())] = 0;
		strerr_warn7("makeautoresp: ", TmpBuf.s, ": uid =", strnum1, ", gid=", strnum2, ": ", &strerr_sys);
		return (1);
	}
	if (!stralloc_copys(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/content-type", 13) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (substdio_put(out, "|", 1) ||
			substdio_puts(out, PREFIX) ||
			substdio_put(out, "/bin/autoresponder -q ", 22))
	{
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .qmail", 7) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		strerr_warn3("makeautoresp: write: ", TmpBuf.s, ": ", &strerr_sys);
		return (1);
	}
	if (!access(TmpBuf.s, R_OK)) {
		if (substdio_put(out, "-T ", 3) ||
				substdio_puts(out, dir) ||
				substdio_put(out, "/content-type ", 14))
		{
			copy_status_mesg(html_text[144]);
			if (!stralloc_catb(&StatusMessage, " .qmail", 7) ||
					!stralloc_0(&StatusMessage))
				die_nomem();
			strerr_warn3("makeautoresp: write: ", TmpBuf.s, ": ", &strerr_sys);
			return (1);
		}
	}
	if (substdio_puts(out, dir) ||
			substdio_put(out, "/autoresp/", 10) ||
			substdio_put(out, ActionUser.s, ActionUser.len) ||
			substdio_put(out, "/.autoresp.msg ", 15) ||
			substdio_puts(out, dir) ||
			substdio_put(out, "/autoresp/", 10) ||
			substdio_put(out, ActionUser.s, ActionUser.len) ||
			substdio_put(out, "\n", 1) ||
			substdio_flush(out))
	{
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .qmail", 7) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		strerr_warn3("makeautoresp: write: ", TmpBuf.s, ": ", &strerr_sys);
		return (1);
	}

	GetValue(TmpCGI, &Message, "vmessage=");
	/*- set up the message file */
	if (!stralloc_copys(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/.autoresp.msg", 14) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if ((fd = open_trunc(TmpBuf.s)) == -1) {
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .autoresp.msg", 14) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		return 1;
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_put(&ssout, "Reference: ", 11) ||
			substdio_put(&ssout, subject.s, subject.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "Subject: Autoresponse - ", 24) ||
			substdio_put(&ssout, " Re: ", 5) ||
			substdio_put(&ssout, subject.s, subject.len) ||
			substdio_put(&ssout, "\n\n", 2) ||
			substdio_put(&ssout, Message.s, Message.len) ||
			substdio_flush(&ssout))
	{
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .autoresp.msg", 14) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		strerr_warn3("makeautoresp: write: ", TmpBuf.s, ": ", &strerr_sys);
		return (1);
	}
	close(fd);
	return 0;
}

void
modusergo()
{
	char           *tmpstr, *ptr, *cptr;
	long            q;
	int             fd, ret_code, count, autoresp = 0, saveacopy = 0, rmdotqmail = 1, err, i;
	struct passwd  *vpw = 0;
	static stralloc box = {0}, cforward = {0}, dotqmailfn = {0}, triv_pass = {0};
	char           *olddotqmail = 0;
#ifdef MODIFY_QUOTA
	char           *quotaptr;
	char            qconvert[11];
#endif
	struct stat     sb;
	int             encrypt_flag = 1;
	const char     *flagfields[] = { "zeroflag=", "oneflag=", "twoflag=", "threeflag=" };
	const gid_t     gidflags[] = { V_USER0, V_USER1, V_USER2, V_USER3 };
	gid_t           orig_gid;
	char            outbuf[512], strnum[FMT_ULONG];
	struct substdio ssout;

	if (!(AdminType == DOMAIN_ADMIN ||
			(AdminType == USER_ADMIN && !str_diffn(ActionUser.s, Username.s, ActionUser.len > Username.len ? ActionUser.len : Username.len)))) {
		copy_status_mesg(html_text[142]);
		moduser();
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	/*
	 * Password1, Password2, gecos and b64salt
	 * is set in the function process_commands
	 */
	if (Password1.len && Password2.len) {
		if (str_diff(Password1.s, Password2.s)) {
			copy_status_mesg(html_text[200]);
			moduser();
			iclose();
			iweb_exit(AUTH_FAILURE);
		}
#ifndef TRIVIAL_PASSWORD_ENABLED
		if (str_str(ActionUser.s, Password1.s)) {
			copy_status_mesg(html_text[320]);
			moduser();
			iclose();
			iweb_exit(INPUT_FAILURE);
		}
#endif
		if (!stralloc_copy(&triv_pass, &RealDir) ||
				!stralloc_catb(&triv_pass, "/.trivial_passwords", 19) ||
				!stralloc_0(&triv_pass))
			die_nomem();
		if (!access(triv_pass.s, F_OK))
			encrypt_flag = 0;
		GetValue(TmpCGI, &box, "cram=");
		cram = !str_diff(box.s, "on") ? 1 : 0;
		GetValue(TmpCGI, &box, "scram=");
		u_scram = !str_diff(box.s, "on") ? 1 : 0;
#ifdef HAVE_GSASL_MKPASSWD
		if (u_scram) {
			switch (scram)
			{
			case 1: /*- SCRAM-SHA-1 */
				gsasl_mkpasswd(0, "SCRAM-SHA-1", iter_count, b64salt.len ? b64salt.s : 0, cram, Password1.s, &result);
				break;
			case 2: /*- SCRAM-SHA-256 */
				gsasl_mkpasswd(0, "SCRAM-SHA-256", iter_count, b64salt.len ? b64salt.s : 0, cram, Password1.s, &result);
				break;
			}
		} else
		if (cram) {
			if (!stralloc_copyb(&result, "{CRAM}", 6) ||
					!stralloc_cats(&result, Password1.s) ||
					!stralloc_0(&result))
				die_nomem();
			ptr = result.s;
		}
		ret_code = ipasswd(ActionUser.s, Domain.s, Password1.s, encrypt_flag, u_scram || cram ? result.s : 0);
#else
		if (cram) {
			if (!stralloc_copyb(&result, "{CRAM}", 6) ||
					!stralloc_cats(&result, Password1.s) ||
					!stralloc_0(&result))
				die_nomem();
			ptr = result.s;
		}
		ret_code = ipasswd(ActionUser.s, Domain.s, Password1.s, encrypt_flag, cram ? result.s : 0);
#endif
		if (ret_code != 1) {
			copy_status_mesg(html_text[140]);
			if (!stralloc_catb(&StatusMessage, " (error code ", 13) ||
					!stralloc_catb(&StatusMessage, strnum, fmt_int(strnum, ret_code)) ||
					!stralloc_append(&StatusMessage, ")") ||
					!stralloc_0(&StatusMessage))
				die_nomem();
		} else
			copy_status_mesg(html_text[139]);
	}
	vpw = sql_getpw(ActionUser.s, Domain.s);
	if (!str_diffn(vpw->pw_passwd, "{SCRAM-SHA-1}", 13) || !str_diffn(vpw->pw_passwd, "{SCRAM-SHA-256}", 15)) {
		i = get_scram_secrets(vpw->pw_passwd, 0, 0, 0, 0, 0, 0, 0, &ptr);
		if (i != 6 && i != 8) {
			strerr_die1x(1, "iwebadmin: unable to get secrets");
			copy_status_mesg(html_text[026]);
			moduser();
			iweb_exit(AUTH_FAILURE);
		}
		vpw->pw_passwd = ptr;
	} else
	if (!str_diffn(vpw->pw_passwd, "{CRAM}", 6)) {
		vpw->pw_passwd += 6;
		i = str_rchr(vpw->pw_passwd, ',');
		if (vpw->pw_passwd[i])
			vpw->pw_passwd += (i + 1);
	}
#ifdef MODIFY_QUOTA
	/*
	 * strings used: 307 = "Invalid Quota", 308 = "Quota set to unlimited",
	 * 309 = "Quota set to %s bytes"
	 */
	if (AdminType == DOMAIN_ADMIN) {
		GetValue(TmpCGI, &Quota, "quota=");
		scan_long(Quota.s, &q);
		if (!Quota.len || !str_diff(vpw->pw_shell, Quota.s)) {
		/*- Blank or no change, do nothing */
		} else
		if (!str_diffn(Quota.s, "NOQUOTA", 8)) {
			if (setuserquota(ActionUser.s, Domain.s, Quota.s) == -1)
				copy_status_mesg(html_text[307]);
			else
				copy_status_mesg(html_text[308]);
		} else
		if (q) {
			quotaptr = Quota.s;
			if (quota_to_bytes(qconvert, quotaptr))
				copy_status_mesg(html_text[307]);
			else
			if (!str_diff(qconvert, vpw->pw_shell)) {
				/*- unchanged, do nothing */
			} else
			if ((err = setuserquota(ActionUser.s, Domain.s, qconvert)) == -1)
				copy_status_mesg(html_text[307]);
			else {
				if (!stralloc_copyb(&StatusMessage, "Quota set to ", 13) ||
						!stralloc_cats(&StatusMessage, qconvert) ||
						!stralloc_catb(&StatusMessage, " bytes", 6) ||
						!stralloc_0(&StatusMessage))
					die_nomem(); /*- html_text[309] */
			}
		} else
			copy_status_mesg(html_text[307]);
	}
#endif
	GetValue(TmpCGI, &Gecos, "gecos=");
	/*- check for the V_USERx flags and set accordingly */
	/*- new code by Tom Collins <tom@tomlogic.com>, Dec 2004 */
	/*- replaces code by James Raftery <james@now.ie>, 12 Dec. 2002 */
	orig_gid = vpw->pw_gid;
	for (i = 0; i < 4; i++) {
		GetValue(TmpCGI, &box, (char *) flagfields[i]);
		if (!str_diff(box.s, "on"))
			vpw->pw_gid |= gidflags[i];
		else
		if (!str_diff(box.s, "off"))
			vpw->pw_gid &= ~gidflags[i];
	}
	/*
	 * we're trying to cut down on unnecessary updates to the password entry
	 * we accomplish this by only updating if the pw_gid or gecos changed
	 */
	if (Gecos.len && str_diff(Gecos.s, vpw->pw_gecos)) {
		vpw->pw_gecos = Gecos.s;
		sql_setpw(vpw, Domain.s, 0);
	} else 
	if (vpw->pw_gid != orig_gid)
		i = sql_setpw(vpw, Domain.s, 0);
	/*- get the value of the autoresp checkbox */
	GetValue(TmpCGI, &box, "autoresp=");
	autoresp = !str_diff(box.s, "on") ? 1 : 0;
	/*- if they want to save a copy */
	GetValue(TmpCGI, &box, "fsaved=");
	saveacopy = !str_diff(box.s, "on") ? 1 : 0;
	/*- get the value of the cforward radio button */
	GetValue(TmpCGI, &cforward, "cforward=");
	/*- open old .qmail file if it exists and load it into memory */
	if (!stralloc_copys(&dotqmailfn, vpw->pw_dir) ||
			!stralloc_catb(&dotqmailfn, "/.qmail", 7) ||
			!stralloc_0(&dotqmailfn))
		die_nomem();
	if (!str_diff(cforward.s, "disable") && !access(dotqmailfn.s, F_OK))
		unlink(dotqmailfn.s);

	if (!str_diff(cforward.s, "disable") && !autoresp) {
		call_hooks(HOOK_MODUSER, ActionUser.s, Domain.s, Password1.s, Gecos.s);
		copy_status_mesg(html_text[119]);
		moduser();
		return;
	}
	err = stat(dotqmailfn.s, &sb);
	if (err == -1 && errno != error_noent) {
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .qmail", 7) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		moduser();
		return;
	}
	if (!err && sb.st_size && !(olddotqmail = (char *) alloc(sb.st_size)))
		die_nomem();
	if (olddotqmail) { /* copy existing .qmail to memory */
		if ((fd = open_read(dotqmailfn.s)) != -1) {
			if (read(fd, olddotqmail, sb.st_size) == -1) {
				strerr_warn2(dotqmailfn.s, ": read: ", &strerr_sys);
				copy_status_mesg(html_text[144]);
				if (!stralloc_catb(&StatusMessage, " .qmail", 7) ||
						!stralloc_0(&StatusMessage))
					die_nomem();
				moduser();
				return;
			}
			close(fd);
		}
		/*- do not delete .qmail on error */
		rmdotqmail = 0;
	}
	dotqmailfn.len -= 5; /*- change dotqmailfns.s from .qmail to .qxxxx */
	if (!stralloc_catb(&dotqmailfn, "xxxx", 4))
		die_nomem();
	if ((fd = open_trunc(dotqmailfn.s)) == -1) {
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .qxxxx", 7) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		moduser();
		return;
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	/*
	 * Scan through old .qmail and write out any unrecognized program delivery
	 * lines to the new .qmail file.
	 */
	if (olddotqmail) {
		for (ptr = cptr = olddotqmail; *cptr; cptr++) {
			if (*cptr == '\n') {
				*cptr = 0;
				if (*ptr == '|' && !str_str(ptr, "/true delete") &&
						!str_str(ptr, "/autoresponder ")) {
					substdio_puts(&ssout, ptr);
					substdio_put(&ssout, "\n", 1);
					ptr = cptr + 1;
				}
			}
		}
		alloc_free(olddotqmail);
	}
	/*
	 * Decide on what to write to the new .qmail file after any old program
	 * delivery lines are written.
	 */
	err = 0;
	/*
	 * note that we consider a .qmail file with just Maildir delivery to be empty
	 * since it can be removed.
	 */
	/*- if they want to forward */
	if (!str_diff(cforward.s, "forward")) {
		/*- get the value of the foward */
		GetValue(TmpCGI, &box, "nforward=");
		/*- If nothing was entered, error */
		if (!box.len) {
			close(fd);
			copy_status_mesg(html_text[215]);
			unlink(dotqmailfn.s);
			moduser();
			return;
		} else {
			tmpstr = str_tok(box.s, " ,;\n");
			/*- tmpstr points to first non-token */
			count = 0;
			while (tmpstr != NULL && count < MAX_FORWARD_PER_USER) {
				if ((*tmpstr != '|') && (*tmpstr != '/')) {
					i = str_chr(tmpstr, '@');
					if (!tmpstr[i]) {
						substdio_put(&ssout, "&", 1);
						substdio_puts(&ssout, tmpstr);
						substdio_put(&ssout, "@", 1);
						substdio_put(&ssout, Domain.s, Domain.len);
						substdio_put(&ssout, "\n", 1);
					} else {
						substdio_put(&ssout, "&", 1);
						substdio_puts(&ssout, tmpstr);
						substdio_put(&ssout, "\n", 1);
					}
					rmdotqmail = 0;
					++count;
				}
				tmpstr = str_tok(NULL, " ,;\n");
			}
		}
	}
	if (str_diff(cforward.s, "forward") || saveacopy) {
		if (!str_diff(cforward.s, "blackhole")) {
			substdio_put(&ssout, "# delete\n", 9);
			rmdotqmail = 0;
		} else {
			/*- this isn't enough to consider the .qmail file non-empty */
			substdio_puts(&ssout, vpw->pw_dir);
			substdio_put(&ssout, "/Maildir/\n", 10);
		}
	}
	if (substdio_flush(&ssout) == -1) {
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .qmail", 7) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		close(fd);
		unlink(dotqmailfn.s);
		moduser();
		return;
	}
	if (autoresp) {
		if (!makeautoresp(&ssout, RealDir.s))
			rmdotqmail = 0;
		else {
			close(fd);
			unlink(dotqmailfn.s);
			moduser();
			return;
		}
	} else {
		/*- delete old autoresp directory */
		if (!stralloc_copy(&TmpBuf, &RealDir) ||
				!stralloc_catb(&TmpBuf, "/autoresp", 9) ||
				!stralloc_cat(&TmpBuf, &ActionUser) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		vdelfiles(TmpBuf.s, ActionUser.s, Domain.s);
	}
	close(fd);
	if (err) {
		unlink(dotqmailfn.s);
		moduser();
		return;
	}
	if (rmdotqmail)
		unlink(dotqmailfn.s);
	else {
		dotqmailfn.len -= 4; /*- change dotqmailfns.s from .qmail to .qxxxx */
		if (!stralloc_copy(&TmpBuf, &dotqmailfn) ||
				!stralloc_catb(&TmpBuf, "mail", 4) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		if (rename(dotqmailfn.s, TmpBuf.s)) {
			unlink(dotqmailfn.s);
			copy_status_mesg(html_text[144]);
			moduser();
			return;
		}
	}
	call_hooks(HOOK_MODUSER, ActionUser.s, Domain.s, Password1.s, Gecos.s);
	copy_status_mesg(html_text[119]);
	moduser();
}

/*
 * display ##i0 - ##i9 macros 
 */
void
parse_users_dotqmail(char newchar)
{
	static struct passwd *vpw = 0;
	char           *ptr;
	static int      fd1 = -1, fd2 = -1;
	int             match, j, inheader;
	static stralloc fn1 = {0}, fn2 = {0}, line = {0};
	static unsigned int dotqmail_flags = 0;
	char            inbuf1[1024], inbuf2[1024];
	struct substdio ssin1, ssin2;
#define DOTQMAIL_STANDARD	(1<<0)
#define DOTQMAIL_FORWARD	(1<<1)
#define DOTQMAIL_SAVECOPY	(1<<3)
#define DOTQMAIL_AUTORESP	(1<<4)
#define DOTQMAIL_BLACKHOLE	(1<<8)
#define DOTQMAIL_OTHERPGM	(1<<14)

	if (!vpw)
		vpw = sql_getpw(ActionUser.s, Domain.s);
	if (!vpw)
		return;
	if (fd1 == -1) {
		if (!stralloc_copys(&fn1, vpw->pw_dir) ||
				!stralloc_catb(&fn1, "/.qmail", 7) ||
				!stralloc_0(&fn1))
			die_nomem();
		if ((fd1 = open_read(fn1.s)) == -1)
			/*- no .qmail file, standard delivery */
			dotqmail_flags = DOTQMAIL_STANDARD;
		else {
			substdio_fdbuf(&ssin1, read, fd1, inbuf1, sizeof(inbuf1));
			for (;;) {
				if (getln(&ssin1, &line, &match, '\n') == -1) {
					strerr_warn3("parse_users_dotqmail: read: ", fn1.s, ": ", &strerr_sys);
					out(html_text[144]);
					out(" ");
					out(fn1.s);
					out(" 1<BR>\n");
					flush();
					iclose();
					if (fd1 != -1) {
						close(fd1);
						fd1 = -1;
					}
					if (fd2 != -1) {
						close(fd2);
						fd2 = -1;
					}
					iweb_exit(READ_FAILURE);
				}
				if (!line.len)
					break;
				if (match) {
					line.len--;
					line.s[line.len] = 0;
				} else {
					if (!stralloc_0(&line))
						die_nomem();
					line.len--;
				}
				switch (line.s[0])
				{
				case 0:	/* blank line, ignore */
					break;
				case '.':
				case '/': /* maildir delivery */
					/*- see if it's the user's maildir */
					if (!stralloc_copys(&TmpBuf, vpw->pw_dir) ||
							!stralloc_catb(&TmpBuf, "/Maildir/", 9) ||
							!stralloc_0(&TmpBuf))
						die_nomem();
					if (!str_diff(TmpBuf.s, line.s))
						dotqmail_flags |= DOTQMAIL_SAVECOPY;
					break;
				case '|': /* program delivery */
					/*-
					 * in older versions of QmailAdmin, we used "|/bin/true delete"
					 * for blackhole accounts.  Since the path to true may vary,
					 * just check for the end of the string
					 */
					if (str_str(line.s, "/true delete"))
						dotqmail_flags |= DOTQMAIL_BLACKHOLE;
					else
					if (str_str(line.s, "/autoresponder ") != NULL) {
						dotqmail_flags |= DOTQMAIL_AUTORESP;
						if (!stralloc_copy(&fn2, &RealDir) ||
								!stralloc_catb(&fn2, "/autoresp/", 10) ||
								!stralloc_cat(&fn2, &ActionUser) ||
								!stralloc_catb(&fn2, "/.autoresp.msg", 14) ||
								!stralloc_0(&fn2))
							die_nomem();
						fd2 = open_read(fn2.s);
					} else /* unrecognized program delivery, set a flag so we don't blackhole */
						dotqmail_flags |= DOTQMAIL_OTHERPGM;
					break;
				case '#': /* comment */
					/*- ignore unless it's our 'blackhole' comment */
					if (!str_diff(line.s, "# delete"))
						dotqmail_flags |= DOTQMAIL_BLACKHOLE;
					break;
				default: /* email address delivery */
					dotqmail_flags |= DOTQMAIL_FORWARD;
				}
			} /*- for (;;) */
			/*- if other flags were set, in addition to blackhole, clear blackhole flag */
			if (dotqmail_flags & (DOTQMAIL_FORWARD | DOTQMAIL_SAVECOPY))
				dotqmail_flags &= ~DOTQMAIL_BLACKHOLE;
			/*- if no flags were set (.qmail file without delivery), it's a blackhole */
			if (dotqmail_flags == 0)
				dotqmail_flags = DOTQMAIL_BLACKHOLE;
			/*- clear OTHERPGM flag, as it tells us nothing at this point */
			dotqmail_flags &= ~DOTQMAIL_OTHERPGM;
			/*- if forward is not set, clear save-a-copy */
			if (!(dotqmail_flags & DOTQMAIL_FORWARD))
				dotqmail_flags &= ~DOTQMAIL_SAVECOPY;
		}
		/*-
		 * if deleted and forward aren't set, 
		 * default to standard delivery 
		 */
		if (!(dotqmail_flags & (DOTQMAIL_BLACKHOLE | DOTQMAIL_FORWARD)))
			dotqmail_flags |= DOTQMAIL_STANDARD;
	}
	switch (newchar)
	{
		case '0': /* standard delivery checkbox */
		case '1': /* forward delivery checkbox */
		case '3': /* save-a-copy checkbox */
		case '4': /* autoresp checkbox */
		case '8': /* blackhole checkbox */
			if (dotqmail_flags & (1 << (newchar - '0'))) {
				out("checked ");
				flush();
			}
			break;
		case '2': /* forwarding addresses */
			if (fd1 != -1) {
				lseek(fd1, 0, SEEK_SET);
				substdio_fdbuf(&ssin1, read, fd1, inbuf1, sizeof(inbuf1));
				j = 0;
				for (;;) {
					if (getln(&ssin1, &line, &match, '\n') == -1) {
						strerr_warn3("parse_users_dotqmail: read: ", fn1.s, ": ", &strerr_sys);
						out(html_text[144]);
						out(" ");
						out(fn1.s);
						out(" 1<BR>\n");
						flush();
						iclose();
						if (fd1 != -1) {
							close(fd1);
							fd1 = -1;
						}
						if (fd2 != -1) {
							close(fd2);
							fd2 = -1;
						}
						iweb_exit(READ_FAILURE);
					}
					if (!line.len)
						break;
					if (match) {
						line.len--;
						line.s[line.len] = 0;
					} else {
						if (!stralloc_0(&line))
							die_nomem();
						line.len--;
					}
					switch (line.s[0])
					{
					case 0: /* blank line */
					case '/': /* maildir delivery */
					case '|': /* program delivery */
					case '#': /* comment */
						/*- ignore */
						break;
					default: /* email address delivery */
						/*- print address, skipping over '&' if necessary */
						if (j++)
							out(", ");
						printh("%H\n", line.s + (line.s[0] == '&' ? 1 : 0));
						flush();
					}
				}
			}
			break;
		case '5': /* autoresp subject */
			if (fd2 != -1) {
				lseek(fd2, 0, SEEK_SET);
				substdio_fdbuf(&ssin2, read, fd2, inbuf2, sizeof(inbuf2));
				/*- scan headers for Subject */
				for (;;) {
					if (getln(&ssin2, &line, &match, '\n') == -1) {
						strerr_warn3("parse_users_dotqmail: read: ", fn2.s, ": ", &strerr_sys);
						out(html_text[144]);
						out(" ");
						out(fn2.s);
						out(" 1<BR>\n");
						flush();
						iclose();
						if (fd1 != -1) {
							close(fd1);
							fd1 = -1;
						}
						if (fd2 != -1) {
							close(fd2);
							fd2 = -1;
						}
						iweb_exit(READ_FAILURE);
					}
					if (!line.len)
						break;
					if (line.s[0] == '\n')
						break;
					if (match) {
						line.len--;
						line.s[line.len] = 0;
					} else {
						if (!stralloc_0(&line))
							die_nomem();
						line.len--;
					}
					if (!case_diffb(line.s, 9, "Subject: ")) {
						if ((ptr = str_str(line.s, "Subject: Autoresponse - "))) {
							if ((ptr = str_str(line.s, "Re: ")))
								ptr += 4;
							else
								ptr = line.s + 24;
							printh("%H", ptr);
						} else
							printh("%H", line.s + 9);
					}
				}
			}
			break;
		case '6': /* autoresp message */
			if (fd2 != -1) {
				lseek(fd2, 0, SEEK_SET);
				substdio_fdbuf(&ssin2, read, fd2, inbuf2, sizeof(inbuf2));
				/* read from file, skipping headers (look for first blank line) */
				for (inheader = 1;;) {
					if (getln(&ssin2, &line, &match, '\n') == -1) {
						strerr_warn3("parse_users_dotqmail: read: ", fn2.s, ": ", &strerr_sys);
						out(html_text[144]);
						out(" ");
						out(fn2.s);
						out(" 1<BR>\n");
						flush();
						iclose();
						if (fd1 != -1) {
							close(fd1);
							fd1 = -1;
						}
						if (fd2 != -1) {
							close(fd2);
							fd2 = -1;
						}
						iweb_exit(READ_FAILURE);
					}
					if (!line.len)
						break;
					if (!stralloc_0(&line))
						die_nomem();
					line.len--;
					if (!inheader) {
						printh("%H", line.s);
						continue;
					} else
					if (line.s[0] == '\n')
						inheader = 0;
				}
			}
			break;
		case '7': /* gecos (real name) */
			printh("%H", vpw->pw_gecos);
			break;
	}
}

/*-
 * $Log: user.c,v $
 * Revision 1.36  2023-10-10 18:06:45+05:30  Cprogrammer
 * fixed user subscription to mailing list when adding user
 * delete user subscription from mailing list when deleting user
 *
 * Revision 1.35  2023-07-28 22:31:14+05:30  Cprogrammer
 * replaced exit with my_exit
 *
 * Revision 1.34  2023-07-14 21:52:39+05:30  Cprogrammer
 * set password field to start with {CRAM} when adding/modifying clear text passwords for CRAM authentication
 *
 * Revision 1.33  2022-11-02 16:21:15+05:30  Cprogrammer
 * add scram password if selected during user addition
 *
 * Revision 1.32  2022-10-27 17:34:35+05:30  Cprogrammer
 * added feature to create passwords for SCRAM for new user addition
 *
 * Revision 1.31  2022-10-26 22:30:20+05:30  Cprogrammer
 * refactored user modification page
 *
 * Revision 1.30  2022-09-15 23:11:39+05:30  Cprogrammer
 * replaced exit with _exit
 *
 */
