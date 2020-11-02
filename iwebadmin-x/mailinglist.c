/*
 * $Id: mailinglist.c,v 1.18 2020-11-02 15:03:17+05:30 Cprogrammer Exp mbhangui $
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
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <indimail.h>
#include <indimail_compat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <substdio.h>
#include <getln.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <case.h>
#include <strerr.h>
#include <open.h>
#endif
#include "cgi.h"
#include "limits.h"
#include "mailinglist.h"
#include "printh.h"
#include "iwebadminx.h"
#include "iwebadmin.h"
#include "show.h"
#include "template.h"
#include "util.h"
#include "common.h"

int             rename(const char *, const char *);

static stralloc dotqmail_name = {0}, replyto_addr = {0};
static int      replyto;
static int      dotnum;
extern int      ezmlm_idx, ezmlm_make;
static int      checkopt[256]; /* used to display mailing list options */

#define REPLYTO_SENDER 1
#define REPLYTO_LIST 2
#define REPLYTO_ADDRESS 3
#define GROUP_SUBSCRIBER 0
#define GROUP_MODERATOR 1
#define GROUP_DIGEST 2

void            set_options();
void            default_options();

void
show_mailing_lists()
{
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}

	if (!ezmlm_make || MaxMailingLists == 0)
		return;
	/*- see if there's anything to display */
	count_mailinglists();
	if (CurMailingLists == 0) {
		copy_status_mesg(html_text[231]);
		show_menu();
		iclose();
		exit(0);
	}
	send_template("show_mailinglist.html");
}

void
show_mailing_list_line(char *user, char *dom, time_t mytime, char *dir)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	int             fd, i, j, match, ok;
	char           *addr;
	char            inbuf[1024];
	static stralloc testfn = {0}, line = {0};
	struct substdio ssin;

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	if (!ezmlm_make || MaxMailingLists == 0)
		return;
	if (!(mydir = opendir("."))) {
		out("<tr><td>");
		out(html_text[143]);
		out(" 1</tr><td>");
		flush();
		return;
	}
	/*- First display the title row */
	out("<tr bgcolor=\"#cccccc\">");
	out("<th align=center><font size=2>");
	out(html_text[72]);
	out("</font></th>");
	if (ezmlm_idx) {
		out("<th align=center><font size=2>");
		out(html_text[71]);
		out("</font></th>");
	}
	out("<th align=center><font size=2>");
	out(html_text[81]);
	out("</font></th>");
	out("<th align=center><font size=2>");
	out(html_text[83]);
	out("</font></th>");
	out("<th align=center><font size=2>");
	out(html_text[84]);
	out("</font></th>");
	out("<th align=center><font size=2>");
	out(html_text[85]);
	out("</font></th>");
	if (ezmlm_idx) {
		out("<th align=center><font size=2>");
		out(html_text[86]);
		out("</font></th>");
		out("<th align=center><font size=2>");
		out(html_text[87]);
		out("</font></th>");
		out("<th align=center><font size=2>");
		out(html_text[88]);
		out("</font></th>");
		out("<th align=center><font size=2>");
		out(html_text[237]);
		out("</font></th>");
		out("<th align=center><font size=2>");
		out(html_text[238]);
		out("</font></th>");
		out("<th align=center><font size=2>");
		out(html_text[239]);
		out("</font></th>");
	}
	out("</tr>\n");
	sort_init();
	/*- Now, display each list */
	while ((mydirent = readdir(mydir))) {
		if (!str_diffn(".qmail-", mydirent->d_name, 7)) {
			if ((fd = open_read(mydirent->d_name)) == -1) {
				if (ezmlm_idx)
					out("<tr><td colspan=12>");
				else
					out("<tr><td colspan=5>");
				out(html_text[144]);
				out(" ");
				out(mydirent->d_name);
				out("</td></tr>\n");
				continue;
			}
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				if (ezmlm_idx)
					out("<tr><td colspan=12>");
				else
					out("<tr><td colspan=5>");
				out(html_text[144]);
				out(" ");
				out(mydirent->d_name);
				out("</td></tr>\n");
				continue;
			}
			close(fd);
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
			if (line.len && str_str(line.s, "ezmlm-reject"))
				sort_add_entry(mydirent->d_name + 7, 0);
		}
	}
	closedir(mydir);
	sort_dosort();
	for (i = 0; (addr = (char *) sort_get_entry(i)); ++i) {
		if (!stralloc_copys(&testfn, addr) ||
				!stralloc_catb(&testfn, "/digested", 9) ||
				!stralloc_0(&testfn))
			die_nomem();
		if (!access(testfn.s, F_OK))
			ok = 1;
		else {
			ok = 0;
			if (!stralloc_copyb(&testfn, ".qmail-", 7) ||
					!stralloc_cats(&testfn, addr) ||
					!stralloc_catb(&testfn, "-digest-owner", 13) ||
					!stralloc_0(&testfn))
				die_nomem();
		}
		/*- convert ':' in addr to '.' */
		str_replace(addr, ':', '.');

		out("<tr>");
		qmail_button(addr, "delmailinglist", user, dom, mytime, "trash.png");

		if (ezmlm_idx)
			qmail_button(addr, "modmailinglist", user, dom, mytime, "modify.png");
		printh("<td align=left>%H</td>\n", addr);

		qmail_button(addr, "addlistuser", user, dom, mytime, "delete.png");
		qmail_button(addr, "dellistuser", user, dom, mytime, "delete.png");
		qmail_button(addr, "showlistusers", user, dom, mytime, "delete.png");

		if (ezmlm_idx) {
			qmail_button(addr, "addlistmod", user, dom, mytime, "delete.png");
			qmail_button(addr, "dellistmod", user, dom, mytime, "delete.png");
			qmail_button(addr, "showlistmod", user, dom, mytime, "delete.png");

			if (!ok && (fd = open_read(testfn.s)) != -1) {
				substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
				if (getln(&ssin, &line, &match, '\n') == -1)
					ok = 0;
				else
				if (line.len) {
					if (match) {
						line.len--;
						line.s[line.len] = 0;
					} else {
						if (!stralloc_0(&line))
							die_nomem();
						line.len--;
					}
					j = str_rchr(line.s, '/');
					if (line.s[j] && !str_diffn(line.s + i, "/Mailbox", 8))
						ok = 0;
					else
						ok = 1;
				} else
					ok = 0;
				close(fd);
			} 
			/*- Is it a digest list?  */
			if (ok) {
				qmail_button(addr, "addlistdig", user, dom, mytime, "delete.png");
				qmail_button(addr, "dellistdig", user, dom, mytime, "delete.png");
				qmail_button(addr, "showlistdig", user, dom, mytime, "delete.png");
			} else {
				/*- not a digest list */
				out("<TD COLSPAN=3> </TD>");
			}
		}
		out("</tr>\n");
	}
	flush();
	sort_cleanup();
}

/*
 * mailing list lines on the add user page 
 */
void
show_mailing_list_line2(char *user, char *dom, time_t mytime, char *dir)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	char           *addr;
	int             i, fd, listcount, match;
	char            inbuf[1024], strnum[FMT_ULONG];
	static stralloc line = {0};
	struct substdio ssin;

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	if (!ezmlm_make || MaxMailingLists == 0)
		return;
	if (!(mydir = opendir("."))) {
		out(html_text[143]);
		out(" 1<BR>\n");
		flush();
		return;
	}
	listcount = 0;
	sort_init();
	while ((mydirent = readdir(mydir)) != NULL) {
		if (!str_diffn(".qmail-", mydirent->d_name, 7)) {
			if ((fd = open_read(mydirent->d_name)) == -1) {
				out(html_text[144]);
				out(" ");
				out(mydirent->d_name);
				out("<br>\n");
				continue;
			}
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				if (ezmlm_idx)
					out("<tr><td colspan=12>");
				else
					out("<tr><td colspan=5>");
				out(html_text[144]);
				out(" ");
				out(mydirent->d_name);
				out("</td></tr>\n");
				continue;
			}
			close(fd);
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
			if (line.len && str_str(line.s, "ezmlm-reject")) {
				sort_add_entry(mydirent->d_name + 7, 0);
				listcount++;
			}
		}
	}
	closedir(mydir);
	/*- if there aren't any lists, don't display anything */
	if (listcount == 0) {
		sort_cleanup();
		return;
	}
	out("<hr><table width=100% cellpadding=1 cellspacing=0 border=0");
	out(" align=center bgcolor=\"#000000\"><tr><td>");
	out("<table width=100% cellpadding=0 cellspacing=0 border=0 bgcolor=\"#e6e6e6\">");
	out("<tr><th bgcolor=\"#000000\" colspan=2>");
	out("<font color=\"#ffffff\">");
	out(html_text[95]);
	out("</font></th>\n");
	sort_dosort();
	out("<INPUT NAME=number_of_mailinglist TYPE=hidden VALUE=");
	strnum[fmt_int(strnum, listcount)] = 0;
	out(strnum);
	out(">\n");
	for (i = 0; i < listcount; ++i) {
		addr = (char *) sort_get_entry(i);
		str_replace(addr, ':', '.');
		printh("<TR><TD ALIGN=RIGHT><INPUT NAME=\"subscribe%d\" TYPE=checkbox VALUE=\"%H\"></TD>", i, addr);
		printh("<TD align=LEFT>%H@%H</TD></TR>", addr, Domain.s);
	}
	out("</table></td></tr></table>\n");
	flush();
	sort_cleanup();
}

