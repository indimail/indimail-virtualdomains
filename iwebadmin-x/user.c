/*
 * $Id: user.c,v 1.16 2019-07-15 21:14:40+05:30 Cprogrammer Exp mbhangui $
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
#include <indimail_config.h>
#include <indimail.h>
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
#define HOOK_ADDMAILLIST "addmaillist"
#define HOOK_DELMAILLIST "delmaillist"
#define HOOK_MODMAILLIST "modmaillist"
#define HOOK_LISTADDUSER "addlistuser"
#define HOOK_LISTDELUSER "dellistuser"
#endif

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
		exit(0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn1("show_user_lines: read: .qmail-default: ", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default 1<BR>\n");
		flush();
		return (1);
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
	/*- End determine number of pages */
	scan_int(Pagenumber, &i);
	if (i == 0)
		str_copy(Pagenumber, "1");
	if (str_str(line.s, " bounce-no-mailbox\n")) {
		bounced = 1;
	} else
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
	scan_int(Pagenumber, &i);
	startnumber = MAXUSERSPERPAGE * (i - 1);

	/*
	 * check to see if there are any users to list, 
	 * otherwise repeat previous page
	 *  
	 */
	pw = sql_getall(dom, 1, 1);
	if (AdminType == DOMAIN_ADMIN || (AdminType == USER_ADMIN && !str_diffn(pw->pw_name, Username.s, Username.len))) {
		for (k = 0; k < startnumber; ++k)
			pw = sql_getall(dom, 0, 0);
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
	} else {
		while ((pw) &&
				((k < MAXUSERSPERPAGE + startnumber) ||
				 (AdminType != DOMAIN_ADMIN || AdminType != DOMAIN_ADMIN ||
				  (AdminType == USER_ADMIN && !str_diffn(pw->pw_name, Username.s, Username.len)))))
		{
			if (AdminType == DOMAIN_ADMIN || (AdminType == USER_ADMIN && !str_diffn(pw->pw_name, Username.s, Username.len))) {
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
			}
			pw = sql_getall(dom, 0, 0);
			++k;
		}
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
		exit(0);
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
		exit(0);
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
		iclose();
		exit(0);
	}
	send_template("mod_user.html");
}

void
addusernow()
{
	int             cnt = 0, num, pid, error, len, plen;
	char          **mailingListNames;
	static stralloc email = {0}, tmp1 = {0}, tmp2 = {0};
#ifdef MODIFY_QUOTA
	char            qconvert[11];
#endif
	char            strnum[FMT_ULONG];
	struct passwd  *mypw;

	count_users();
	load_limits();
	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		exit(0);
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
		exit(0);
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
		exit(0);
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
		exit(0);
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
		exit(0);
	}
#ifndef TRIVIAL_PASSWORD_ENABLED
	if (str_str(Newu.s, Password1.s)) {
		copy_status_mesg(html_text[320]);
		adduser();
		iclose();
		exit(0);
	}
#endif

