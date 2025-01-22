/*
 * $Id: autorespond.c,v 1.19 2024-05-30 22:55:32+05:30 Cprogrammer Exp mbhangui $
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
#define REMOVE_MYSQL_H
#include <indimail_compat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_QMAIL
#include <fmt.h>
#include <error.h>
#include <str.h>
#include <strerr.h>
#include <substdio.h>
#include <open.h>
#endif
#include "autorespond.h"
#include "printh.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "show.h"
#include "template.h"
#include "util.h"
#include "common.h"

void
show_autoresponders(const char *user, const char *dom, time_t mytime)
{
	if (MaxAutoResponders == 0)
		return;
	count_autoresponders();
	if (CurAutoResponders == 0) {
		copy_status_mesg(html_text[233]);
		show_menu();
	} else {
		send_template("show_autorespond.html");
	}
}

void
show_autorespond_line(const char *user, const char *dom, time_t mytime, const char *dir)
{
	const char     *addr, *domptr;
	int             i;
	DIR            *mydir;
	struct dirent  *mydirent;
	struct passwd  *vpw;

	sort_init();
	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp", 9) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (access(TmpBuf.s, F_OK) && errno == 2)
		return;
	if (!(mydir = opendir(TmpBuf.s))) {
		out("<tr><td>");
		out("unable to open directory ");
		out(TmpBuf.s);
		out(": ");
		out(error_str(errno));
		out(" 1</tr><td>");
		flush();
		return;
	}
	while ((mydirent = readdir(mydir))) {
		if (mydirent->d_name[0] == '.')
			continue;
		sort_add_entry(mydirent->d_name, 0);
	}
	i = str_rchr(RealDir.s, '/');
	if (RealDir.s[i])
		domptr = RealDir.s + i + 1;
	else
		domptr = dom;
	sort_dosort();
	for (i = 0; (addr = (char *) sort_get_entry(i)); ++i) {
		out("<tr>");
		out("<td align=\"center\">");
		printh("<a href=\"%s&modu=%C\">", cgiurl("delautorespond"), addr);
		out("<img src=\"");
		out(IMAGEURL);
		out("/trash.png\" border=\"0\"></a>");
		out("</td>");
		out("<td align=\"center\">");
		if ((vpw = sql_getpw(addr, domptr)))
			printh("<a href=\"%s&moduser=%C\">", cgiurl("moduser"), addr);
		else
			printh("<a href=\"%s&modu=%C\">", cgiurl("modautorespond"), addr);
		out("<img src=\"");
		out(IMAGEURL);
		out("/modify.png\" border=\"0\"></a>");
		out("</td>");
		printh("<td align=\"left\">%H@%H</td>", addr, Domain.s);
		out("</tr>\n");
	}
	flush();
	sort_cleanup();
}

void
addautorespond()
{
	char            strnum[FMT_ULONG];

	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	count_autoresponders();
	if (MaxAutoResponders != -1 && CurAutoResponders >= MaxAutoResponders) {
		out(html_text[158]);
		out(" ");
		strnum[fmt_uint(strnum, MaxAutoResponders)] = 0;
		out(strnum);
		out("\n");
		flush();
		show_menu();
		iclose();
		iweb_exit(LIMIT_FAILURE);
	}
	send_template("add_autorespond.html");

}

void
addautorespondnow()
{
	int             i = 0, len, plen, fd, flag = 0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], outbuf[1024];
	struct substdio ssout;

	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	count_autoresponders();
	if (MaxAutoResponders != -1 && CurAutoResponders >= MaxAutoResponders) {
		out(html_text[158]);
		out(" ");
		strnum1[fmt_uint(strnum1, MaxAutoResponders)] = 0;
		out(strnum1);
		out("\n");
		flush();
		show_menu();
		iclose();
		iweb_exit(LIMIT_FAILURE);
	}
	if (fixup_local_name(ActionUser.s)) {
		len = str_len(html_text[174]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[174], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else
	if (check_local_user(ActionUser.s)) {
		len = str_len(html_text[175]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[175], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else
	if (!ActionUser.len)
		copy_status_mesg(html_text[176]);
	else
	if (Newu.len > 0 && check_email_addr(Newu.s)) {
		len = str_len(html_text[177]) + Newu.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[177], Newu.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else
	if (!Alias.len) {
		len = str_len(html_text[178]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[178], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else
	if (!Message.len) {
		len = str_len(html_text[179]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[179], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	}
	/*- if there was an error, go back to the add screen */
	if (StatusMessage.len) {
		addautorespond();
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
	/*- Make the autoresponder directory */
	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if (r_mkdir(TmpBuf.s, 0750, Uid, Gid)) {
		strnum1[fmt_uint(strnum1, getuid())] = 0;
		strnum2[fmt_uint(strnum2, getgid())] = 0;
		strerr_warn7("mkdir: ", TmpBuf.s, ": uid=", strnum1, ", gid=", strnum2, ": ", &strerr_sys);
		ack("143", "autoresp/user");
	}
	/*- Make the autoresponder message file */
	if (!stralloc_catb(&TmpBuf, "/.autoresp.msg", 14) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if ((fd = open_trunc(TmpBuf.s)) == -1) {
		strerr_warn3("open: ", TmpBuf.s, ": ", &strerr_sys);
		ack("144", "write .autoresp.msg");
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
	/*- subject in iwebadmin autoresponder panel */
	if (substdio_put(&ssout, "Reference: ", 11) ||
			substdio_put(&ssout, Alias.s, Alias.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "Subject: ", 9) ||
			substdio_put(&ssout, Alias.s, Alias.len) ||
			substdio_put(&ssout, "\n", 1))
	{
		strerr_warn3("write: ", TmpBuf.s, ": ", &strerr_sys);
		ack("144", "write .autoresp.msg");
	}
	for (i = 400; i < 450; i++) {
		if (html_text[i] == NULL)
			break;
		if ((*(html_text[i]) == ' ') || (*(html_text[i]) == '\t') || 
			(*(html_text[i]) == '\r') || (*(html_text[i]) == '\n') || (!(*(html_text[i]))))
			continue;
		if (substdio_puts(&ssout, html_text[i]) ||
				substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("write: ", TmpBuf.s, ": ", &strerr_sys);
			ack("144", "write .autoresp.msg");
		}
	}
	if (substdio_put(&ssout, "MIME-Version: 1.0\n", 18) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, Message.s, Message.len) ||
			substdio_flush(&ssout))
	{
		strerr_warn3("write: ", TmpBuf.s, ": ", &strerr_sys);
		ack("144", "write .autoresp.msg");
	}
	close(fd);
	/*- Make the autoresponder .qmail file */
	valias_delete(ActionUser.s, Domain.s, 0);
	if (Newu.len > 0) {
		if (!stralloc_copyb(&TmpBuf, "&", 1) ||
				!stralloc_cat(&TmpBuf, &Newu) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		valias_insert(ActionUser.s, Domain.s, TmpBuf.s, 1);
	}
	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/content-type", 13) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (!access(TmpBuf.s, R_OK))
		flag = 1;
	if (!stralloc_copyb(&TmpBuf, "|", 1) ||
			!stralloc_cats(&TmpBuf, PREFIX) ||
			!stralloc_catb(&TmpBuf, "/bin/autoresponder -q ", 22))
		die_nomem();
	if (flag) {
		if (!stralloc_catb(&TmpBuf, "-T ", 3) ||
				!stralloc_cat(&TmpBuf, &RealDir) ||
				!stralloc_catb(&TmpBuf, "/content-type ", 14))
			die_nomem();
	}
	if (!stralloc_cat(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/.autoresp.msg ", 15) ||
			!stralloc_cat(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	valias_insert(ActionUser.s, Domain.s, TmpBuf.s, 1);
	/*- Report success */
	len = str_len(html_text[180]) + ActionUser.len + Domain.len + 28;
	for (plen = 0;;) {
		if (!stralloc_ready(&StatusMessage, len))
			die_nomem();
		plen = snprinth(StatusMessage.s, len, "%s %H@%H\n", html_text[180], ActionUser.s, Domain.s);
		if (plen < len) {
			StatusMessage.len = plen;
			break;
		}
		len = plen + 28;
	}
	show_autoresponders(Username.s, Domain.s, mytime);
}

void
delautorespond()
{
	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	send_template("del_autorespond_confirm.html");
}

void
delautorespondnow()
{
	int             len, plen;
	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	/*- delete the alias */
	valias_delete(ActionUser.s, Domain.s, 0);
	/*- delete the autoresponder directory */
	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	vdelfiles(TmpBuf.s, ActionUser.s, Domain.s);
	len = str_len(html_text[182]) + ActionUser.len + 28;
	for (plen = 0;;) {
		if (!stralloc_ready(&StatusMessage, len))
			die_nomem();
		plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[182], ActionUser.s);
		if (plen < len) {
			StatusMessage.len = plen;
			break;
		}
		len = plen + 28;
	}
	count_autoresponders();
	if (CurAutoResponders == 0)
		show_menu();
	else
		send_template("show_autorespond.html");
}

void
modautorespond()
{
	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	/*- send_template("show_forwards.html"); -*/
	send_template("mod_autorespond.html");
}


/*
 * addautorespondnow and modautorespondnow should be merged into a single function 
 */
void
modautorespondnow()
{
	int             fd, len, plen, flag = 0;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], outbuf[1024];
	struct substdio ssout;

	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		iweb_exit(PERM_FAILURE);
	}
	if (fixup_local_name(ActionUser.s)) {
		len = str_len(html_text[174]) + ActionUser.len;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[174], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else
	if (Newu.len > 0 && check_email_addr(Newu.s)) {
		len = str_len(html_text[177]) + Newu.len;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[177], Newu.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else /*- subject */
	if (!Alias.len) {
		len = str_len(html_text[178]) + ActionUser.len;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[178], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else
	if (!Message.len) {
		len = str_len(html_text[179]) + ActionUser.len;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[179], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	}
	/*- exit on errors */
	if (StatusMessage.len) {
		modautorespond();
		iclose();
		iweb_exit(INPUT_FAILURE);
	}
	/*- Make the autoresponder directory */
	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if (r_mkdir(TmpBuf.s, 0750, Uid, Gid)) {
		strnum1[fmt_uint(strnum1, getuid())] = 0;
		strnum2[fmt_uint(strnum2, getgid())] = 0;
		strerr_warn7("mkdir: ", TmpBuf.s, ": uid=", strnum1, ", gid=", strnum2, ": ", &strerr_sys);
		ack("143", "autoresp/user");
	}
	/*- Make the autoresponder message file */
	if (!stralloc_catb(&TmpBuf, "/.autoresp.msg", 14) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if ((fd = open_trunc(TmpBuf.s)) == -1) {
		strerr_warn3("open: ", TmpBuf.s, ": ", &strerr_sys);
		ack("144", ".autoresp");
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
	if (substdio_put(&ssout, "Reference: ", 11) ||
			substdio_put(&ssout, Alias.s, Alias.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "Subject: ", 9) ||
			substdio_put(&ssout, Alias.s, Alias.len) ||
			substdio_put(&ssout, "\n\n", 2) || 
			substdio_put(&ssout, Message.s, Message.len) ||
			substdio_flush(&ssout))
	{
		strerr_warn3("write: ", TmpBuf.s, ": ", &strerr_sys);
		ack("144", ".autoresp");
	}
	close(fd);
	/*- Make the autoresponder .qmail file */
	valias_delete(ActionUser.s, Domain.s, 0);
	if (Newu.len > 0) {
		if (!stralloc_copyb(&TmpBuf, "&", 1) ||
				!stralloc_cat(&TmpBuf, &Newu) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		valias_insert(ActionUser.s, Domain.s, TmpBuf.s, 1);
	}
	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/content-type", 13) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (!access(TmpBuf.s, R_OK))
		flag = 1;
	if (!stralloc_copyb(&TmpBuf, "|", 1) ||
			!stralloc_cats(&TmpBuf, PREFIX) ||
			!stralloc_catb(&TmpBuf, "/bin/autoresponder -q ", 22))
		die_nomem();
	if (flag) {
		if (!stralloc_catb(&TmpBuf, "-T ", 3) ||
				!stralloc_cat(&TmpBuf, &RealDir) ||
				!stralloc_catb(&TmpBuf, "/content-type ", 14))
			die_nomem();
	}
	if (!stralloc_cat(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/.autoresp.msg ", 15) ||
			!stralloc_cat(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp/", 10) ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	valias_insert(ActionUser.s, Domain.s, TmpBuf.s, 1);
	/*- Report success */
	len = str_len(html_text[183]) + ActionUser.len + Domain.len + 28;
	for (plen = 0;;) {
		if (!stralloc_ready(&StatusMessage, len))
			die_nomem();
		plen = snprinth(StatusMessage.s, len, "%s %H@%H\n", html_text[183], ActionUser.s, Domain.s);
		if (plen < len) {
			StatusMessage.len = plen;
			break;
		}
		len = plen + 28;
	}
	show_autoresponders(Username.s, Domain.s, mytime);
}

void
count_autoresponders()
{
	static stralloc alias_name = {0};
	char           *alias_line;
	DIR            *mydir;
	struct dirent  *mydirent;

	CurAutoResponders = 0;
	for (;;) {
		if (!(alias_line = valias_select_all(&alias_name, &Domain)))
			break;
		if (str_str(alias_line, "/autoresponder ") != 0)
			CurAutoResponders++;
	}
	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_catb(&TmpBuf, "/autoresp", 9) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (access(TmpBuf.s, F_OK) && errno == 2)
		return;
	if (!(mydir = opendir(TmpBuf.s))) {
		out("<tr><td>");
		out(html_text[143]);
		out(" 1</tr><td>");
		flush();
		return;
	}
	while ((mydirent = readdir(mydir)) != NULL) {
		if (mydirent->d_name[0] == '.')
			continue;
		CurAutoResponders++;
	}
}