void
addmailinglist()
{
	char            strnum[FMT_ULONG];

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}

	if (!ezmlm_make || MaxMailingLists == 0)
		return;
	count_mailinglists();
	load_limits();
	if (MaxMailingLists != -1 && CurMailingLists >= MaxMailingLists) {
		out(html_text[184]);
		strnum[fmt_int(strnum, MaxMailingLists)] = 0;
		out(" ");
		out(strnum);
		out("\n");
		flush();
		show_menu();
		iclose();
		exit(0);
	}
	/*- set up default options for new list */
	default_options();
	if (ezmlm_idx)
		send_template("add_mailinglist-idx.html");
	else
		send_template("add_mailinglist-no-idx.html");

}

void
delmailinglist()
{
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	send_template("del_mailinglist_confirm.html");
}

void
delmailinglistnow()
{
	DIR            *mydir;
	int             len, plen;
	struct dirent  *mydirent;
	static stralloc tmp1 = {0}, tmp2 = {0};

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	if (fixup_local_name(ActionUser.s)) {
		/*- invalid address given, abort */
		iclose();
		exit(0);
	}

	if ((mydir = opendir(".")) == NULL) {
		out(html_text[143]);
		out(" 1<BR>\n");
		out("</table>");
		flush();
		return;
	}
	/*- make dotqmail name */
	if (!stralloc_copy(&dotqmail_name, &ActionUser) || !stralloc_0(&dotqmail_name))
		die_nomem();
	dotqmail_name.len--;
	for (dotnum = 0; dotqmail_name.s[dotnum] != '\0'; dotnum++) {
		if (dotqmail_name.s[dotnum] == '.')
			dotqmail_name.s[dotnum] = ':';
	}
	if (!stralloc_copyb(&tmp1, ".qmail-", 7))
		die_nomem();
	if (!stralloc_cat(&tmp1, &dotqmail_name))
		die_nomem();
	if (!stralloc_copy(&tmp2, &tmp1))
		die_nomem();
	else
	if (!stralloc_append(&tmp2, "-"))
		die_nomem();
	if (!stralloc_0(&tmp1))
		die_nomem();
	if (!stralloc_0(&tmp2))
		die_nomem();
	tmp2.len--;
	while ((mydirent = readdir(mydir)) != NULL) {
		/*- 
		 * delete the main .qmail-"list" file 
		 * delete secondary .qmail-"list"-* files
		 */
		if (!str_diff(tmp1.s, mydirent->d_name) || !str_diffn(tmp2.s, mydirent->d_name, tmp2.len)) {
			if (unlink(mydirent->d_name))
				ack("185", mydirent->d_name);
		}
	}
	closedir(mydir);
	if (!stralloc_copy(&tmp1, &RealDir) ||
			!stralloc_append(&tmp1, "/") ||
			!stralloc_cat(&tmp1, &ActionUser) ||
			!stralloc_0(&tmp1))
		die_nomem();
	vdelfiles(tmp1.s, ActionUser.s, Domain.s);
	count_mailinglists();
	len = str_len(html_text[186]) + ActionUser.len + 28;
	for(plen = 0;;) {
		if (!stralloc_ready(&StatusMessage, len))
			die_nomem();
		plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[186], ActionUser.s);
		if (plen < len) {
			StatusMessage.len = plen;
			break;
		}
		len = plen + 28;
	}
	if (CurMailingLists == 0)
		show_menu();
	else
		show_mailing_lists();
}

/*-
 * sets the Reply-To header in header* files based on form fields
 * designed to be called by ezmlmMake() (after calling ezmlm-make)
 * Replaces the "Reply-To" line in <filename> with <newtext>.
 */
void
ezmlm_setreplyto(char *filename, char *newtext)
{
	int             hfd, tmp_fd, match;
	static stralloc realfn = {0}, tempfn = {0}, line = {0};
	char            inbuf[1024], outbuf[1024];
	struct substdio ssin, ssout;

	if (!stralloc_copy(&realfn, &RealDir) ||
			!stralloc_append(&realfn, "/") ||
			!stralloc_cat(&realfn, &ActionUser) ||
			!stralloc_append(&realfn, "/") ||
			!stralloc_cats(&realfn, filename) ||
			!stralloc_0(&realfn))
		die_nomem();
	realfn.len--;
	if (!stralloc_copy(&tempfn, &realfn) ||
			!stralloc_catb(&tempfn, ".tmp", 4) ||
			!stralloc_0(&tempfn))
		die_nomem();
	if ((tmp_fd = open_trunc(tempfn.s)) == -1)
		return;
	substdio_fdbuf(&ssout, write, tmp_fd, outbuf, sizeof(outbuf));
	if ((hfd = open_read(realfn.s)) != -1) {
		substdio_fdbuf(&ssin, read, hfd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("ezmlm_setreplyto: read: ", realfn.s, ": ", &strerr_sys);
				close(tmp_fd);
				close(hfd);
				unlink(tempfn.s);
				return;
			}
			if (line.len == 0)
				break;
			if (!match) {
				if (!stralloc_append(&line, "\n"))
					die_nomem();
			}
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
			/*- copy contents to new file, except for Reply-To header */
			if (case_diffb(line.s, 8, "Reply-To")) {
				if (substdio_put(&ssout, line.s, line.len)) {
					close(tmp_fd);
					close(hfd);
					unlink(tempfn.s);
					return;
				}
			}
		}
		close(hfd);
	}
	if (substdio_puts(&ssout, newtext) || substdio_put(&ssout, "\n", 1) || substdio_flush(&ssout)) {
		close(tmp_fd);
		unlink(tempfn.s);
		return;
	}
	close(tmp_fd);
	unlink(realfn.s);
	if (rename(tempfn.s, realfn.s) == -1)
		unlink(tempfn.s);
}

#ifdef ENABLE_MYSQL
void
write_mysql(char *mysql_host, char *mysql_socket, char *mysql_user, char *mysql_passwd, char *mysql_database, char *mysql_table)
{
	char            outbuf[1024];
	int             fd;
	struct substdio ssout;

	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/sql", 4) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if (!mysql_host || !mysql_socket || !mysql_user || !mysql_passwd || !mysql_database || !mysql_table) {
		unlink(TmpBuf.s);
		TmpBuf.len -= 3;
		if (!stralloc_catb(&TmpBuf, "subdb", 5) || !stralloc_0(&TmpBuf))
			die_nomem();
		unlink(TmpBuf.s);
		return;
	}
	if ((fd = open_trunc(TmpBuf.s)) == -1)
		return;
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_puts(&ssout, mysql_host) || substdio_put(&ssout, ":", 1) ||
			substdio_puts(&ssout, mysql_socket) || substdio_put(&ssout, ":", 1) ||
			substdio_puts(&ssout, mysql_user) || substdio_put(&ssout, ":", 1) ||
			substdio_puts(&ssout, mysql_passwd) || substdio_put(&ssout, ":", 1) ||
			substdio_puts(&ssout, mysql_database) || substdio_put(&ssout, ":", 1) ||
			substdio_puts(&ssout, mysql_table) || substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout)) {
		close(fd);
		unlink(TmpBuf.s);
		return;
	}
	close(fd);
	return;
}
#endif