#ifndef ENABLE_LEARN_PASSWORDS
	if (!Password1.len) {
		copy_status_mesg(html_text[234]);
		adduser();
		iclose();
		exit(0);
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
		if (!(mailingListNames = (char **) alloc(sizeof (char *) * num))) {
			copy_status_mesg(html_text[201]);
			iclose();
			exit(0);
		} else {
			for (cnt = 0; cnt < num; cnt++) {
				if (!stralloc_copys(&tmp1, "subscribe") ||
						!stralloc_catb(&tmp1, strnum, fmt_int(strnum, cnt)) ||
						!stralloc_0(&tmp1))
					die_nomem();
				if ((error = GetValue(TmpCGI, &tmp2, tmp1.s)) != -1) {
					if (!(pid = fork())) {
						if (!(mailingListNames[cnt] = (char *) alloc(tmp2.len + 1))) {
							copy_status_mesg(html_text[201]);
							iclose();
							exit(0);
						}
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
						exit(127);
					} else {
						wait(&pid);
					}
				}
			}
		}
	}
	/*- add the user then get the vpopmail password structure */
	create_flag = 1;
	if (iadduser(Newu.s, Domain.s, 0, Password1.s, Gecos.s, 0, 0, USE_POP, 1) == 0 &&
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
		if (!str_diffn(Quota.s, "NOQUOTA", 8)) {
			setuserquota(Newu.s, Domain.s, "NOQUOTA");
		} else
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
	if ((fd = open_read(hooks_path.s)) == -1) {
		/*- then try /etc/indimail */
		if (!stralloc_copys(&hooks_path, SYSCONFDIR) ||
				!stralloc_catb(&hooks_path, "/.iwebadmin-hooks", 17) ||
				!stralloc_0(&hooks_path))
			die_nomem();
		if ((fd = open_read(hooks_path.s)) == -1)
			return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("show_user_lines: read: ", hooks_path.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" .iwebadmin-hooks 1<BR>\n");
			flush();
			return (0);
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
		if (!*ptr)
			continue;
		match = str_chr(ptr, ' ');
		if (ptr[match] && !str_diffn(ptr, hook_type, match + 1)) {
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
delusergo()
{
	static stralloc forward = {0}, forwardto = {0};
	int             len, plen;

	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		exit(0);
	}
	if (deluser(ActionUser.s, Domain.s, 1) ) {
		copy_status_mesg(html_text[145]);
		iclose();
		exit(0);
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
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn1("set_qmaildefault: read: .qmail-default", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default 1<BR>\n");
		flush();
		return;
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
		return;
	}
	close(fd);
	use_vfilter = str_str(ptr, "vfilter") ? 1 : 0;

	if ((fd = open_trunc(".qmail-default")) == -1) {
		strerr_warn1("set_qmaildefault: open_trunc: .qmail-default: ", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default<br>\n");
		flush();
	} else {
		substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
		if (substdio_put(&ssout, "| ", 2) ||
				substdio_puts(&ssout, INDIMAILDIR) ||
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
		}
		close(fd);
	}
	show_users();
	iclose();
	exit(0);
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
		exit(0);
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

int
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
		exit(0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn1("get_catchall: read: .qmail-default", &strerr_sys);
		out(html_text[144]);
		out(" .qmail-default 1<BR>\n");
		flush();
		iclose();
		exit(0);
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
		iclose();
		exit(0);
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
	return 0;
}

int
makevacation(substdio *out, char *dir)
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
	/*- make the vacation directory */
	if (!stralloc_copys(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/vacation/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (r_mkdir(TmpBuf.s, 0750, Uid, Gid)) {
		copy_status_mesg(html_text[143]);
		strnum1[fmt_uint(strnum1, getuid())] = 0;
		strnum2[fmt_uint(strnum2, getgid())] = 0;
		strerr_warn7("makevacation: ", TmpBuf.s, ": uid =", strnum1, ", gid=", strnum2, ": ", &strerr_sys);
		return (1);
	}
	if (!stralloc_copys(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/content-type", 13) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (access(TmpBuf.s, R_OK)) {
		if (substdio_puts(out, INDIMAILDIR) ||
				substdio_put(out, "/bin/autoresponder -q ", 22) ||
				substdio_puts(out, dir) ||
				substdio_put(out, "/vacation", 9) ||
				substdio_put(out, ActionUser.s, ActionUser.len) ||
				substdio_put(out, "/.vacation.msg ", 15) ||
				substdio_puts(out, dir) ||
				substdio_put(out, "/vacation/", 10) ||
				substdio_put(out, ActionUser.s, ActionUser.len) ||
				substdio_put(out, "\n", 1) ||
				substdio_flush(out))
		{
			copy_status_mesg(html_text[144]);
			strerr_warn3("makevacation: write: ", TmpBuf.s, ": ", &strerr_sys);
			return (1);
		}
	} else {
		if (substdio_puts(out, INDIMAILDIR) ||
				substdio_put(out, "/bin/autoresponder -q -T ", 25) ||
				substdio_puts(out, dir) ||
				substdio_put(out, "/content-type ", 14) ||
				substdio_puts(out, dir) ||
				substdio_put(out, "/vacation", 9) ||
				substdio_put(out, ActionUser.s, ActionUser.len) ||
				substdio_put(out, "/.vacation.msg ", 15) ||
				substdio_puts(out, dir) ||
				substdio_put(out, "/vacation/", 10) ||
				substdio_put(out, ActionUser.s, ActionUser.len) ||
				substdio_put(out, "\n", 1) ||
				substdio_flush(out))
		{
			copy_status_mesg(html_text[144]);
			strerr_warn3("makevacation: write: ", TmpBuf.s, ": ", &strerr_sys);
			return (1);
		}
	}
	GetValue(TmpCGI, &Message, "vmessage=");
	/*- set up the message file */
	if (!stralloc_copys(&TmpBuf, dir) ||
			!stralloc_catb(&TmpBuf, "/vacation/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/.vacation.msg", 14) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if ((fd = open_trunc(TmpBuf.s)) == -1) {
		copy_status_mesg(html_text[150]);
		if (!stralloc_append(&StatusMessage, " ") ||
				!stralloc_cat(&StatusMessage, &TmpBuf) ||
				!stralloc_append(&StatusMessage, "\n") ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		return 1;
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_put(&ssout, "Reference: ", 11) ||
			substdio_put(&ssout, subject.s, subject.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "Subject: This is an autoresponse From: ", 39) ||
			substdio_put(&ssout, ActionUser.s, ActionUser.len) ||
			substdio_put(&ssout, "@", 1) ||
			substdio_put(&ssout, Domain.s, Domain.len) ||
			substdio_put(&ssout, " Re: ", 5) ||
			substdio_put(&ssout, subject.s, subject.len) ||
			substdio_put(&ssout, "\n\n", 2) ||
			substdio_put(&ssout, Message.s, Message.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout))
	{
		copy_status_mesg(html_text[144]);
		strerr_warn3("makevacation: write: ", TmpBuf.s, ": ", &strerr_sys);
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
	int             fd, ret_code, count, vacation = 0, saveacopy = 0, emptydotqmail, err, i;
	struct passwd  *vpw = 0;
	static stralloc box = {0}, cforward = {0}, dotqmailfn = {0}, triv_pass = {0};
	char           *olddotqmail = 0;
#ifdef MODIFY_QUOTA
	char           *quotaptr;
	char            qconvert[11];
#endif
	struct stat     sb;
	extern int      encrypt_flag;
	const char     *flagfields[] = { "zeroflag=", "oneflag=", "twoflag=", "threeflag=" };
	const gid_t     gidflags[] = { V_USER0, V_USER1, V_USER2, V_USER3 };
	gid_t           orig_gid;
	char            outbuf[512], strnum[FMT_ULONG];
	struct substdio ssout;

	if (!(AdminType == DOMAIN_ADMIN ||
			(AdminType == USER_ADMIN && !str_diffn(ActionUser.s, Username.s, ActionUser.len > Username.len ? ActionUser.len : Username.len)))) {
		copy_status_mesg(html_text[142]);
		iclose();
		exit(0);
	}
	if (Password1.len && Password2.len) {
		if (str_diff(Password1.s, Password2.s)) {
			copy_status_mesg(html_text[200]);
			moduser();
			iclose();
			exit(0);
		}
#ifndef TRIVIAL_PASSWORD_ENABLED
		if (str_str(ActionUser.s, Password1.s)) {
			copy_status_mesg(html_text[320]);
			moduser();
			iclose();
			exit(0);
		}
#endif
		if (!stralloc_copy(&triv_pass, &RealDir) ||
				!stralloc_catb(&triv_pass, "/.trivial_passwords", 19) ||
				!stralloc_0(&triv_pass))
			die_nomem();
		if (!access(triv_pass.s, F_OK))
			encrypt_flag = 1;
		ret_code = ipasswd(ActionUser.s, Domain.s, Password1.s, USE_POP);
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
#ifdef MODIFY_QUOTA
	/*
	 * strings used: 307 = "Invalid Quota", 308 = "Quota set to unlimited",
	 * 309 = "Quota set to %s bytes"
	 */
	if (AdminType == DOMAIN_ADMIN) {
		GetValue(TmpCGI, &Quota, "quota=");
		scan_long(Quota.s, &q);
		vpw = sql_getpw(ActionUser.s, Domain.s);
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
	vpw = sql_getpw(ActionUser.s, Domain.s);
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
		sql_setpw(vpw, Domain.s);
	} else 
	if (vpw->pw_gid != orig_gid)
		sql_setpw(vpw, Domain.s);
	/*- get the value of the vacation checkbox */
	GetValue(TmpCGI, &box, "vacation=");
	if (!str_diff(box.s, "on"))
		vacation = 1;
	/*- if they want to save a copy */
	GetValue(TmpCGI, &box, "fsaved=");
	if (!str_diff(box.s, "on"))
		saveacopy = 1;
	/*- get the value of the cforward radio button */
	GetValue(TmpCGI, &cforward, "cforward=");
	if (!str_diff(cforward.s, "vacation"))
		vacation = 1;
	/*- open old .qmail file if it exists and load it into memory */
	if (!stralloc_copys(&dotqmailfn, vpw->pw_dir) ||
			!stralloc_catb(&dotqmailfn, "/.qmail", 7) ||
			!stralloc_0(&dotqmailfn))
		die_nomem();
	if (!(err = stat(dotqmailfn.s, &sb))) {
		if ((olddotqmail = (char *) alloc(sb.st_size))) {
			if ((fd = open_read(dotqmailfn.s)) != -1) {
				if (read(fd, olddotqmail, sb.st_size) == -1)
					strerr_warn2(dotqmailfn.s, ": read: ", &strerr_sys);
				close(fd);
			}
		}
	}
	if ((fd = open_trunc(dotqmailfn.s)) == -1) {
		copy_status_mesg(html_text[150]);
		if (!stralloc_catb(&StatusMessage, ": ", 2) ||
				!stralloc_cats(&StatusMessage, error_str(errno)) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		out("<h3>");
		out(dotqmailfn.s);
		out(" ");
		out(html_text[150]);
		out(": ");
		out(error_str(errno));
		out("</h3>\n");
		flush();
		iclose();
		exit(0);
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	/*
	 * Scan through old .qmail and write out any unrecognized program delivery
	 * lines to the new .qmail file.
	 */
	emptydotqmail = 1;
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
			copy_status_mesg(html_text[215]);
			err = 1;
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
					emptydotqmail = 0;
					++count;
				}
				tmpstr = str_tok(NULL, " ,;\n");
			}
		}
	}
	if (str_diff(cforward.s, "forward") || saveacopy) {
		if (!str_diff(cforward.s, "blackhole")) {
			substdio_put(&ssout, "# delete\n", 9);
			emptydotqmail = 0;
		} else {
			substdio_puts(&ssout, vpw->pw_dir);
			substdio_put(&ssout, "/Maildir/\n", 10);
		}
		/*- this isn't enough to consider the .qmail file non-empty */
	}
	if (vacation) {
		err = makevacation(&ssout, RealDir.s);
		emptydotqmail = 0;
	} else {
		/*- delete old vacation directory */
		if (!stralloc_copy(&TmpBuf, &RealDir) ||
				!stralloc_catb(&TmpBuf, "/vacation", 9) ||
				!stralloc_cat(&TmpBuf, &ActionUser) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		vdelfiles(TmpBuf.s, ActionUser.s, Domain.s);
	}
	close(fd);
	if (emptydotqmail)
		unlink(dotqmailfn.s);
	if (err) {
		moduser();
		iclose();
		exit(0);
	}
	call_hooks(HOOK_MODUSER, ActionUser.s, Domain.s, Password1.s, Gecos.s);
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
	int             match, j;
	static stralloc fn1 = {0}, fn2 = {0}, line = {0};
	int             inheader;
	static unsigned int dotqmail_flags = 0;
	char            inbuf1[1024], inbuf2[1024];
	struct substdio ssin1, ssin2;
#define DOTQMAIL_STANDARD	(1<<0)
#define DOTQMAIL_FORWARD	(1<<1)
#define DOTQMAIL_SAVECOPY	(1<<3)
#define DOTQMAIL_VACATION	(1<<4)
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
		fd1 = open_read(fn1.s);
		if (fd1 == -1) {
			/*- no .qmail file, standard delivery */
			dotqmail_flags = DOTQMAIL_STANDARD;
		} else {
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
					exit(0);
				}
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
				case 0:	 /* blank line, ignore */
					break;
				case '.':
				case '/': /* maildir delivery */
					/*- see if it's the user's maildir */
					if (1)
						dotqmail_flags |= DOTQMAIL_SAVECOPY;
					break;
				case '|':		/* program delivery */
					/*-
					 * in older versions of QmailAdmin, we used "|/bin/true delete"
					 * for blackhole accounts.  Since the path to true may vary,
					 * just check for the end of the string
					 */
					if (str_str(line.s, "/true delete"))
						dotqmail_flags |= DOTQMAIL_BLACKHOLE;
					else
					if (str_str(line.s, "/autoresponder ") != NULL) {
						dotqmail_flags |= DOTQMAIL_VACATION;
						if (!stralloc_copy(&fn2, &RealDir) ||
								!stralloc_catb(&fn2, "/vacation/", 10) ||
								!stralloc_cat(&fn2, &ActionUser) ||
								!stralloc_catb(&fn2, "/.vacation.msg", 14) ||
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
			}
			/*
			 * if other flags were set, in addition to blackhole, clear blackhole flag 
			 */
			if (dotqmail_flags & (DOTQMAIL_FORWARD | DOTQMAIL_SAVECOPY))
				dotqmail_flags &= ~DOTQMAIL_BLACKHOLE;
			/*
			 * if no flags were set (.qmail file without delivery), it's a blackhole 
			 */
			if (dotqmail_flags == 0)
				dotqmail_flags = DOTQMAIL_BLACKHOLE;
			/*
			 * clear OTHERPGM flag, as it tells us nothing at this point 
			 */
			dotqmail_flags &= ~DOTQMAIL_OTHERPGM;
			/*
			 * if forward and save-a-copy are set, it will actually set the spam flag 
			 */
			if ((dotqmail_flags & DOTQMAIL_FORWARD))
				dotqmail_flags |= DOTQMAIL_SAVECOPY;
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
		case '4': /* vacation checkbox */
		case '8': /* blackhole checkbox */
		case '9': /* spam check checkbox */
			if (dotqmail_flags & (1 << (newchar - '0'))) {
				out("checked ");
				flush();
			}
			break;
		case '2': /* forwarding addresses */
			if (fd1 != -1) {
				lseek(fd1, 0, SEEK_SET);
				ssin1.p = 0;
				ssin1.n = sizeof(inbuf1);
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
						exit(0);
					}
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
		case '5': /* vacation subject */
			if (fd2 != -1) {
				substdio_fdbuf(&ssin2, read, fd2, inbuf2, sizeof(inbuf2));
				lseek(fd2, 0, SEEK_SET);
				ssin2.p = 0;
				ssin2.n = sizeof(inbuf2);
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
						exit(0);
					}
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
						if ((ptr = str_str(line.s, "Subject: This is an autoresponse From:"))) {
							if ((ptr = str_str(line.s, "Re: ")))
								ptr += 4;
							else
								ptr += 39;
							printh("%H", ptr);
						} else
							printh("%H", line.s + 9);
					}
					if (!case_diffb(line.s, 11, "Reference: "))
						printh("%H", line.s + 11);
				}
			}
			break;
		case '6': /* vacation message */
			if (fd2 != -1) {
				substdio_fdbuf(&ssin2, read, fd2, inbuf2, sizeof(inbuf2));
				lseek(fd2, 0, SEEK_SET);
				ssin2.p = 0;
				ssin2.n = sizeof(inbuf2);
				/* read from file, skipping headers (look for first blank line) */
				inheader = 1;
				for (;;) {
					if (getln(&ssin2, &line, &match, '\n') == -1) {
						strerr_warn3("parse_users_dotqmail: read: ", fn2.s, ": ", &strerr_sys);
						out(html_text[144]);
						out(" ");
						out(fn2.s);
						out(" 1<BR>\n");
						flush();
						iclose();
						exit(0);
					}
					if (!inheader)
						printh("%H", line.s);
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
