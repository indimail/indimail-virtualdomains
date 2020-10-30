/*
 * $Id: template.c,v 1.14 2020-10-30 22:24:44+05:30 Cprogrammer Exp mbhangui $
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
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <scan.h>
#include <env.h>
#include <subfd.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#include <fmt.h>
#include <strerr.h>
#endif
#include "alias.h"
#include "autorespond.h"
#include "cgi.h"
#include "forward.h"
#include "limits.h"
#include "mailinglist.h"
#include "printh.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "template.h"
#include "user.h"
#include "util.h"
#include "common.h"

static char     dchar[4];
void            check_mailbox_flags(char newchar);
void            transmit_block(substdio *ss);
void            ignore_to_end_tag(substdio *ss);
void            get_calling_host();
char           *get_session_val(char *session_var);

/*
 * send an html template to the browser 
 */
int
send_template(char *actualfile)
{
	send_template_now("header.html");
	send_template_now(actualfile);
	send_template_now("footer.html");
	return 0;
}

int
getch(substdio *ss)
{
	int             r;
	char            buf[1];

	if ((r = substdio_get(ss, buf, 1)) == 1)
		return (buf[0]);
	return (r);
}

int
putch(int ch, substdio *ss)
{
	if (substdio_put(ss, (char *) &ch, 1) == -1)
		return (-1);
	return (1);
}