void
ezmlmMake(int newlist)
{
	int             pid, len, plen, fd, argc, loop, i, sql_support = 0;
	struct substdio ssout;
	static stralloc list_owner = {0}, owneremail = {0};
	static stralloc loop_ch = {0}, tmp1 = {0}, tmp2 = {0}, tmp3 = {0};
	char            options[128], outbuf[1024];
	char           *t, *s;
	char           *mysql_host = 0, *mysql_user = 0, *mysql_passwd = 0,
				   *mysql_socket = 0, *mysql_database = 0, *mysql_table = 0;
	char           *arguments[MAX_BUFF];
	/*-
	 * Initialize listopt to be a string of the characters A-Z, with each one
	 * set to the correct case (e.g., A or a) to match the expected behavior
	 * of not checking any checkboxes.  Leave other letters blank.
	 * NOTE: Leave F out, since we handle it manually.
	 */
	char            listopt[] = "A  D   hIj L N pQRST      ";

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	if (fixup_local_name(ActionUser.s)) {
		len = str_len(html_text[188]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[188], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		addmailinglist();
		iclose();
		exit(0);
	}

	/*- update listopt based on user selections */
	if (!stralloc_ready(&tmp1, FMT_ULONG + 4))
		die_nomem();
	for (loop = 0; loop < 20; loop++) {
		s = tmp1.s;
		s += fmt_strn(s, "opt", 3);
		s += fmt_int(s, loop);
		*s++ = '=';
		*s++ = 0;
		GetValue(TmpCGI, &loop_ch, tmp1.s);
		for (s = loop_ch.s; *s; s++) {
			if ((*s >= 'A') && (*s <= 'Z'))
				listopt[*s - 'A'] = *s;
			else
			if ((*s >= 'a') && (*s <= 'z'))
				listopt[*s - 'a'] = *s;
		}
	}

	/*- don't allow option c, force option e if modifying existing list */
	listopt[2] = ' ';
	listopt[4] = newlist ? ' ' : 'e';
	argc = 0;
	arguments[argc++] = "ezmlm-make";
	if (ezmlm_idx) {
		/*- check the list owner entry */
		GetValue(TmpCGI, &list_owner, "listowner="); /*- Get the listowner -*/
		if (list_owner.len > 0) {
			if (!stralloc_copyb(&owneremail, "-5", 2) ||
					!stralloc_cat(&owneremail, &list_owner) ||
					!stralloc_0(&owneremail))
				die_nomem();
			arguments[argc++] = owneremail.s;
		}
	}

	/*- build the option string */
	s = options;
	arguments[argc++] = s;
	*s++ = '-';
	if (ezmlm_idx) {
		/*- ignore options v-z, but test a-u */
		for (i = 0; i <= ('u' - 'a'); i++) {
			if (listopt[i] != ' ')
				*s++ = listopt[i];
		}
		*s++ = 0; /* add NULL terminator */
		/*- check for sql support */
		GetValue(TmpCGI, &tmp1, "sqlsupport=");
		if (tmp1.len > 0) {
			sql_support = 1;
			arguments[argc++] = s;
			s += fmt_strn(s, tmp1.s, tmp1.len);
			*s++ = 0;
			arguments[argc++] = s;
			for (tmp2.len = 0, loop = 1; loop <= NUM_SQL_OPTIONS; loop++) {
				t = tmp1.s;
				t += fmt_strn(t, "sql", 3);
				t += fmt_int(t, loop);
				*t++ = '=';
				*t++ = 0;
				GetValue(TmpCGI, &loop_ch, tmp1.s);
				if (!stralloc_cat(&tmp2, &loop_ch) || !stralloc_0(&tmp2))
					die_nomem();
				s += fmt_strn(s, loop_ch.s, loop_ch.len);
				s += fmt_strn(s, ":", 1);
			} /*- for (tmp2.len = 0, loop = 1; loop <= NUM_SQL_OPTIONS; loop++) { */
			/*- remove trailing ':' */
			s--;
			*s++ = 0;
		}
	} else {
		/*- non idx list, only allows options A and P */
		*s++ = listopt[0];	/* a or A */
		*s++ = listopt['p' - 'a'];	/* p or P */
		*s++ = 0;	/* add NULL terminator */
	}
	if (sql_support) {
		for (len = 0, loop = 1; loop <= NUM_SQL_OPTIONS; loop++) {
			switch (loop)
			{
			case 1:
				mysql_host = tmp2.s + len;
				len += (str_len(mysql_host) + 1);
				break;
			case 2:
				mysql_socket = tmp2.s + len;
				len += (str_len(mysql_socket) + 1);
				break;
			case 3:
				mysql_user = tmp2.s + len;
				len += (str_len(mysql_user) + 1);
				break;
			case 4:
				mysql_passwd = tmp2.s + len;
				len += (str_len(mysql_passwd) + 1);
				break;
			case 5:
				mysql_database = tmp2.s + len;
				len += (str_len(mysql_database) + 1);
				break;
			case 6:
				mysql_table = tmp2.s + len;
				len += (str_len(mysql_table) + 1);
				break;
			}
		}
	}
#ifdef ENABLE_MYSQL
	write_mysql(mysql_host, mysql_socket, mysql_user, mysql_passwd, mysql_database, mysql_table);
#endif
	/*- make dotqmail name */
	if (!stralloc_copy(&dotqmail_name, &ActionUser) || !stralloc_0(&dotqmail_name))
		die_nomem();
	dotqmail_name.len--;
	for (dotnum = 0; dotqmail_name.s[dotnum] != '\0'; dotnum++) {
		if (dotqmail_name.s[dotnum] == '.')
			dotqmail_name.s[dotnum] = ':';
	}
	pid = fork();
	if (pid == 0) {
		if (!stralloc_copys(&tmp1, EZMLMDIR) ||
				!stralloc_catb(&tmp1, "/ezmlm-make", 11) ||
				!stralloc_0(&tmp1))
			die_nomem();
		if (!stralloc_copy(&tmp2, &RealDir) ||
				!stralloc_append(&tmp2, "/") ||
				!stralloc_cat(&tmp2, &ActionUser) ||
				!stralloc_0(&tmp2))
			die_nomem();
		if (!stralloc_copy(&tmp3, &RealDir) ||
				!stralloc_catb(&tmp3, "/.qmail-", 8) ||
				!stralloc_cat(&tmp3, &dotqmail_name) ||
				!stralloc_0(&tmp3))
			die_nomem();
		arguments[argc++] = tmp2.s;
		arguments[argc++] = tmp3.s;
		arguments[argc++] = ActionUser.s;
		arguments[argc++] = Domain.s;
		arguments[argc] = (char *) NULL;
		execv(tmp1.s, arguments);
		exit(127);
	} else
		wait(&pid);
	/*- 
	 * Check for prefix setting 
	 * the value here gets displayed by
	 * calling get_mailinglist_prefix()
	 * in template.c
	 */
	GetValue(TmpCGI, &tmp1, "prefix=");

	/*- strip leading '[' and trailing ']' from tmp */
	i = str_chr(tmp1.s, ']');
	if (tmp1.s[i])
		tmp1.s[i] = 0;
	s = tmp1.s;
	while (*s == '[')
		s++;
	/*- Create (or delete) the file as appropriate */
	if (!stralloc_copy(&tmp2, &RealDir) ||
			!stralloc_append(&tmp2, "/") ||
			!stralloc_cat(&tmp2, &ActionUser) ||
			!stralloc_catb(&tmp2, "/prefix", 7) ||
			!stralloc_0(&tmp2))
		die_nomem();
	if (str_len(s) > 0) {
		if ((fd = open_trunc(tmp2.s)) == -1)
			return;
		substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
		if (substdio_put(&ssout, "[", 1) ||
				substdio_puts(&ssout, s) ||
				substdio_put(&ssout, "]", 1) ||
				substdio_flush(&ssout))
		{
			close(fd);
			return;
		}
		close(fd);
	} else
		unlink(tmp2.s);
	/*- set Reply-To header */
	GetValue(TmpCGI, &tmp1, "replyto=");
	scan_int(tmp1.s, &replyto);
	if (replyto == REPLYTO_SENDER) {
		/*- ezmlm shouldn't remove/add Reply-To header */
		ezmlm_setreplyto("headeradd", "");
		ezmlm_setreplyto("headerremove", "");
	} else {
		if (replyto == REPLYTO_ADDRESS) {
			GetValue(TmpCGI, &replyto_addr, "replyaddr=");
			if (!stralloc_copyb(&tmp1, "Reply-To: ", 10) ||
					!stralloc_cat(&tmp1, &replyto_addr) ||
					!stralloc_0(&tmp1))
				die_nomem();
		} else {/* REPLYTO_LIST */
			if (!stralloc_copyb(&tmp1, "Reply-To: <#l#>@<#h#>", 21) ||
					!stralloc_0(&tmp1))
				die_nomem();
		}
		ezmlm_setreplyto("headeradd", tmp1.s);
		ezmlm_setreplyto("headerremove", "Reply-To");
	}
	/*- update inlocal file */
	if (!stralloc_copy(&tmp1, &RealDir) ||
			!stralloc_append(&tmp1, "/") ||
			!stralloc_cat(&tmp1, &ActionUser) ||
			!stralloc_catb(&tmp1, "/inlocal", 8) ||
			!stralloc_0(&tmp1))
		die_nomem();
	if ((fd = open_trunc(tmp1.s)) == -1) {
		strerr_warn3("ezmlmMake: open_trunc: ", tmp1.s,  ": ", &strerr_sys);
		return;
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (substdio_put(&ssout, Domain.s, Domain.len) ||
			substdio_put(&ssout, "-", 1) ||
			substdio_put(&ssout, ActionUser.s, ActionUser.len) ||
			substdio_flush(&ssout))
	{
		close(fd);
		return;

	}
	close(fd);
	if (ezmlm_idx) {
		/*- if this is a new list, add owner as subscriber */
		if (newlist && list_owner.len) {
			ezmlm_sub(GROUP_SUBSCRIBER, list_owner.s);
			if (listopt['M' - 'A'] == 'm') {	/* moderation on */
				/*- add owner as moderator/remote admin as well */
				ezmlm_sub(GROUP_MODERATOR, list_owner.s);
			}
		}
	}
}

void
addmailinglistnow()
{
	char            strnum[FMT_ULONG];
	int             len, plen;

	if (!ezmlm_make || MaxMailingLists == 0)
		return;
	count_mailinglists();
	load_limits();
	if (MaxMailingLists != -1 && CurMailingLists >= MaxMailingLists) {
		strnum[fmt_int(strnum, MaxMailingLists)] = 0;
		out(html_text[184]);
		out(" ");
		out(strnum);
		out("\n");
		flush();
		show_menu();
		iclose();
		exit(0);
	}
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
		addmailinglist();
		iclose();
		exit(0);
	}
	ezmlmMake(1);
	len = str_len(html_text[187]) + ActionUser.len + Domain.len + 28;
	for (plen = 0;;) {
		if (!stralloc_ready(&StatusMessage, len))
			die_nomem();
		plen = snprinth(StatusMessage.s, len, "%s %H@%H\n", html_text[187], ActionUser.s, Domain.s);
		if (plen < len) {
			StatusMessage.len = plen;
			break;
		}
		len = plen + 28;
	}
	show_mailing_lists();
}

/*-
 * mod = 0 for subscribers, 1 for moderators, 2 for digest users 
 */
void
show_list_group_now(int mod)
{
	int             handles[2], pid, z = 0, subuser_count = 0, match;
	static stralloc tmp1 = {0}, tmp2 = {0}, tmp3 = {0}, line = {0};
	char            inbuf[1024], strnum[FMT_ULONG];
	char           *addr;
	struct substdio ssin;

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}

	lowerit(ActionUser.s);
	if (pipe(handles)) {
		iclose();
		exit(0);
	}

	pid = fork();
	if (pid == 0) {
		close(handles[0]);
		dup2(handles[1], 1);
		if (!stralloc_copys(&tmp1, EZMLMDIR) ||
				!stralloc_catb(&tmp1, "/ezmlm-list", 11) ||
				!stralloc_0(&tmp1))
			die_nomem();
		if (!stralloc_copy(&tmp2, &RealDir) ||
				!stralloc_append(&tmp2, "/") ||
				!stralloc_cat(&tmp2, &ActionUser) ||
				!stralloc_0(&tmp2))
			die_nomem();
		if (mod == GROUP_MODERATOR)
			execl(tmp1.s, "ezmlm-list", tmp2.s, "mod", (char *) 0);
		else
		if (mod == GROUP_DIGEST)
			execl(tmp1.s, "ezmlm-list", tmp2.s, "digest", (char *) 0);
		else
			execl(tmp1.s, "ezmlm-list", tmp2.s, (char *) 0);
		exit(127);
	} else {
		close(handles[1]);
		substdio_fdbuf(&ssin, read, handles[0], inbuf, sizeof(inbuf));
		sort_init();
		/*- Load subscriber/moderator list */
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn1("show_list_group_now: read: ", &strerr_sys);
				close(handles[0]);
				return;
			}
			if (line.len == 0)
				break;
			if (!match) {
				if (!stralloc_append(&line, "\n"))
					die_nomem();
			}
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
			sort_add_entry(line.s, '\n');
			subuser_count++;
		}
		close(handles[0]);
		sort_dosort();
		/*- Display subscriber/moderator/digest list, along with delete button */
		if (!stralloc_ready(&tmp1, 5) ||
				!stralloc_ready(&tmp2, 5) ||
				!stralloc_ready(&tmp3, 5))
			die_nomem();
		if (mod == 1) {
			str_copyb(tmp1.s, "228", 4); /*- Total Moderators: */
			str_copyb(tmp2.s, "220", 4); /*- Moderator Address */
		} else
		if (mod == 2) {
			str_copyb(tmp1.s, "244", 4); /*- Total Digest Subscribers: */
			str_copyb(tmp2.s, "246", 4); /*- Digest Subscriber Address */
		} else {
			str_copyb(tmp1.s, "230", 4); /*- Total subscribers */
			str_copyb(tmp2.s, "222", 4); /*- Subscriber address */
		}
		str_copyb(tmp3.s, "072", 4); /*- delete */
		out("<TABLE border=0 width=\"100%\">\n");
		out(" <TR>\n");
		out("  <TH align=left COLSPAN=4><B>");
		out(get_html_text(tmp1.s));
		out("</B> ");
		strnum[fmt_int(strnum, subuser_count)] = 0;
		out(strnum);
		out("<BR><BR></TH>\n");
		out(" </TR>\n");
		out(" <TR align=center bgcolor=");
		out(get_color_text("002"));
		out(">\n");
		out("  <TH align=center><b><font size=2>");
		out(get_html_text(tmp3.s));
		out("</font></b></TH>\n");
		out("  <TH align=center><b><font size=2>");
		out(get_html_text(tmp2.s));
		out("</font></b></TH>\n");
		out("  <TH align=center><b><font size=2>");
		out(get_html_text(tmp3.s));
		out("</font></b></TH>\n");
		out("  <TH align=center><b><font size=2>");
		out(get_html_text(tmp2.s));
		out("</font></b></TH>\n");
		out(" </TR>\n");
		if (!stralloc_ready(&tmp1, 16))
			die_nomem();
		if (mod == 1)
			str_copy(tmp1.s, "dellistmodnow");
		else
		if (mod == 2)
			str_copy(tmp1.s, "dellistdignow");
		else
			str_copy(tmp1.s, "dellistusernow");
		for (z = 0; (addr = (char *) sort_get_entry(z)); ++z) {
			out(" <TR align=center>");
			printh("  <TD align=right><A href=\"%s&modu=%C&newu=%C\"><IMG src=\"%s/trash.png\" border=0></A></TD>\n",
				   cgiurl(tmp1.s), ActionUser.s, addr, IMAGEURL);
			printh("  <TD align=left>%H</TD>\n", addr);
			++z;
			if ((addr = (char *) sort_get_entry(z))) {
				printh("  <TD align=right><A href=\"%s&modu=%C&newu=%C\"><IMG src=\"%s/trash.png\" border=0></A></TD>\n",
					   cgiurl(tmp1.s), ActionUser.s, addr, IMAGEURL);
				printh("  <TD align=left>%H</TD>\n", addr);
			} else {
				out("  <TD COLSPAN=2> </TD>");
			}
			out(" </TR>");
		}
		sort_cleanup();
		out("</TABLE>");
		flush();
		wait(&pid);
		copy_status_mesg(html_text[190]);
	}
}

