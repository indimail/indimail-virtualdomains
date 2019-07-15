/*
 * $Id: command.c,v 1.6 2019-06-03 06:46:21+05:30 Cprogrammer Exp mbhangui $
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#endif
#include "alias.h"
#include "autorespond.h"
#include "cgi.h"
#include "command.h"
#include "forward.h"
#include "mailinglist.h"
#include "printh.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "show.h"
#include "user.h"
#include "util.h"
#include "common.h"

void
process_commands(char *cmmd)
{
	static stralloc tmp = {0};

	if (!str_diff(cmmd, "showmenu")) {
		show_menu();
	} else
	if (!str_diff(cmmd, "quick")) {
		/*
		 * This feature sponsored by PinkRoccade Public Sector, Sept 2004 
		 */
		/*
		 * we use global ActionUser here because the functions that
		 * quickAction calls expect the username in that global.
		 */
		GetValue(TmpCGI, &ActionUser, "modu=");
		lowerit(ActionUser.s);	/* convert username to lower case */
		GetValue(TmpCGI, &tmp, "MODIFY=");
		if (tmp.len)
			quickAction(ActionUser.s, ACTION_MODIFY);
		else {
			GetValue(TmpCGI, &tmp, "DELETE=");
			if (tmp.len)
				quickAction(ActionUser.s, ACTION_DELETE);
			else {
				/*- malformed request -- missing fields */
				show_menu();
				iclose();
				exit(0);
			}
		}
	} else
	if (!str_diff(cmmd, "showusers")) {
		GetValue(TmpCGI, &tmp, "page=");
		str_copy(Pagenumber, tmp.s);
		GetValue(TmpCGI, &SearchUser, "searchuser=");
		show_users();
	} else
	if (!str_diff(cmmd, "showaliases")) {
		GetValue(TmpCGI, &tmp, "page=");
		str_copy(Pagenumber, tmp.s);
		show_aliases();
	} else
	if (!str_diff(cmmd, "showforwards")) {
		GetValue(TmpCGI, &tmp, "page=");
		str_copy(Pagenumber, tmp.s);
		GetValue(TmpCGI, &SearchUser, "searchuser=");
		show_forwards(Username.s, Domain.s, mytime);
	} else
	if (!str_diff(cmmd, "showmailinglists"))
		show_mailing_lists();
	else
	if (!str_diff(cmmd, "showautoresponders"))
		show_autoresponders(Username.s, Domain.s, mytime);
	else
	if (!str_diff(cmmd, "adduser"))
		adduser();
	else
	if (!str_diff(cmmd, "addusernow"))
		addusernow();
#ifdef CATCHALL_ENABLED
	else
	if (!str_diff(cmmd, "setdefault")) {
		GetValue(TmpCGI, &ActionUser, "deluser=");
		GetValue(TmpCGI, &tmp, "page=");
		str_copy(Pagenumber, tmp.s);
		setdefaultaccount();
	} else
	if (!str_diff(cmmd, "bounceall"))
		bounceall();
	else
	if (!str_diff(cmmd, "deleteall"))
		deleteall();
	else
	if (!str_diff(cmmd, "setremotecatchall"))
		setremotecatchall();
	else
	if (!str_diff(cmmd, "setremotecatchallnow"))
		setremotecatchallnow();