int
send_template_now(char *filename)
{
	int             i, fd, match, inchar;
	char           *ptr, *alias_line, *qnote = " MB";
	struct stat     mystat;
	char            qconvert[FMT_ULONG], inbuf[1024], strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	struct passwd  *vpw;
	static stralloc value1 = {0}, value2 = {0};
	struct substdio ssin;

	if (str_str(filename, "/") || str_str(filename, "..")) {
		out("warning: invalid file name ");
		out(filename);
		out("\n");
		flush();
		return (-1);
	}
	if (!(ptr = env_get(QMAILADMIN_TEMPLATEDIR)))
		ptr = HTMLLIBDIR;
	if (!stralloc_copys(&TmpBuf, ptr) ||
			!stralloc_catb(&TmpBuf, "/html/", 6) ||
			!stralloc_cats(&TmpBuf, filename) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if (lstat(TmpBuf.s, &mystat) == -1) {
		out("Warning: cannot lstat '");
		out(TmpBuf.s);
		out("', check permissions.<BR>\n");
		flush();
		return (-1);
	}
	if (S_ISLNK(mystat.st_mode)) {
		out("Warning: '");
		out(TmpBuf.s);
		out("' is a symbolic link.<BR>\n");
		flush();
		return (-1);
	}
	/* open the template */
	if ((fd = open_read(TmpBuf.s)) == -1) {
		strerr_warn3("send_template: open: ", TmpBuf.s, ": ", &strerr_sys);
		out(html_text[144]);
		out(" ");
		out(TmpBuf.s);
		out("<br>\n");
		flush();
		return 0;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	/* parse the template looking for "##" pattern */
	for (;;) {
		if ((inchar = getch(&ssin)) == -1) {
			strerr_warn3("send_template: read: ", TmpBuf.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" ");
			out(TmpBuf.s);
			out("<br>\n");
			flush();
			return 0;
		} else
		if (!inchar)
			break;
		/* if not '#' then send it */
		if (inchar != '#') {
			putch(inchar, subfdoutsmall);
		} else { /* found a '#' */
			/* look for a second '#' */
			inchar = getch(&ssin);
			if (!inchar)
				break;
			else /* found a tag */
			if (inchar == '#') {
				inchar = getch(&ssin);
				if (!inchar)
					break;
				/* switch on the tag */
				switch (inchar)
				{
				case '&': /* send stock (user, dom, time) cgi parameters */
					printh("user=%C&dom=%C&time=%d&", Username.s, Domain.s, mytime);
					break;
				case '~':
					out(Lang);
					break;
				case 'A': /* send the action user parameter */
					printh("%H", ActionUser.s);
					break;
				case 'a': /* send the Alias parameter */
					printh("%H", Alias.s);
					break;
				case 'B': /* show number of pop accounts */
					load_limits();
					count_users();
					if (MaxPopAccounts > -1) {
						strnum1[fmt_int(strnum1, CurPopAccounts)] = 0;
						out(strnum1);
						out("/");
						strnum1[fmt_int(strnum1, MaxPopAccounts)] = 0;
					} else {
						strnum1[fmt_int(strnum1, CurPopAccounts)] = 0;
						out(strnum1);
						out("/");
						out(html_text[229]);
					}
					break;
				case 'b': /* show the lines inside a alias table (not used, see ##d) */
					show_dotqmail_lines(Username.s, Domain.s, mytime);
					break;
				case 'C': /* send the CGIPATH parameter */
					out(CGIPATH);
					break;
				case 'c': /* show the lines inside a mailing list table */
					show_mailing_list_line2(Username.s, Domain.s, mytime, RealDir.s);
					break;
				case 'D': /* send the domain parameter */
					if (Domain.len)
						printh("%H", Domain.s);
					break;
				case 'd': /* show the lines inside a forward table */
					show_dotqmail_lines(Username.s, Domain.s, mytime);
					break;
				case 'E': /* this will be used to parse mod_mailinglist-idx.html */
					show_current_list_values();
					break;
				case 'e': /* show the lines inside a mailing list table */
					show_mailing_list_line(Username.s, Domain.s, mytime, RealDir.s);
					break;
				case 'F': /* display a file (used for mod_autorespond ONLY) */
					/* should verify here that alias_line contains "/autorespond " */
					{
					char            inbuf[1024];
					int             fd;
					struct substdio ssin;

					if ((alias_line = valias_select(ActionUser.s, Domain.s))) {
						i = str_rchr(alias_line, '/');
						if (alias_line[i])
							for (ptr = alias_line + i - 1; ptr != alias_line && *ptr != '/'; ptr--);
						if (*ptr == '/')
							printh("value=\"%H@%H\"></td>\n", ptr + 1, Domain.s);
						else
							printh("value=\"%H\"></td>\n", *alias_line == '&' ? alias_line + 1 : alias_line);
					}
					if (!stralloc_copyb(&TmpBuf, "vacation/", 9) ||
							!stralloc_cat(&TmpBuf, &ActionUser) ||
							!stralloc_catb(&TmpBuf, "/.vacation.msg", 14) ||
							!stralloc_0(&TmpBuf))
						die_nomem();
					if ((fd = open_read(TmpBuf.s)) == -1) {
						strnum1[fmt_uint(strnum1, getuid())] = 0;
						strnum2[fmt_uint(strnum2, getgid())] = 0;
						strerr_warn7("send_template_now: ", TmpBuf.s, ": uid=", strnum1, ", gid=", strnum2, ": ", &strerr_sys);
						ack("150", TmpBuf.s);
					}
					substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
					/*- read Reference: and Subjec: line */
					for (i = 0; i < 2; i++) {
						if (getln(&ssin, &line, &match, '\n') == -1) {
							strerr_warn3("send_template_now: ", TmpBuf.s, ": ", &strerr_sys);
							out(html_text[144]);
							out(" ");
							out(TmpBuf.s);
							out(" 1<BR>\n");
							close(fd);
							flush();
							return (1);
						}
						if (!line.len)
							break;
					}
					if (match) {
						line.len--;
						line.s[line.len] = 0;
					} else {
						if (!stralloc_0(&line))
							die_nomem();
						line.len--;
					}
					out("         <td>&nbsp;</td>\n");
					out("         </tr>\n");
					out("         <tr>\n");
					out("         <td align=right><b>");
					out(html_text[6]);
					out("</b></td>\n");
					printh("         <td><input type=\"text\" size=40 name=\"alias\" maxlength=128 value=\"%H\"></td>\n",
						   line.s + 9);
					out("         <td>&nbsp;</td>\n");
					out("        </tr>\n");
					out("       </table>\n");
					out("       <textarea cols=80 rows=40 name=\"message\">");

					/*- Skip custom headers */
					while(1) {
						if (getln(&ssin, &line, &match, '\n') == -1) {
							strerr_warn3("send_template_now: ", TmpBuf.s, ": ", &strerr_sys);
							out(html_text[144]);
							out(" ");
							out(TmpBuf.s);
							out(" 1<BR>\n");
							flush();
							close(fd);
							return (1);
						}
						if (!line.len || line.s[0] == '\r' || line.s[0] == '\n')
							break;
					}
					out(line.s);
					for (;;) {
						if (getln(&ssin, &line, &match, '\n') == -1) {
							strerr_warn3("send_template_now: ", TmpBuf.s, ": ", &strerr_sys);
							out(html_text[144]);
							out(" ");
							out(TmpBuf.s);
							out(" 1<BR>\n");
							flush();
							close(fd);
							return (1);
						}
						if (!line.len)
							break;
						substdio_put(subfdoutsmall, line.s, line.len);
					}
					close(fd);
					out("</textarea>");
					flush();
					}
					break;
				case 'f': /* show the forwards */
					if (AdminType == DOMAIN_ADMIN)
						show_forwards(Username.s, Domain.s, mytime);
					break;
				case 'G': /*- show the mailing list digest subscribers */
					if (AdminType == DOMAIN_ADMIN)
						show_list_group_now(2);
					break;
				case 'g': /* show the lines inside a autorespond table */
					show_autorespond_line(Username.s, Domain.s, mytime, RealDir.s);
					break;
				case 'H': /* show returnhttp (from TmpCGI) */
					GetValue(TmpCGI, &value1, "returnhttp=");
					printh("%C", value1.s);
					break;
				case 'h': /* show the counts */
					show_counts();
					break;
				case 'I':
					show_dotqmail_file(ActionUser.s);
					break;
				case 'i': /* check for user forward and forward/store vacation */
					parse_users_dotqmail(getch(&ssin));
					break;
				case 'J': /* show mailbox flag status */
					check_mailbox_flags(getch(&ssin));
					break;
				case 'j': /* show number of mailing lists */
					load_limits();
					count_mailinglists();
					if (MaxMailingLists > -1) {
						strnum1[fmt_int(strnum1, CurMailingLists)] = 0;
						strnum2[fmt_int(strnum2, MaxMailingLists)] = 0;
						out(strnum1);
						out("/");
						out(strnum1);
					} else {
						strnum1[fmt_int(strnum1, CurMailingLists)] = 0;
						out(strnum1);
						out("/");
						out(html_text[229]);
					}
					break;
				case 'k': /* show number of forwards */
					load_limits();
					count_forwards();
					if (MaxForwards > -1) {
						strnum1[fmt_int(strnum1, CurForwards)] = 0;
						strnum2[fmt_int(strnum2, MaxForwards)] = 0;
						out(strnum1);
						out("/");
						out(strnum1);
					} else {
						strnum1[fmt_int(strnum1, CurForwards)] = 0;
						out(strnum1);
						out("/");
						out(html_text[229]);
					}
					break;
				case 'K': /* show number of autoresponders */
					load_limits();
					count_autoresponders();
					if (MaxAutoResponders > -1) {
						strnum1[fmt_int(strnum1, CurAutoResponders)] = 0;
						strnum2[fmt_int(strnum2, MaxAutoResponders)] = 0;
						out(strnum1);
						out("/");
						out(strnum1);
					} else {
						strnum1[fmt_int(strnum1, CurAutoResponders)] = 0;
						out(strnum1);
						out("/");
						out(html_text[229]);
					}
					break;
				case 'l': /* show the aliases stuff */
					if (AdminType == DOMAIN_ADMIN)
						show_aliases();
					break;
				case 'L': /* login username */
					if (Username.len)
						printh("%H", Username.s);
					else
					if (TmpCGI && GetValue(TmpCGI, &value1, "user=") == 0)
						printh("%H", value1.s);
					else
						out("postmaster");
					break;
				case 'M': /* show the mailing list subscribers */
					if (AdminType == DOMAIN_ADMIN)
						show_list_group_now(0);
					break;
				case 'm': /* show the mailing lists */
					if (AdminType == DOMAIN_ADMIN)
						show_mailing_lists();
					break;
				case 'N': /* parse include files */
					i = getch(&ssin);
					if (i == '/')
						out(html_text[144]);
					else
					if (i > 0) {
						TmpBuf.len = 0;
						for (;;) {
							if (!(i = getch(&ssin)))
								break;
							if (i != '#' && !stralloc_append(&TmpBuf, (char *) &i))
								die_nomem();
						}
						if (!stralloc_0(&TmpBuf))
							die_nomem();
						TmpBuf.len--;
						if ((str_str(TmpBuf.s, "../"))) {
							out(html_text[144]);
							out(" ");
							out(TmpBuf.s);
						} else
						if (str_diff(TmpBuf.s, filename))
							send_template_now(TmpBuf.s);
					}
					break;
				case 'n': /* show returntext (from TmpCGI) */
					GetValue(TmpCGI, &value1, "returntext=");
					printh("%H", value1.s);
					break;
				case 'O': /* build a pulldown menu of all POP/IMAP users */
					{
						struct passwd  *pw;

						pw = sql_getall(Domain.s, 1, 1);
						while (pw) {
							printh("<option value=\"%H\">%H</option>\n", pw->pw_name, pw->pw_name);
							pw = sql_getall(Domain.s, 0, 0);
						}
					}
					break;
				case 'o': /* show the mailing list moderators */
					if (AdminType == DOMAIN_ADMIN)
						show_list_group_now(1);
					break;
				case 'P': /* display mailing list prefix */
					get_mailinglist_prefix(&TmpBuf);
					printh("%H", TmpBuf.s);
					break;
				case 'p': /* show POP/IMAP users */
					show_user_lines(Username.s, Domain.s, mytime, RealDir.s);
					break;
				case 'Q': /* show quota usage */
					vpw = sql_getpw(ActionUser.s, Domain.s);
					if (str_diffn(vpw->pw_shell, "NOQUOTA", 8)) {
						mdir_t          diskquota = 0;
						mdir_t          maxmsg = 0;

						quota_to_megabytes(qconvert, vpw->pw_shell);
						if (!stralloc_copys(&TmpBuf, vpw->pw_dir) ||
								!stralloc_0(&TmpBuf))
							die_nomem();
						diskquota = check_quota(TmpBuf.s, &maxmsg);
						strnum1[fmt_double(strnum1, diskquota/1048576.0, 2)] = 0; /* Convert to MB */
						out(strnum1);
						out(" /");
					}
					break;
				case 'q': /* display user's quota (mod user page) */
					vpw = sql_getpw(ActionUser.s, Domain.s);
					if (str_diffn(vpw->pw_shell, "NOQUOTA", 8)) {
						quota_to_megabytes(qconvert, vpw->pw_shell);
						out(qconvert);
					} else {
						if (AdminType == DOMAIN_ADMIN)
							out("NOQUOTA");
						else
							out(html_text[229]);
					}
					break;
				case 'R': /* show returntext/returnhttp if set in CGI vars */
					GetValue(TmpCGI, &value1, "returntext=");
					GetValue(TmpCGI, &value2, "returnhttp=");
					if (value1.len)
						printh("<A HREF=\"%C\">%H</A>", value2.s, value1.s);
					break;
				case 'r': /* show the autoresponder stuff */
					if (AdminType == DOMAIN_ADMIN)
						show_autoresponders(Username.s, Domain.s, mytime);
					break;
				case 'S': /* send the status message parameter */
					out(StatusMessage.s);
					break;
				case 's': /* show the catchall name */
					get_catchall();
					break;
				case 'T': /* send the time parameter */
					strnum1[fmt_ulong(strnum1, (unsigned long) mytime)] = 0;
					out(strnum1);
					break;
				case 't': /* transmit block?  */
					transmit_block(&ssin);
					break;
				case 'U': /* send the username parameter */
					printh("%H", Username.s);
					break;
				case 'u': /* show the users */
					show_users(Username.s, Domain.s, mytime);
					break;
				case 'V': /* show version number */
					out("<a href=\"http://sourceforge.net/projects/indimail/\">");
					out(PACKAGE);
					out("</a> ");
					out(VERSION);
					out("<BR>");
					out("<a href=\"http://localhost://mailmrtg/\">MRTG Graphs</a><BR>");
					break;
				case 'v': /* display the main menu */
					printh("<font size=\"2\" color=\"#000000\"><b>%H</b></font><br><br>", Domain.s);
					out("<font size=\"2\" color=\"#ff0000\"><b>");
					out(html_text[1]);
					out("</b></font><br>");
					if (AdminType == DOMAIN_ADMIN) {
						if (MaxPopAccounts != 0) {
							printh("<a href=\"%s\">", cgiurl("showusers"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[61]);
							out("</b></font></a><br>");
						}
						if (MaxForwards != 0 || MaxAliases != 0) {
							printh("<a href=\"%s\">", cgiurl("showforwards"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[122]);
							out("</b></font></a><br>");
						}
						if (MaxAutoResponders != 0) {
							printh("<a href=\"%s\">", cgiurl("showautoresponders"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[77]);
							out("</b></a></font><br>");
						}
						if (*EZMLMDIR != 'n' && MaxMailingLists != 0) {
							printh("<a href=\"%s\">", cgiurl("showmailinglists"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[80]);
							out("</b></font></a><br>");
						}
					} else {
						/*
						 * the quota code in here is kinda screwy and could use review
						 * then again, with recent changes, the non-admin shouldn't
						 * even get to this page.
						 */
						mdir_t          diskquota = 0;
						mdir_t          maxmsg = 0;

						vpw = sql_getpw(Username.s, Domain.s);
						printh("<a href=\"%s&moduser=%C\">", cgiurl("moduser"), Username.s);
						printh("<font size=\"2\" color=\"#000000\"><b>%s %H</b></font></a><br><br>", html_text[111], Username.s);
						if (str_diffn(vpw->pw_shell, "NOQUOTA", 8)) {
							quota_to_megabytes(qconvert, vpw->pw_shell);
						} else {
							str_copy(qconvert, html_text[229]);
							qnote = "";
						}
						out("<font size=\"2\" color=\"#000000\"><b>");
						out(html_text[249]);
						out(":</b><br>");
						out(html_text[253]);
						out(" ");
						out(qconvert);
						out(" ");
						out(qnote);
						out("<br>");
						out(html_text[254]);
						if (!stralloc_copys(&TmpBuf, vpw->pw_dir) ||
								!stralloc_catb(&TmpBuf, "/Maildir", 8) ||
								!stralloc_0(&TmpBuf))
							die_nomem();
						diskquota = check_quota(TmpBuf.s, &maxmsg);
						strnum1[fmt_double(strnum1, (double) diskquota/1048576.0, 2)] = 0;
						out(strnum1);
						out(" MB</font><br>");	/* Convert to MB */
					}
					if (AdminType == DOMAIN_ADMIN) {
						out("<br>");
						if (MaxPopAccounts != 0) {
							printh("<a href=\"%s\">", cgiurl("adduser"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[125]);
							out("</b></font></a><br>");
						}
						if (MaxForwards != 0) {
							printh("<a href=\"%s\">", cgiurl("adddotqmail"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[127]);
							out("</b></font></a><br>");
						}
						if (MaxAutoResponders != 0) {
							printh("<a href=\"%s\">", cgiurl("addautorespond"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[128]);
							out("</b></a></font><br>");
						}
						if (*EZMLMDIR != 'n' && MaxMailingLists != 0) {
							printh("<a href=\"%s\">", cgiurl("addmailinglist"));
							out("<font size=\"2\" color=\"#000000\"><b>");
							out(html_text[129]);
							out("</b></font></a><br>");
						}
					}
					break;
				case 'W': /* Password */
					printh("%H", Password.s);
					break;
				case 'X': /* dictionary entry, followed by three more chars for the entry # */
					for (i = 0; i < 3; ++i)
						dchar[i] = getch(&ssin);
					dchar[i] = 0;
					scan_int(dchar, &i);
					if ((i >= 0) && (i <= MAX_LANG_STR))
						out(html_text[i]);
					break;
				case 'x': /* exit / logout link/text */
					if (!stralloc_copys(&value1, get_session_val("returntext=")) ||
							!stralloc_0(&value1))
						die_nomem();
					value1.len--;
					if (value1.len)
						printh("<a href=\"%C\">%H", get_session_val("returnhttp="), value1.s);
					else
						printh("<a href=\"%s\">%s", cgiurl("logout"), html_text[218]);
					out("</a>\n");
					break;
				case 'y': /* returnhttp */
					printh("%C", get_session_val("returnhttp="));
					break;
				case 'Y': /* returntext */
					printh("%H", get_session_val("returntext="));
					break;
				case 'Z': /* send the image URL directory */
					out(IMAGEURL);
					break;
				case 'z':
					/*
					 * display domain on login page (last used, value of dom in URL,
					 * or guess from hostname in URL).
					 */
					if (Domain.len)
						printh("%H", Domain.s);
					else
					if (TmpCGI && GetValue(TmpCGI, &value1, "dom=") == 0) {
						printh("%H", value1.s);
#ifdef DOMAIN_AUTOFILL
					} else {
						get_calling_host();
#endif
					}
					break;
				default:
					break;
				}
			} else {
				/*
				 * didn't find a tag, so send out the first '#' and 
				 * the current character
				 */
				putch('#', subfdoutsmall);
				putch(inchar, subfdoutsmall);
			}
		}
	}
	close(fd);
	substdio_flush(subfdoutsmall);
	iclose();
	return 0;
}

/*
 * Display status of mailbox flags 
 */
/*
 * James Raftery <james@now.ie> 12 Dec. 2002 / 15 Apr. 2003 
 */
void
check_mailbox_flags(char newchar)
{
	static struct passwd *vpw = NULL;

	if (!vpw)
		vpw = sql_getpw(ActionUser.s, Domain.s);
	switch (newchar)
	{
	case '1': /* "checked" if V_USER0 is set */
		if (vpw->pw_gid & V_USER0)
			out("checked");
		break;
	case '2': /* "checked" if V_USER0 is unset */
		if (!(vpw->pw_gid & V_USER0))
			out("checked");
		break;
	case '3': /* "checked" if V_USER1 is set */
		if (vpw->pw_gid & V_USER1)
			out("checked");
		break;
	case '4': /* "checked" if V_USER1 is unset */
		if (!(vpw->pw_gid & V_USER1))
			out("checked");
		break;
	case '5': /* "checked" if V_USER2 is set */
		if (vpw->pw_gid & V_USER2)
			out("checked");
		break;
	case '6': /* "checked" if V_USER2 is unset */
		if (!(vpw->pw_gid & V_USER2))
			out("checked");
		break;
	case '7': /* "checked" if V_USER3 is set */
		if (vpw->pw_gid & V_USER3)
			out("checked");
		break;
	case '8': /* "checked" if V_USER3 is unset */
		if (!(vpw->pw_gid & V_USER3))
			out("checked");
		break;
	default:
		break;
	}
	flush();
	return;
}

/*
 * tests to see if text between ##tX and ##tt should be transmitted 
 * where X is a letter corresponding to one of the below values     
 *
 * jeff.hedlund@matrixsi.com                                        
 */
void
transmit_block(substdio *ss)
{
	char            inchar;

	inchar = getch(ss);
	switch (inchar)
	{
	case 'a': /* administrative commands */
		if (AdminType != DOMAIN_ADMIN)
			ignore_to_end_tag(ss);
		break;
	case 'h':
#ifndef HELP
		ignore_to_end_tag(ss);
#endif
		break;
	case 'm':
#ifndef ENABLE_MYSQL
		ignore_to_end_tag(ss);
#endif
		break;
	case 'q':
#ifndef MODIFY_QUOTA
		ignore_to_end_tag(ss);
#endif
		break;
	case 's':
#ifndef MODIFY_SPAM
		ignore_to_end_tag(ss);
#endif
		break;
	case 't': /* explicitly break if it was ##tt, that's an end tag */
		break;
	case 'u': /* user (not administrative) */
		if (AdminType != USER_ADMIN) {
			ignore_to_end_tag(ss);
		}
		break;
	}
}

/*
 * simply looks for the ending tag (##tt) ignoring everything else 
 *
 * jeff.hedlund@matrixsi.com                                       
 */
void
ignore_to_end_tag(substdio *ss)
{
	int             nested = 0;
	int             inchar;

	inchar = getch(ss);
	while (inchar >= 0) {
		if (inchar == '#') {
			inchar = getch(ss);
			if (inchar == '#') {
				inchar = getch(ss);
				if (inchar == 't') {
					inchar = getch(ss);
					if (inchar == 't' && nested == 0)
						return;
					else if (inchar != 't')
						nested++;
					else
						nested--;
				}
			}
		}
		inchar = getch(ss);
	}
}

/*
 * printout the virtual host that matches the HTTP_HOST 
 */
void
get_calling_host()
{
	char           *srvnam, *domptr;
	int             fd, match;
	char            inbuf[1024];
	struct substdio ssin;

	if (!stralloc_copys(&TmpBuf, SYSCONFDIR) ||
			!stralloc_catb(&TmpBuf, "/control/virtualdomains", 23) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((srvnam = env_get("HTTP_HOST")))
		lowerit(srvnam);
	if ((fd = open_read(TmpBuf.s)) == -1) {
		strerr_warn3("get_calling_host: open: ", TmpBuf.s, ": ", &strerr_sys);
		out(html_text[144]);
		out(" ");
		out(TmpBuf.s);
		out(" 1<BR>\n");
		flush();
		return;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	/*- read Reference: and Subjec: line */
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("get_calling_host: read: ", TmpBuf.s, ": ", &strerr_sys);
			out(html_text[144]);
			out(" ");
			out(TmpBuf.s);
			out(" 1<BR>\n");
			close(fd);
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
		if (!line.len)
			continue;
		domptr = line.s;
		for (; *domptr && *domptr != ':'; domptr++);
		domptr++;
		lowerit(domptr);
		if (str_str(srvnam, domptr)) {
			printh("%H", domptr);
			break;
		}
	}
	close(fd);
}

/*
 * returns the value of session_var, first checking the .qw file for saved 
 *
 * value, or the TmpCGI if it's not yet been saved                         
 */
char           *
get_session_val(char *session_var)
{
	static stralloc value = {0};
	int             fd, match;
	char           *retval;
	char            strnum[1024], inbuf[1024];
	struct passwd  *vpw;
	struct substdio ssin;

	retval = "";
	if ((vpw = sql_getpw(Username.s, Domain.s))) {
		if (!stralloc_copys(&TmpBuf, vpw->pw_dir) ||
				!stralloc_catb(&TmpBuf, "/Maildir/", 9) ||
				!stralloc_catb(&TmpBuf, strnum, fmt_ulong(strnum, (unsigned long) mytime)) ||
				!stralloc_catb(&TmpBuf, ".qw", 3) ||
				!stralloc_0(&TmpBuf))
			die_nomem();
		if ((fd = open_read(TmpBuf.s)) == -1) {
			strerr_warn3("get_session_val: open: ", TmpBuf.s, ": ", &strerr_sys);
			return (retval);
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("get_session_val: read: ", TmpBuf.s, ": ", &strerr_sys);
			close(fd);
			return (retval);
		}
		if (line.len && !GetValue(line.s, &value, session_var))
			retval = value.s;
		close(fd);
	} else
	if (TmpCGI && !GetValue(TmpCGI, &value, session_var))
		retval = value.s;
	return (retval);
}