void
show_list_group(char *template)
{
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	if (!ezmlm_make || MaxMailingLists == 0)
		return;
	send_template(template);
}

void
addlistgroup(char *template)
{
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	send_template(template);
}

void
addlistuser()
{
	addlistgroup("add_listuser.html");
}

void
addlistmod()
{
	addlistgroup("add_listmod.html");
}

void
addlistdig()
{
	addlistgroup("add_listdig.html");
}

/*-
 * returns 0 for success 
 */
int
ezmlm_sub(int mod, char *email)
{
	int             pid;
	static stralloc subpath = {0}, listpath = {0};

	pid = fork();
	if (pid == 0) {
		if (!stralloc_copys(&subpath, EZMLMDIR) ||
				!stralloc_catb(&subpath, "/ezmlm-sub", 10) ||
				!stralloc_0(&subpath))
			die_nomem();
		if (!stralloc_copy(&listpath, &RealDir) ||
				!stralloc_append(&listpath, "/") ||
				!stralloc_cat(&listpath, &ActionUser) ||
				!stralloc_0(&listpath))
			die_nomem();
		if (mod == GROUP_MODERATOR)
			execl(subpath.s, "ezmlm-sub", listpath.s, "mod", email, (char *) NULL);
		else if (mod == GROUP_DIGEST)
			execl(subpath.s, "ezmlm-sub", listpath.s, "digest", email, (char *) NULL);
		else
			execl(subpath.s, "ezmlm-sub", listpath.s, email, (char *) NULL);
		exit(127);
	} else
		wait(&pid);
	/*- need to check exit code for failure somehow */
	return 0;
}