#endif
	else
	if (!str_diff(cmmd, "addlistmodnow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Newu, "newu=");
		addlistgroupnow(1);
	} else
	if (!str_diff(cmmd, "dellistmod")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		dellistgroup("del_listmod.html");
	} else
	if (!str_diff(cmmd, "dellistmodnow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Newu, "newu=");
		dellistgroupnow(1);
	} else
	if (!str_diff(cmmd, "addlistmod")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		addlistgroup("add_listmod.html");
	} else
	if (!str_diff(cmmd, "showlistmod")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		show_list_group("show_moderators.html");
	} else
	if (!str_diff(cmmd, "addlistdig")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		addlistgroup("add_listdig.html");
	} else
	if (!str_diff(cmmd, "addlistdignow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Newu, "newu=");
		addlistgroupnow(2);
	} else
	if (!str_diff(cmmd, "dellistdig")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		dellistgroup("del_listdig.html");
	} else
	if (!str_diff(cmmd, "dellistdignow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Newu, "newu=");
		dellistgroupnow(2);
	} else
	if (!str_diff(cmmd, "showlistdig")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		show_list_group("show_digest_subscribers.html");
	} else
	if (!str_diff(cmmd, "moduser")) {
		GetValue(TmpCGI, &ActionUser, "moduser=");
		moduser();
	} else
	if (!str_diff(cmmd, "modusernow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Password1, "password1=");
		GetValue(TmpCGI, &Password2, "password2=");
		GetValue(TmpCGI, &Gecos, "gecos=");
		modusergo();
	} else
	if (!str_diff(cmmd, "deluser")) {
		GetValue(TmpCGI, &ActionUser, "deluser=");
		ideluser();
	} else
	if (!str_diff(cmmd, "delusernow")) {
		GetValue(TmpCGI, &ActionUser, "deluser=");
		delusergo();
	} else
	if (!str_diff(cmmd, "moddotqmail")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		moddotqmail();
	} else
	if (!str_diff(cmmd, "moddotqmailnow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Newu, "newu=");
		GetValue(TmpCGI, &LineData, "linedata=");
		GetValue(TmpCGI, &Action, "action=");
		moddotqmailnow();
	} else
	if (!str_diff(cmmd, "deldotqmail")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		deldotqmail();
	} else
	if (!str_diff(cmmd, "deldotqmailnow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		deldotqmailnow();
	} else
	if (!str_diff(cmmd, "adddotqmail"))
		adddotqmail();
	else
	if (!str_diff(cmmd, "adddotqmailnow")) {
		GetValue(TmpCGI, &ActionUser, "newu=");
		GetValue(TmpCGI, &Alias, "alias=");
		adddotqmailnow();
	} else
	if (!str_diff(cmmd, "addmailinglist"))
		addmailinglist();
	else
	if (!str_diff(cmmd, "delmailinglist")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		delmailinglist();
	} else
	if (!str_diff(cmmd, "delmailinglistnow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		delmailinglistnow();
	} else
	if (!str_diff(cmmd, "addlistusernow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Newu, "newu=");
		addlistgroupnow(0);
	} else
	if (!str_diff(cmmd, "dellistuser")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		dellistgroup("del_listuser.html");
	} else
	if (!str_diff(cmmd, "dellistusernow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		GetValue(TmpCGI, &Newu, "newu=");
		dellistgroupnow(0);
	} else
	if (!str_diff(cmmd, "addlistuser")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		addlistgroup("add_listuser.html");
	} else
	if (!str_diff(cmmd, "addmailinglistnow")) {
		GetValue(TmpCGI, &ActionUser, "newu=");
		addmailinglistnow();
	} else
	if (!str_diff(cmmd, "modmailinglist")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		modmailinglist();
	} else
	if (!str_diff(cmmd, "modmailinglistnow")) {
		GetValue(TmpCGI, &ActionUser, "newu=");
		modmailinglistnow();
	} else
	if (!str_diff(cmmd, "modautorespond")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		modautorespond();
	} else
	if (!str_diff(cmmd, "addautorespond"))
		addautorespond();
	else
	if (!str_diff(cmmd, "addautorespondnow")) {
		GetValue(TmpCGI, &ActionUser, "newu=");
		GetValue(TmpCGI, &Alias, "alias="); /* subject */
		GetValue(TmpCGI, &Message, "message=");
		GetValue(TmpCGI, &Newu, "owner=");
		addautorespondnow();
	} else
	if (!str_diff(cmmd, "modautorespondnow")) {
		GetValue(TmpCGI, &ActionUser, "newu=");
		GetValue(TmpCGI, &Alias, "alias="); /* subject */
		GetValue(TmpCGI, &Message, "message=");
		GetValue(TmpCGI, &Newu, "owner=");
		modautorespondnow();
	} else
	if (!str_diff(cmmd, "showlistusers")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		show_list_group("show_subscribers.html");
	} else
	if (!str_diff(cmmd, "delautorespond")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		delautorespond();
	} else
	if (!str_diff(cmmd, "delautorespondnow")) {
		GetValue(TmpCGI, &ActionUser, "modu=");
		delautorespondnow();
	} else
	if (!str_diff(cmmd, "logout")) {
		if (!stralloc_copy(&TmpBuf, &RealDir) ||
				!stralloc_append(&TmpBuf, "/") ||
				!stralloc_cat(&TmpBuf, &Username) ||
				!stralloc_catb(&TmpBuf, "/Maildir", 8) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		del_id_files(&TmpBuf);
		show_login();
	} else
	if (!str_diff(cmmd, "showcounts"))
		show_counts();
	iclose();
	exit(0);
}

void
setdefaultaccount()
{
	struct passwd  *pw;
	int             fd, use_vfilter = 0, match, len, plen;
	char            inbuf[1024], outbuf[512];
	char           *ptr;
	struct substdio ssin, ssout;

	if ((fd = open_read(".qmail-default")) == -1) {
		out(html_text[144]);
		out(" .qmail-default<br>\n");
		flush();
		return;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn1("setdefaultaccount: read: .qmail-default", &strerr_sys);
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
	if (!(pw = sql_getpw(ActionUser.s, Domain.s))) {
		len = str_len(html_text[223]) + ActionUser.len + Domain.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H@%H", html_text[223], ActionUser.s, Domain.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else {
		if ((fd = open_trunc(".qmail-default")) == -1) {
			strerr_warn1("setdefaultaccount: open_trunc: .qmail-default: ", &strerr_sys);
			out(html_text[144]);
			out(" .qmail-default<br>\n");
			flush();
			return;
		} else {
			substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
			if (substdio_put(&ssout, "| ", 2) ||
					substdio_puts(&ssout, INDIMAILDIR) ||
					substdio_put(&ssout, "/sbin/", 6) ||
					substdio_put(&ssout, use_vfilter ? "vfilter" : "vdelivermail", use_vfilter ? 7 : 12) ||
					substdio_put(&ssout, " '' ", 4) ||
					substdio_put(&ssout, ActionUser.s, ActionUser.len) ||
					substdio_put(&ssout, "@", 1) ||
					substdio_put(&ssout, Domain.s, Domain.len)||
					substdio_put(&ssout, "\n", 1) ||
					substdio_flush(&ssout))
			{
				strerr_warn1("set_qmaildefault: write: ", &strerr_sys);
				out(html_text[144]);
				out(" .qmail-default<br>\n");
				flush();
				return;
			}
			close(fd);
		}
	}
	show_users();
	iclose();
	exit(0);
}