/*- mod = 0 for subscribers, 1 for moderators, 2 for digest subscribers */
void
addlistgroupnow(int mod)
{
	int             len, plen;

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	lowerit(ActionUser.s);
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
		if (mod == GROUP_MODERATOR)
			addlistmod();
		else if (mod == GROUP_DIGEST)
			addlistdig();
		else
			addlistuser();
		iclose();
		exit(0);
	}

	ezmlm_sub(mod, Newu.s);
	if (mod == GROUP_MODERATOR) {
		len = str_len(html_text[194]) + Newu.len + ActionUser.len + Domain.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%H %s %H@%H\n", Newu.s, html_text[194], ActionUser.s, Domain.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		send_template("add_listmod.html");
	} else if (mod == GROUP_DIGEST) {
		len = str_len(html_text[240]) + Newu.len + ActionUser.len + Domain.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%H %s %H@%H\n", Newu.s, html_text[240], ActionUser.s, Domain.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		send_template("add_listdig.html");
	} else {
		len = str_len(html_text[193]) + Newu.len + ActionUser.len + Domain.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%H %s %H@%H\n", Newu.s, html_text[193], ActionUser.s, Domain.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		send_template("add_listuser.html");
	}
	iclose();
	exit(0);
}

void
dellistgroup(char *template)
{
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	send_template(template);
}

void
dellistgroupnow(int mod)
{
	int             pid, i, len, plen;
	static stralloc tmp1 = {0}, tmp2 = {0};

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	lowerit(Newu.s);
	/*-
	 * for dealing with AOL spam complaints, if address doesn't contain @,
	 * but does contain '=', change the '=' to '@'.
	 */
	i = str_chr (Newu.s, '@');
	if (!Newu.s[i]) {
		i = str_chr(Newu.s, '=');
		if (Newu.s[i])
			Newu.s[i] = '@';
	}
	pid = fork();
	if (pid == 0) {
		if (!stralloc_copys(&tmp1, EZMLMDIR) ||
				!stralloc_catb(&tmp1, "/ezmlm-unsub", 12) ||
				!stralloc_0(&tmp1))
			die_nomem();
		if (!stralloc_copy(&tmp2, &RealDir) ||
				!stralloc_append(&tmp2, "/") ||
				!stralloc_cat(&tmp2, &ActionUser) ||
				!stralloc_0(&tmp2))
			die_nomem();
		if (mod == GROUP_MODERATOR)
			execl(tmp1.s, "ezmlm-unsub", tmp2.s, "mod", Newu.s, (char *) 0);
		else
		if (mod == GROUP_DIGEST)
			execl(tmp1.s, "ezmlm-unsub", tmp2.s, "digest", Newu.s, (char *) 0);
		else
			execl(tmp1.s, "ezmlm-unsub", tmp2.s, Newu.s, (char *) 0);
		exit(127);
	} else
		wait(&pid);

	if (mod == GROUP_MODERATOR) {
		len = str_len(html_text[197]) + Newu.len + ActionUser.len + Domain.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%H %s %H@%H\n", Newu.s, html_text[197], ActionUser.s, Domain.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else
	if (mod == GROUP_DIGEST) {
		len = str_len(html_text[242]) + Newu.len + ActionUser.len + Domain.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%H %s %H@%H\n", Newu.s, html_text[242], ActionUser.s, Domain.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else {
		len = str_len(html_text[203]) + Newu.len + ActionUser.len + Domain.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%H %s %H@%H\n", Newu.s, html_text[203], ActionUser.s, Domain.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	}
	show_mailing_lists();
	iclose();
	exit(0);
}

void
count_mailinglists()
{
	DIR            *mydir;
	struct dirent  *mydirent;
	int             fd, match;
	static stralloc line = {0};
	char            inbuf[1024];
	struct substdio ssin;

	if ((mydir = opendir(".")) == NULL) {
		out(html_text[143]);
		out(" 1<BR>\n");
		out("</table>");
		flush();
		return;
	}
	CurMailingLists = 0;
	while ((mydirent = readdir(mydir))) {
		if (!str_diffn(".qmail-", mydirent->d_name, 7)) {
			if ((fd = open_read(mydirent->d_name)) == -1) {
				strerr_warn3("count_mailinglists: open: ", mydirent->d_name, ": ", &strerr_sys);
				out(html_text[144]);
				out(" ");
				out(mydirent->d_name);
				out(" 1<BR>\n");
				continue;
			}
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn1("count_mailinglists: read: ", &strerr_sys);
				out(html_text[144]);
				out(" ");
				out(mydirent->d_name);
				out(" 1<BR>\n");
				flush();
				return;
			}
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
			if (line.len && str_str(line.s, "ezmlm-reject"))
				++CurMailingLists;
			close(fd);
		}
	}
	closedir(mydir);
	flush();
}

/*-
 * name of list to modify is stored in ActionUser 
 */
void
modmailinglist()
{
	int             fd, match;
	static stralloc line = {0};
	char            inbuf[1024];
	struct substdio ssin;

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	Alias.len = 0;
	/*- get the current listowner and copy it to Alias */
	if (!stralloc_copy(&dotqmail_name, &ActionUser) || !stralloc_0(&dotqmail_name))
		die_nomem();
	dotqmail_name.len--;
	str_replace(dotqmail_name.s, '.', ':');
	if (!stralloc_copyb(&TmpBuf, ".qmail-", 7) ||
			!stralloc_cat(&TmpBuf, &dotqmail_name) ||
			!stralloc_catb(&TmpBuf, "-owner", 6) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("modmailinglist: read: ", TmpBuf.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" ");
			out(TmpBuf.s);
			out(" 1<BR>\n");
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
		if (line.len && str_str(line.s, "@")) {
			if (!stralloc_copyb(&Alias, line.s[0] == '&' ? (line.s + 1) : line.s, line.s[0] == '&' ? (line.len - 1) : line.len) ||
					!stralloc_0(&Alias))
				die_nomem();
			Alias.len--;
		}
		close(fd);
	}
	/*- set default to "replies go to original sender" */
	replyto = REPLYTO_SENDER;	/* default */
	replyto_addr.len = 0;
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/headeradd", 10) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	/*- get the Reply-To setting for the list */
	if ((fd = open_read(TmpBuf.s)) != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("modmailinglist: read: ", TmpBuf.s, ": ", &strerr_sys);
				out(html_text[144]);
				out(" ");
				out(TmpBuf.s);
				out(" 1<BR>\n");
				flush();
				return;
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
			if (!case_diffb("Reply-To: ", 10, line.s)) {
				if (!str_diff("<#l#>@<#h#>", line.s + 10)) {
					replyto = REPLYTO_LIST;
				} else {
					replyto = REPLYTO_ADDRESS;
					if (!stralloc_copyb(&replyto_addr, line.s + 10, line.len - 10) ||
							!stralloc_0(&replyto_addr))
						die_nomem();
					replyto_addr.len--;
				}
			}
		}
		close(fd);
	}
	/*- read in options for the current list */
	set_options();
	if (ezmlm_idx)
		send_template("mod_mailinglist-idx.html");
	else
		send_template("show_mailinglists.html");

}

void
modmailinglistnow()
{
	int             len, plen;

	ezmlmMake(0);
	len = str_len(html_text[226]) + ActionUser.len + Domain.len + 28;
	for (plen = 0;;) {
		if (!stralloc_ready(&StatusMessage, len))
			die_nomem();
		plen = snprinth(StatusMessage.s, len, "%s %H@%H\n", html_text[226], ActionUser.s, Domain.s);
		if (plen < len) {
			StatusMessage.len = plen;
			break;
		}
		len = plen + 28;
	}
	show_mailing_lists();
}

void
build_list_value(char *param, char *color, char *opt1, int desc1, char *opt2, int desc2, int checked)
{
	out("<tr bgcolor=");
	out(get_color_text(color));
	out(">\n");
	out("  <td>\n");
	out("    <input type=radio name=");
	out(param);
	out(" value=");
	out(opt1);
	if (!checked)
		out(" CHECKED");
	out("></td>\n");
	out("  <td>");
	out(html_text[desc1]);
	out("</td>\n");
	out("  <td>\n");
	out("    <input type=radio name=");
	out(param);
	out(" value=");
	out(opt2);
	if (checked)
		out(" CHECKED");
	out("></td>\n");
	out("  <td>");
	out(html_text[desc2]);
	out("</td>\n");
	out("</tr>\n");
	flush();
}

void
build_option_str(char *type, char *param, char *options, char *description)
{
	int             selected;
	char           *optptr;

	selected = 1;
	for (optptr = options; *optptr; optptr++) {
		selected = selected && checkopt[(int) *optptr];
	}
	/*- selected is now true if all options for this radio button are true */
	printh("<INPUT TYPE=%s NAME=\"%H\" VALUE=\"%H\"%s> %s\n", type, param, options, selected ? " CHECKED" : "", description);
}

int
file_exists(char *filename)
{
	return (!access(filename, F_OK));
}

int
get_ezmlmidx_line_arguments(char *line, char *program, char argument)
{
	char           *begin = 0, *end = 0, *arg = 0;
	int             i;

	/*- does line contain program name? */
	if (str_str(line, program)) {
		/*- find the options */
		i = str_chr(line, ' ');
		if (line[i])
			begin = line + i + 1;
		if (*begin == '-') {
			i = str_chr(begin, ' ');
			if (begin[i]) {
				end = begin + i;
			}
			i = str_chr(begin, argument);
			if (begin[i])
				arg = begin + i;
			/*- if arg is found && it's in the options (before the trailing space), return 1 */
			if (arg && (arg < end))
				return 1;
		}
	}
	return 0;
}

void
set_options()
{
	char            c;
	int             fd, match;
	static stralloc line = {0};
	char            inbuf[1024];
	struct substdio ssin;

	/*-
	 * Note that with ezmlm-idx it might be possible to replace most
	 * of this code by reading the config file in the list's directory.
	 */

	/*- make dotqmail name (ActionUser with '.' replaced by ':') */
	if (!stralloc_copy(&dotqmail_name, &ActionUser) ||
			!stralloc_0(&dotqmail_name))
		die_nomem();
	dotqmail_name.len--;
	for (dotnum = 0; dotqmail_name.s[dotnum]; dotnum++) {
		if (dotqmail_name.s[dotnum] == '.')
			dotqmail_name.s[dotnum] = ':';
	}
	/*- default to false for lowercase letters */
	for (c = 'a'; c <= 'z'; checkopt[(int) c++] = 0);

	/*- ------ newer configuration reads -*/

	/*- -s: Subscription moderation. touching dir/modsub */
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/modsub", 7) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['s'] = file_exists(TmpBuf.s);
	/*- -h: Help  subscription. Don't require confirmation. Not recommented! -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/nosubconfirm", 13) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['h'] = file_exists(TmpBuf.s);
	/*- -j Jump off. Unsubscribe  does not require confirmation. -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/nounsubconfirm", 15) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['j'] = file_exists(TmpBuf.s);

	/*- -m: Message  moderation. touch dir/modpost -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/modpost", 8) || /*- valid for newer ezmlm-versions -*/
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['m'] = file_exists(TmpBuf.s);
	/*- -o: Reject others than; applicable to message moderated lists only -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/modpostonly", 12) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['o'] = file_exists(TmpBuf.s);
	/*- -u: User posts only. subscribers, digest-subscribers and dir/allow -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/subpostonly", 12) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['u'] = file_exists(TmpBuf.s);

	/*- -f: Subject Prefix. outgoing subject will be pre-fixed with the list name -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/prefix", 7) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['f'] = file_exists(TmpBuf.s);
	/*- -t: Message Trailer. create dir/text/trailer -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/addtrailer", 11) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['t'] = file_exists(TmpBuf.s);

	/*- -a: Archived: touch dir/archived and dir/indexed -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/achived", 8) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['a'] = file_exists(TmpBuf.s);
	/*- -i: indexed for WWW archive access -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/threaded", 9) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['i'] = file_exists(TmpBuf.s);
	/*- -p: Public archive. touch dir/public -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/public", 7) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['p'] = file_exists(TmpBuf.s);
	/*- -g: Guard archive. Access requests from unrecognized SENDERs will be rejected. -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/subgetonly", 11) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['g'] = file_exists(TmpBuf.s);
	/*- -b: Block archive. Only moderators are allowed to access the archive. -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/modgetonly", 11) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['b'] = file_exists(TmpBuf.s);

	/*- -d: Digest -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/digested", 9) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['d'] = file_exists(TmpBuf.s);

	/*- -r: Remote admin. touching dir/remote -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/remote", 7) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['r'] = file_exists(TmpBuf.s);
	/*- -l List subscribers. administrators can request a subscriber -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/modcanlist", 11) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['l'] = file_exists(TmpBuf.s);
	/*- -n New text file. administrators may edit texts -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/modcanedit", 11) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	checkopt['n'] = file_exists(TmpBuf.s);

	/*- ------ end of newer configuration reads -*/

	/*- ------ read in old ezmlm's values -*/
	/*- figure out some options in the -default file; -*/
	if (!stralloc_copyb(&TmpBuf, ".qmail-", 7) ||
			!stralloc_cat(&TmpBuf, &dotqmail_name) ||
			!stralloc_catb(&TmpBuf, "-default", 8) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("set_options: read: ", TmpBuf.s, ": ", &strerr_sys);
				out(html_text[144]);
				out(" ");
				out(TmpBuf.s);
				out(" 1<BR>\n");
				flush();
				return;
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
			/*- -b: Block archive. Only moderators are allowed to access the archive. -*/
			if ((get_ezmlmidx_line_arguments(line.s, "ezmlm-get", 'P')) > 0)
				checkopt['b'] = 1;
			/*- -g: Guard archive. Access requests from unrecognized SENDERs will be rejected. -*/
			if ((get_ezmlmidx_line_arguments(line.s, "ezmlm-get", 's')) > 0)
				checkopt['g'] = 1;
			/*- -h: Help  subscription. Don't require confirmation. Not recommented! -*/
			if ((get_ezmlmidx_line_arguments(line.s, "ezmlm-manage", 'S')) > 0)
				checkopt['h'] = 1;
			/*- -j Jump off. Unsubscribe  does not require confirmation. -*/
			if ((get_ezmlmidx_line_arguments(line.s, "ezmlm-manage", 'U')) > 0)
				checkopt['j'] = 1;
			/*- -l List subscribers. administrators can request a subscriber -*/
			if ((get_ezmlmidx_line_arguments(line.s, "ezmlm-manage", 'l')) > 0)
				checkopt['l'] = 1;
			/*- -n New text file. administrators may edit texts -*/
			if ((get_ezmlmidx_line_arguments(line.s, "ezmlm-manage", 'e')) > 0)
				checkopt['n'] = 1;
		}
		close(fd);
	}
	/*- figure out some options in the qmail file -*/
	if (!stralloc_copyb(&TmpBuf, ".qmail-", 7) ||
			!stralloc_cat(&TmpBuf, &dotqmail_name) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("set_options: read: ", TmpBuf.s, ": ", &strerr_sys);
				out(html_text[144]);
				out(" ");
				out(TmpBuf.s);
				out(" 1<BR>\n");
				flush();
				return;
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
			if (((get_ezmlmidx_line_arguments(line.s, "ezmlm-store", 'P')) > 0))
				checkopt['o'] = 1;
		}
		close(fd);
	}
	/*- -t: Message Trailer. create dir/text/trailer -*/
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/text/trailer", 13) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (file_exists(TmpBuf.s))
		checkopt['t'] = 1;
	/*- ------ end of read in old ezmlm's values -*/
	/*- update the uppercase option letters (just the opposite of the lowercase) */
	for (c = 'A'; c <= 'Z'; c++) {
		checkopt[(int) c] = !checkopt[(int) (c - 'A' + 'a')];
	}
}

void
default_options()
{
	char            c;

	dotqmail_name.len = 0;
	replyto = REPLYTO_SENDER;
	replyto_addr.len = 0;

	/*-
	 * These are currently set to defaults for a good, generic list.
	 * Basically, make it safe/friendly and don't turn anything extra on.
	 */
	/*- for the options below, use 1 for "on" or "yes" */
	checkopt['a'] = 1;			/* Archive */
	checkopt['b'] = 1;			/* Moderator-only access to archive */
	checkopt['c'] = 0;			/* ignored */
	checkopt['d'] = 0;			/* Digest */
	checkopt['e'] = 0;			/* ignored */
	checkopt['f'] = 1;			/* Prefix */
	checkopt['g'] = 1;			/* Guard Archive */
	checkopt['h'] = 0;			/* Subscribe doesn't require conf */
	checkopt['i'] = 0;			/* Indexed */
	checkopt['j'] = 0;			/* Unsubscribe doesn't require conf */
	checkopt['k'] = 0;			/* Create a blocked sender list */
	checkopt['l'] = 0;			/* Remote admins can access subscriber list */
	checkopt['m'] = 0;			/* Moderated */
	checkopt['n'] = 0;			/* Remote admins can edit text files */
	checkopt['o'] = 0;			/* Others rejected (for Moderated lists only */
	checkopt['p'] = 1;			/* Public */
	checkopt['q'] = 1;			/* Service listname-request, no longer supported */
	checkopt['r'] = 0;			/* Remote Administration */
	checkopt['s'] = 0;			/* Subscriptions are moderated */
	checkopt['t'] = 0;			/* Add Trailer to outgoing messages */
	checkopt['u'] = 1;			/* Only subscribers can post */
	checkopt['v'] = 0;			/* ignored */
	checkopt['w'] = 0;			/* special ezmlm-warn handling (ignored) */
	checkopt['x'] = 0;			/* enable some extras (ignored) */
	checkopt['y'] = 0;			/* ignored */
	checkopt['z'] = 0;			/* ignored */
	/*- update the uppercase option letters (just the opposite of the lowercase) */
	for (c = 'A'; c <= 'Z'; c++) {
		checkopt[(int) c] = !checkopt[(int) (c - 'A' + 'a')];
	}
}

void
show_current_list_values()
{
	int             i, fd, ok, checked, len, plen, match, loop;
	static stralloc listname = {0}, line1 = {0}, line2 = {0};
	char            strnum[FMT_ULONG];
	char           *ptr, *cptr;
	char           *mysql_host = 0, *mysql_user = 0, *mysql_passwd = 0,
				   *mysql_socket = 0;
	char            inbuf[1024];
	struct substdio ssin;

	/*-
	 * Note that we do not support the following list options:
	 * k - posts from addresses in listname/deny are rejected
	 * x - strip annoying MIME parts (spreadsheets, rtf, html, etc.)
	 * 0 - make the list a sublist of another list
	 * 3 - replace the From: header with another address
	 * 4 - digest options (limits related to sending digest out)
	 * 7, 8, 9 - break moderators up into message, subscription and admin
	 */

	/*-
	 * IMPORTANT: If you change the behavior of the checkboxes, you need
	 * to update the default settings in modmailinglistnow so iwebadmin
	 * will use the proper settings when a checkbox isn't checked.
	 */

	if (dotqmail_name.len) { /* modifying an existing list */
		len = dotqmail_name.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&listname, len))
				die_nomem();
			plen = snprinth(listname.s, len, "%H", dotqmail_name.s);
			if (plen < len) {
				listname.len = plen;
				break;
			}
			len = plen + 28;
		}
		str_replace(listname.s, ':', '.');
	} else {
		if (!stralloc_copyb(&listname, "<I>", 3) ||
				!stralloc_cats(&listname, html_text[261]) ||
				!stralloc_catb(&listname, "</I>", 4) ||
				!stralloc_0(&listname))
			die_nomem();
	}
	/*- Posting Messages */
	out("<P><B><U>");
	out(html_text[262]);
	out("</U></B><BR>\n");
	build_option_str("RADIO", "opt1", "MU", html_text[263]);
	out("<BR>\n");
	build_option_str("RADIO", "opt1", "Mu", html_text[264]);
	out("<BR>\n");
	build_option_str("RADIO", "opt1", "mu", html_text[265]);
	out("<BR>\n");
	build_option_str("RADIO", "opt1", "mUo", html_text[266]);
	out("<BR>\n");
	build_option_str("RADIO", "opt1", "mUO", html_text[267]);
	out("</P>\n");

	/*- List Options */
	out("<P><B><U>");
	out(html_text[268]);
	out("</U></B><BR>\n");
	out("<TABLE><TR><TD ROWSPAN=3 VALIGN=TOP>");
	out(html_text[310]);
	out("</TD>");
	out("<TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"");
	strnum[fmt_int(strnum, REPLYTO_SENDER)] = 0;
	out(strnum);
	out("\"");
	if (replyto == REPLYTO_SENDER)
		out(" CHECKED");
	out(">");
	out(html_text[311]);
	out("</TD></TR>\n");

	out("<TR><TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"");
	strnum[fmt_int(strnum, REPLYTO_LIST)] = 0;
	out(strnum);
	out("\"");
	if (replyto == REPLYTO_LIST)
		out(" CHECKED");
	out(">");
	out(html_text[312]);
	out("</TD></TR>\n");

	out("<TR><TD><INPUT TYPE=RADIO NAME=\"replyto\" VALUE=\"");
	strnum[fmt_int(strnum, REPLYTO_ADDRESS)] = 0;
	out(strnum);
	out("\"");
	if (replyto == REPLYTO_ADDRESS)
		out(" CHECKED");
	out(">");
	out(html_text[313]);
	out(" ");

	printh("<INPUT TYPE=TEXT NAME=\"replyaddr\" VALUE=\"%H\" SIZE=30></TD></TR>\n", replyto_addr.len ? replyto_addr.s : "");
	out("</TABLE><BR>\n");
	build_option_str("CHECKBOX", "opt4", "t", html_text[270]);
	out("<BR>\n");
	build_option_str("CHECKBOX", "opt5", "d", html_text[271]);

	if (!stralloc_copys(&TmpBuf, html_text[272]) ||
			!stralloc_cat(&TmpBuf, &listname) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	out("<SMALL>(");
	out(TmpBuf.s);
	out(")</SMALL>");
	out("</P>");

	/*- Remote Administration */
	out("<P><B><U>");
	out(html_text[275]);
	out("</U></B><BR>\n");
	build_option_str("CHECKBOX", "opt7", "r", html_text[276]);
	out("<BR>\n");
	build_option_str("CHECKBOX", "opt8", "P", html_text[277]);
	out("<SMALL>(");
	out(html_text[278]);
	out(")</SMALL><BR>");
	out("<TABLE><TR><TD ROWSPAN=2 VALIGN=TOP>");
	out(html_text[279]);
	out("</TD>");
	out("<TD>");
	build_option_str("CHECKBOX", "opt9", "l", html_text[280]);
	out("</TD>\n</TR><TR>\n<TD>");
	build_option_str("CHECKBOX", "opt10", "n", html_text[281]);
	out("<SMALL>(");
	out(html_text[282]);
	out(")</SMALL>.</TD>\n");
	out("</TR></TABLE>\n</P>\n");

	out("<P><B><U>");
	out(html_text[283]);
	out("</U></B><BR>\n");
	out(html_text[284]);
	out("<BR>\n&nbsp; &nbsp; ");
	build_option_str("CHECKBOX", "opt11", "H", html_text[285]);
	out("<BR>\n&nbsp; &nbsp; ");
	build_option_str("CHECKBOX", "opt12", "s", html_text[286]);
	out("<BR>\n");
	out(html_text[287]);
	out("<BR>\n&nbsp; &nbsp; ");
	build_option_str("CHECKBOX", "opt13", "J", html_text[285]);
	out("<BR>\n");
	out("<SMALL>");
	out(html_text[288]);
	out("</SMALL>\n</P>\n");

	out("<P><B><U>");
	out(html_text[289]);
	out("</U></B><BR>\n");
	build_option_str("CHECKBOX", "opt14", "a", html_text[290]);
	out(" &nbsp; ");
	out(html_text[292]);
	out("\n<SELECT NAME=\"opt15\">");

	out("<OPTION VALUE=\"BG\"");
	if (checkopt['B'] && checkopt['G'])
		out(" SELECTED");
	out(">");
	out(html_text[293]);
	out("\n");

	out("<OPTION VALUE=\"Bg\"");
	if (checkopt['B'] && checkopt['g'])
		out(" SELECTED");
	out(">");
	out(html_text[294]);
	out("\n");

	out("<OPTION VALUE=\"b\"");
	if (checkopt['b'])
		out(" SELECTED");
	out(">");
	out(html_text[295]);
	out("\n");
	out("</SELECT>.");
	out("<BR>\n");
	/*-
	 * note that if user doesn't have ezmlm-cgi installed, it might be
	 * a good idea to default to having option i off. 
	 */
	build_option_str("CHECKBOX", "opt16", "i", html_text[291]);
	out("</P>\n");

	/*- Begin MySQL options
	 * See if sql is turned on
	 * host:username:password:socket_or_port:ssl_or_nossl
	 */
	checked = 0;
	if (!stralloc_copy(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/sql", 4) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		checked = 1;
		if (getln(&ssin, &line1, &match, '\n') == -1) {
			strerr_warn3("show_current_list_values: read: ", TmpBuf.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" ");
			out(TmpBuf.s);
			out(" 1<BR>\n");
			flush();
			return;
		}
		if (match) {
			line1.len--;
			line1.s[line1.len] = 0;
		} else {
			if (!stralloc_0(&line1))
				die_nomem();
			line1.len--;
		}
		close(fd);
	}
#ifdef ENABLE_MYSQL
	out("<P><B><U>");
	out(html_text[99]); /*- MySQL Settings */
	out("</U></B><BR>\n");
	out("<input type=checkbox name=\"sqlsupport\" value=\"-6\"");
	if (checked)
		out(" CHECKED");
	out("> ");
	out(html_text[53]); /*- Enable MySQL support */
	/*- parse dir/sql file for SQL settings */
	out("    <table cellpadding=0 cellspacing=2 border=0>\n");
#else
	if (checked)
		out("<INPUT TYPE=HIDDEN NAME=sqlsupport VALUE=\"-6\">\n");
#endif
	if (!stralloc_copys(&TmpBuf, SYSCONFDIR) ||
			!stralloc_catb(&TmpBuf, "/control/host.mysql", 23) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line2, &match, '\n') == -1) {
			strerr_warn3("show_current_list_values: read: ", TmpBuf.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" ");
			out(TmpBuf.s);
			out(" 1<BR>\n");
			flush();
			return;
		}
		if (match) {
			line2.len--;
			line2.s[line2.len] = 0;
		} else {
			if (!stralloc_0(&line2))
				die_nomem();
			line2.len--;
		}
		close(fd);
	}
	/*- host:user:password:socket_or_port:ssl_or_nossl */
	for (plen = 0, ok = loop = 1; ok && loop < NUM_SQL_OPTIONS - 1; loop++) {
		switch (loop)
		{
		case 1:
			mysql_host = line2.s + plen;
			i = str_chr(mysql_host, ':');
			if (mysql_host[i])
				mysql_host[i] = 0;
			else
				ok = 0;
			plen += (str_len(mysql_host) + 1);
			break;
		case 2:
			mysql_user = line2.s + plen;
			i = str_chr(mysql_user, ':');
			if (mysql_user[i])
				mysql_user[i] = 0;
			else
				ok = 0;
			plen += (str_len(mysql_user) + 1);
			break;
		case 3:
			mysql_passwd = line2.s + plen;
			i = str_chr(mysql_passwd, ':');
			if (mysql_passwd[i])
				mysql_passwd[i] = 0;
			else
				ok = 0;
			plen += (str_len(mysql_passwd) + 1);
			break;
		case 4:
			mysql_socket = line2.s + plen;
			i = str_chr(mysql_socket, ':');
			if (mysql_socket[i])
				mysql_socket[i] = 0;
			else
				ok = 0;
			plen += (str_len(mysql_socket) + 1);
			break;
		}
	}

	/*- get hostname */
	if (line1.len) {
		ptr = line1.s;
		for (cptr = ptr; *cptr && *cptr != ':';cptr++);
		if (*cptr == ':') {
			ok = 1;
			*cptr = 0;
		} else {
			ok = 0;
			ptr = mysql_host ? mysql_host : "localhost";
		}
	} else {
		ok = 0;
		ptr = mysql_host ? mysql_host : "localhost";
	}
#ifdef ENABLE_MYSQL
	out("      <tr>\n");
	out("        <td ALIGN=RIGHT>");
	out(html_text[54]); /*- Host */
	out(":\n");
	out("          </td><td>\n");
	printh("          <input type=text name=sql1 value=\"%H\"></td>\n", ptr);
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql1 VALUE=\"%H\">\n", ptr);
#endif

	/*- get port */
	if (ok) {
		ptr = cptr + 1;
		for (cptr = ptr; *cptr && *cptr != ':';cptr++);
		if (*cptr == ':') {
			ok = 1;
			*cptr = 0;
		} else {
			ok = 0;
			ptr = mysql_socket ? mysql_socket : "/var/run/mysqld/mysqld.sock";
		}
	} else {
		ok = 0;
		ptr = mysql_socket ? mysql_socket : "/var/run/mysqld/mysqld.sock";
	}
#ifdef ENABLE_MYSQL
	out("        <td ALIGN=RIGHT>");
	out(html_text[55]); /*- Port */
	out(":\n");
	out("          </td><td>\n");
	printh("          <input type=text size=7 name=sql2 value=\"%H\"></td>\n", ptr);
	out("      </tr>\n");
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql2 VALUE=\"%H\">\n", ptr);
#endif

	/*- get user */
	if (ok) {
		ptr = cptr + 1; /*- user */
		for (cptr = ptr; *cptr && *cptr != ':';cptr++);
		if (*cptr == ':') {
			ok = 1;
			*cptr = 0;
		} else {
			ok = 0;
			ptr = mysql_user ? mysql_user : "";
		}
	} else {
		ok = 0;
		ptr = mysql_user ? mysql_user : "";
	}
#ifdef ENABLE_MYSQL
	out("      <tr>\n");
	out("        <td ALIGN=RIGHT>");
	out(html_text[56]); /*- user */
	out(":\n");
	out("          </td><td>\n");
	printh("          <input type=text name=sql3 value=\"%H\"></td>\n", ptr);
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql3 VALUE=\"%H\">\n", ptr);
#endif

	/*- get password */
	if (ok) {
		ptr = cptr + 1; /*- password */
		for (cptr = ptr; *cptr && *cptr != ':';cptr++);
		if (*cptr == ':') {
			ok = 1;
			*cptr = 0;
		} else {
			ok = 0;
			ptr = mysql_passwd ? mysql_passwd : "";
		}
	} else {
		ok = 0;
		ptr = mysql_passwd ? mysql_passwd : "";
	}
#ifdef ENABLE_MYSQL
	out("        <td ALIGN=RIGHT>");
	out(html_text[57]); /*- password */
	out(":\n");
	out("          </td><td>\n");
	printh("          <input type=text name=sql4 value=\"%H\"></td>\n", ptr);
	out("      </tr>\n");
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql4 VALUE=\"%H\">\n", ptr);
#endif

	/*- get database */
	if (ok) {
		ptr = cptr + 1; /*- database */
		for (cptr = ptr; *cptr && *cptr != ':';cptr++);
		if (*cptr == ':') {
			ok = 1;
			*cptr = 0;
		} else {
			ok = 0;
			ptr = "indimail";
		}
	} else {
		ok = 0;
		ptr = "indimail";
	}
#ifdef ENABLE_MYSQL
	out("      <tr>\n");
	out("        <td ALIGN=RIGHT>");
	out(html_text[58]); /*- database */
	out(":\n");
	out("          </td><td>\n");
	printh("          <input type=text name=sql5 value=\"%H\"></td>\n", ptr);
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql5 VALUE=\"%H\">\n", ptr);
#endif

	/*- get tablename */
	if (ok) {
		ptr = cptr + 1; /*- table name */
		for (cptr = ptr; *cptr && *cptr != ':';cptr++);
		if (*cptr == ':') {
			ok = 1;
			*cptr = 0;
		} else {
			ok = 0;
			ptr = "ezmlm";
		}
	} else {
		ok = 0;
		ptr = "ezmlm";
	}
#ifdef ENABLE_MYSQL
	out("        <td ALIGN=RIGHT>");
	out(html_text[59]); /*- table */
	out(":\n");
	out("          </td><td>\n");
	printh("          <input type=text name=\"sql6\" value=\"%H\"></td>\n", ptr);
	out("      </tr>\n");
	out("    </table>\n");
#else
	printh("<INPUT TYPE=HIDDEN NAME=sql6 VALUE=\"%H\">\n", ptr);
#endif
	flush();
}

int
get_mailinglist_prefix(stralloc *prefix)
{
	char           *b, *p;
	int             fd, match, len;
	static stralloc line = {0};
	char            inbuf[1024];
	struct substdio ssin;

	if (!stralloc_copy(&TmpBuf, &RealDir) ||
			!stralloc_append(&TmpBuf, "/") ||
			!stralloc_cat(&TmpBuf, &ActionUser) ||
			!stralloc_catb(&TmpBuf, "/prefix", 7) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("get_mailinglist_prefix: read: ", TmpBuf.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" ");
			out(TmpBuf.s);
			out(" 1<BR>\n");
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
		b = line.s;
		if (!stralloc_ready(prefix, line.len))
			die_nomem();
		p = prefix->s;
		while (*b == '[')
			b++;
		for (len = 0; *b && *b != ']' && *b != '\n'; len++)
			*p++ = *b++;
		*p++ = 0;
		prefix->len = len;
	} else {
		if (!stralloc_ready(prefix, 1))
			die_nomem();
		*prefix->s = 0;
		return 1;
	}
	return 0;
}
