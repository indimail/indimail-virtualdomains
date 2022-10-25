/*
 * $Id: alias.c,v 1.9 2022-10-25 23:49:25+05:30 Cprogrammer Exp mbhangui $
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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
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
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <alloc.h>
#include <scan.h>
#include <fmt.h>
#include <case.h>
#include <str.h>
#endif
#include "alias.h"
#include "forward.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "dotqmail.h"
#include "html.h"
#include "limits.h"
#include "util.h"
#include "printh.h"
#include "show.h"
#include "template.h"
#include "common.h"

char           *dotqmail_alias_command(char *command);

int
show_aliases(void)
{
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	send_template("show_alias.html");
	return 0;
}

struct aliasentry {
	char           *alias_name;
	char           *alias_command;
	struct aliasentry *next;
};

struct aliasentry *firstalias = (struct aliasentry *) 0, *curalias = (struct aliasentry *) 0;

void
add_alias_entry(char *alias_name, char *alias_command)
{
	int             len;

	if (!firstalias) {
		firstalias = (struct aliasentry *) alloc(sizeof (struct aliasentry));
		curalias = firstalias;
	} else {
		curalias->next = (struct aliasentry *) alloc(sizeof (struct aliasentry));
		curalias = curalias->next;
	}
	curalias->next = (struct aliasentry *) 0;
	curalias->alias_name = alloc(len = (str_len(alias_name) + 1));
	str_copyb(curalias->alias_name, alias_name, len);
	curalias->alias_command = alloc(len = (str_len(alias_command) + 1));
	str_copyb(curalias->alias_command, alias_command, len);
}

struct aliasentry *
get_alias_entry()
{
	struct aliasentry *temp;

	temp = curalias->next;
	alloc_free((char *) curalias->alias_name);
	alloc_free((char *) curalias->alias_command);
	alloc_free((char *) curalias);
	return temp;
}

int str_casecmp(char *s1, char *s2)
{
    while (toupper((unsigned char)*s1) == toupper((unsigned char)*s2++))
	if (*s1++ == '\0')
	    return 0;
    return(toupper((unsigned char)*s1) - toupper((unsigned char)*--s2));
}

int str_casecmpn(char *s1, char *s2, register int n)
{
    while (--n >= 0 && toupper((unsigned char)*s1) == toupper((unsigned char)*s2++))
	if (*s1++ == '\0')
	    return 0;
    return(n < 0 ? 0 : toupper((unsigned char)*s1) - toupper((unsigned char)*--s2));
}

void
show_dotqmail_lines(char *user, char *dom, time_t mytime)
{
	int             moreusers = 0, stop, i, k, startnumber, len, plen;
	unsigned int    page;
	static stralloc alias_user = {0}, alias_copy = {0}, this_alias = {0};
	char           *alias_domain, *alias_name_from_command;
#ifdef VALIAS
	static stralloc alias_name = {0};
	static stralloc domain = {0};
	char           *alias_line;
#else
	DIR            *mydir;
	struct dirent  *mydirent, dirent **namelist;
	int             i, j, m, n, fd;
	struct stat     sbuf;
#endif

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	scan_uint(Pagenumber, &page);
	if (page == 0)
		page = 1;
	startnumber = MAXALIASESPERPAGE * (page - 1);
	k = 0;
#ifdef VALIAS
	if (SearchUser.len) {
		startnumber = 0;
		if (!stralloc_copy(&domain, &Domain) || !stralloc_0(&domain)) {
			copy_status_mesg(html_text[201]);
			iclose();
			exit(0);
		}
		domain.len--;
		alias_line = valias_select_all(&alias_name, &domain);
		while (alias_line) {
			if (!stralloc_copy(&this_alias, &alias_name) ||
					!stralloc_0(&this_alias))
				die_nomem();
			this_alias.len--;
			alias_name_from_command = dotqmail_alias_command(alias_line);
			if (alias_name_from_command || *alias_line == '#') {
				if (str_casecmp(SearchUser.s, alias_name.s) <= 0)
					break;
				startnumber++;
			}
			/*- burn through remaining lines for this alias, if necessary */
			i = alias_name.len > this_alias.len ? alias_name.len : this_alias.len;
			while (alias_line && !str_diffn(this_alias.s, alias_name.s, i)) {
				alias_line = valias_select_all(&alias_name, &domain);
				i = alias_name.len > this_alias.len ? alias_name.len : this_alias.len;
			}
		}
		page = startnumber / MAXALIASESPERPAGE + 1;
		Pagenumber[fmt_uint(Pagenumber, page)] = 0;
	}
	alias_line = valias_select_all(&alias_name, &domain);
	while (alias_line) {
		if (!stralloc_copy(&this_alias, &alias_name) ||
				!stralloc_0(&this_alias))
			die_nomem();
		this_alias.len--;
		alias_name_from_command = dotqmail_alias_command(alias_line);
		if (alias_name_from_command || *alias_line == '#') {
			k++;
			if (k > MAXALIASESPERPAGE + startnumber) {
				moreusers = 1;
				break;
			}
			if (k > startnumber) {
				if (*alias_line == '#') {
					alias_line = valias_select_all(&alias_name, &domain);
					i = alias_name.len > this_alias.len ? alias_name.len : this_alias.len;
					if (str_diffn(this_alias.s, alias_name.s, i)) {
						/*- single comment, treat as blackhole */
						add_alias_entry(this_alias.s, "#");
						continue;
					} else
						alias_name_from_command = dotqmail_alias_command(alias_line);
				}
				while (1) {
					if (alias_name_from_command)
						add_alias_entry(alias_name.s, alias_name_from_command);
					alias_line = valias_select_all(&alias_name, &domain);

					/*- exit if we run out of alias lines, or go to a new alias name */
					i = alias_name.len > this_alias.len ? alias_name.len : this_alias.len;
					if (!alias_line || str_diffn(this_alias.s, alias_name.s, i))
						break;
					alias_name_from_command = dotqmail_alias_command(alias_line);
				}
			}
		}
		/*- burn through remaining lines for this alias, if necessary */
		i = alias_name.len > this_alias.len ? alias_name.len : this_alias.len;
		while (alias_line && !str_diffn(this_alias.s, alias_name.s, i)) {
			alias_line = valias_select_all(&alias_name, &domain);
			i = alias_name.len > this_alias.len ? alias_name.len : this_alias.len;
		}
	}
#endif
	curalias = firstalias;
	while (curalias) {
		if (!stralloc_copys(&this_alias, curalias->alias_name) ||
				!stralloc_0(&this_alias))
			die_nomem();
		this_alias.len--;
		/*-
		 * display the entry 
		 * This is a big assumption, and may cause problems at some point.
		 */
		out(HTML_ALIAS_ROW_START);
		qmail_button(this_alias.s, "deldotqmail", user, dom, mytime, "trash.png");
		if (*curalias->alias_command == '#') {
			out(HTML_EMPTY_TD); /* don't allow modify on blackhole */
			flush();
		} else
			qmail_button(this_alias.s, "moddotqmail", user, dom, mytime, "modify.png");
		printh(HTML_ALIAS_NAME, this_alias.s);
		out(HTML_ALIAS_DEST_START);
		stop = 0;
		if (*curalias->alias_command == '#') { /*- this is a blackhole account */
			printh(HTML_ALIAS_BLACKHOLE, html_text[303]);
			stop = 1;
		}
		while (!stop) {
			if (!stralloc_copys(&alias_copy, curalias->alias_command) ||
					!stralloc_0(&alias_copy))
				die_nomem();
			alias_copy.len--;
			/*- get the domain alone from alias_copy */
			for (alias_domain = alias_copy.s; *alias_domain && *alias_domain != '@' && *alias_domain != ' '; alias_domain++);

			/*- if a local user, strip domain name from address */
			if ((*alias_domain == '@') && !str_casecmpn(alias_domain + 1, Domain.s, Domain.len)) {
				/*- strip domain name from address */
				*alias_domain = '\0';
				if (!check_local_user(alias_copy.s)) {
					/*
					 * make it red so it jumps out -- this is no longer a valid forward 
					 */
					len = str_len(curalias->alias_command) + str_len(HTML_ALIAS_INVALID) + 28;
					for (plen = 0;;) {
						if (!stralloc_ready(&alias_user, len))
							die_nomem();
						plen = snprinth(alias_user.s, len, HTML_ALIAS_INVALID, curalias->alias_command);
						if (plen < len) {
							alias_user.len = plen;
							break;
						}
						len = plen + 28;
					}
				} else {
					len = alias_copy.len + str_len(HTML_ALIAS_LOCAL) + 28;
					for (plen = 0;;) {
						if (!stralloc_ready(&alias_user, len))
							die_nomem();
						plen = snprinth(alias_user.s, len, HTML_ALIAS_LOCAL, alias_copy);
						if (plen < len) {
							alias_user.len = plen;
							break;
						}
						len = plen + 28;
					}
				}
			} else {
				len = str_len(curalias->alias_command) + str_len(HTML_ALIAS_REMOTE) + 28;
				for (plen = 0;;) {
					if (!stralloc_ready(&alias_user, len))
						die_nomem();
					plen = snprinth(alias_user.s, len, HTML_ALIAS_REMOTE, curalias->alias_command);
					if (plen < len) {
						alias_user.len = plen;
						break;
					}
					len = plen + 28;
				}
			}

			/* find next entry, so we know if we should print a , or not */
			while (1) {
				curalias = get_alias_entry();
				/*- exit if we run out of alias lines, or go to a new alias name */
				if (!curalias || str_diffn(this_alias.s, curalias->alias_name, this_alias.len)) {
					stop = 1;
					if (!stralloc_0(&alias_user))
						die_nomem();
					alias_user.len--;
					out(alias_user.s);
					break;
				}
				if (!stralloc_0(&alias_user))
					die_nomem();
				alias_user.len--;
				out(alias_user.s);
				out(", ");
				break;
			}
		}
		/*- burn through any remaining entries */
		while (curalias && !str_diffn(this_alias.s, curalias->alias_name, this_alias.len))
			curalias = get_alias_entry();
		out(HTML_ALIAS_DEST_END);
		out(HTML_ALIAS_ROW_END);
		flush();
	}

	if (AdminType == DOMAIN_ADMIN) {
		print_user_index("showforwards", 4, user, dom, mytime);
		out(HTML_ALIAS_FOOTER_START);
		out(HTML_MENU_START);
		/*-
		 * When searching for a user on systems using .qmail files, we make things
		 * easy by starting the page with the first matching address.  As a result,
		 * the previous page will be 'page' and not 'page-1'.  Refresh is accomplished
		 * by repeating the search.
		 */
		if (SearchUser.len && ((startnumber % MAXALIASESPERPAGE) != 1)) {
			/* previous page */
			printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page, html_text[135]);
			out(HTML_MENU_SEP);
			/* refresh */
			printh(HTML_ALIAS_DOSEARCH, cgiurl("showforwards"), SearchUser.s, html_text[136]);
		} else {
			if (page > 1) {
				/* previous page */
				printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page - 1, html_text[135]);
				out(HTML_MENU_SEP);
			}
			/* refresh */
			printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page, html_text[136]);
		}
		if (moreusers) {
			out(HTML_MENU_SEP);
			/* next page */
			printh(HTML_ALIAS_SHOWPAGE, cgiurl("showforwards"), page + 1, html_text[137]);
		}
		out(HTML_MENU_END);
		out(HTML_ALIAS_FOOTER_END);
		flush();
	}
}

/*
 * This Function shows the inside of a .qmail file, 
 * with the edit mode
 */
void
show_dotqmail_file(char *user)
{
	static stralloc alias_user = {0};
	char           *alias_domain, *alias_name_from_command, *alias_line;
	int             firstrow, j, len, plen;

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	for(;;) {
		if (!(alias_line = valias_select(user, Domain.s)))
			break;
		alias_name_from_command = dotqmail_alias_command(alias_line);
		/*
		 * Make sure it is valid before displaying it. 
		 */
		if (alias_name_from_command)
			add_alias_entry(user, alias_line);
	}

	curalias = firstalias;
	firstrow = 1;
	while (curalias) {
		alias_line = curalias->alias_command;
		alias_name_from_command = dotqmail_alias_command(alias_line);
		if (!stralloc_copys(&alias_user, alias_name_from_command) ||
				!stralloc_0(&alias_user))
			die_nomem();
		alias_user.len--;
		/*- get the domain alone from alias_user */
		alias_domain = alias_user.s;
		for (; *alias_domain && *alias_domain != '@' && *alias_domain != ' '; alias_domain++);
		alias_domain++;
		if (!str_diffn(alias_domain, Domain.s, Domain.len)) {
			/*- if a local user, exclude the domain */
			if (!stralloc_copy(&TmpBuf, &alias_user) ||
					!stralloc_0(&TmpBuf))
				die_nomem();
			TmpBuf.len--;
			for (j = 0; TmpBuf.s[j] != 0 && TmpBuf.s[j] != '@'; j++);
			TmpBuf.s[j] = 0;
			if (check_local_user(TmpBuf.s)) {
				len = str_len(TmpBuf.s) + str_len(HTML_ALIAS_LOCAL) + 28;
				for (plen = 0;;) {
					if (!stralloc_ready(&alias_user, len))
						die_nomem();
					plen = snprinth(alias_user.s, len, HTML_ALIAS_LOCAL, TmpBuf.s);
					if (plen < len) {
						alias_user.len = plen;
						break;
					}
					len = plen + 28;
				}
			} else {
				/*- make it red so it jumps out -- this is no longer a valid forward */
				len = str_len(TmpBuf.s) + str_len(HTML_ALIAS_INVALID) + 28;
				for (plen = 0;;) {
					if (!stralloc_ready(&alias_user, len))
						die_nomem();
					plen = snprinth(alias_user.s, len, HTML_ALIAS_INVALID, alias_name_from_command);
					if (plen < len) {
						alias_user.len = plen;
						break;
					}
					len = plen + 28;
				}
			}
		} else {
			len = str_len(TmpBuf.s) + str_len(HTML_ALIAS_REMOTE) + 28;
			for (plen = 0;;) {
				if (!stralloc_ready(&alias_user, len))
					die_nomem();
				plen = snprinth(alias_user.s, len, HTML_ALIAS_REMOTE, alias_name_from_command);
				if (plen < len) {
					alias_user.len = plen;
					break;
				}
				len = plen + 28;
			}
		}
		out(HTML_ALIAS_MOD_ROW_START);
		if (firstrow) {
			firstrow = 0;
			printh(HTML_ALIAS_MOD_NAME, user);
		} else
			printh(HTML_ALIAS_MOD_NAME, "");
		printh(HTML_ALIAS_MOD_DEST, alias_user);
		printh(HTML_ALIAS_MOD_DELETE, cgiurl("moddotqmailnow"), user, alias_line);
		out(HTML_ALIAS_MOD_ROW_END);
		flush();
		curalias = get_alias_entry();
	}
}

int
onevalidonly(char *user)
{
	char           *alias_line;
	int             lines;

	lines = 0;
	for (;;) {
		if (!(alias_line = valias_select(user, Domain.s)))
			break;
		/*
		 * check to see if it is an invalid line , if so skip to next 
		 */
		if (dotqmail_alias_command(alias_line))
			lines++;
	}
	return (lines < 2);
}

void
moddotqmail()
{
	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	send_template("mod_dotqmail.html");
}

void
moddotqmailnow()
{
	int              len, plen;

	if (!str_diffn(ActionUser.s, "default", ActionUser.len)) {
		iclose();
		exit(0);
	}

	if (!str_diff(Action.s, "delentry")) {
		if (onevalidonly(ActionUser.s))
			copy_status_mesg(html_text[149]);
		else
		if (dotqmail_del_line(ActionUser.s, LineData.s)) {
			copy_status_mesg(html_text[150]);
			if (!stralloc_catb(&StatusMessage, " valias delete", 14) ||
					!stralloc_0(&StatusMessage))
				die_nomem();
			StatusMessage.len--;
		} else
			copy_status_mesg(html_text[151]);
	} else
	if (str_diff(Action.s, "add") == 0) {
		if (!adddotqmail_shared(ActionUser.s, &Newu, 0)) {
			len = Newu.len + str_len(html_text[152]) + 28;
			for (plen = 0;;) {
				if (!stralloc_ready(&StatusMessage, len))
					die_nomem();
				plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[152], Newu.s);
				if (plen < len) {
					StatusMessage.len = plen;
					break;
				}
				len = plen + 28;
			}
		}
	} else
		copy_status_mesg(html_text[155]);
	moddotqmail();
	iclose();
	exit(0);
}

void
adddotqmail()
{
	char             strnum[FMT_ULONG];

	count_forwards();
	load_limits();
	if (MaxForwards != -1 && CurForwards >= MaxForwards) {
		copy_status_mesg(html_text[157]);
		if (!stralloc_append(&StatusMessage, " ") ||
				!stralloc_catb(&StatusMessage, strnum, fmt_int(strnum, MaxForwards)) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		StatusMessage.len--;
		show_menu();
		iclose();
		exit(0);
	}
	send_template("add_forward.html");
}


void
adddotqmailnow()
{
	char             strnum[FMT_ULONG];

	if (AdminType != DOMAIN_ADMIN && !(AdminType == USER_ADMIN && !str_diffn(ActionUser.s, Username.s, Username.len > ActionUser.len ? Username.len : ActionUser.len))) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	count_forwards();
	load_limits();
	if (MaxForwards != -1 && CurForwards >= MaxForwards) {
		copy_status_mesg(html_text[157]);
		if (!stralloc_append(&StatusMessage, " ") ||
				!stralloc_catb(&StatusMessage, strnum, fmt_int(strnum, MaxForwards)) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		StatusMessage.len--;
		send_template("add_forward.html");
		iclose();
		exit(0);
	}
	if (adddotqmail_shared(Alias.s, &ActionUser, -1)) {
		adddotqmail();
		iclose();
		exit(0);
	} else {
		copy_status_mesg(html_text[152]);
		show_forwards(Username.s, Domain.s, mytime);
	}
}

int
adddotqmail_shared(char *forwardname, stralloc *dest, int create)
{
	int              len, plen, i;

	if ((plen = str_len(forwardname)) <= 0) {
		copy_status_mesg(html_text[163]);
		return (-1);
	} else /*- make sure forwardname is valid */
	if (fixup_local_name(forwardname)) {
		len = str_len(html_text[163]) + plen + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[163], forwardname);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		return (-1);
	} else /*- check to see if we already have a user with this name (only for create) */
	if (create != 0 && check_local_user(forwardname)) {
		len = str_len(html_text[175]) + plen + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[175], forwardname);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		return (-1);
	}

	if (str_diff(dest->s, "#") == 0) {
		if (dotqmail_add_line(forwardname, "#")) {
			copy_status_mesg(html_text[150]);
			if (!stralloc_catb(&StatusMessage, " valias insert", 14) ||
					!stralloc_0(&StatusMessage))
				die_nomem();
			StatusMessage.len--;
			return (-1);
		}
		return 0;
	}

	/*- see if forwarding to a local user */
	i = str_chr(dest->s, '@');
	if (!dest->s[i]) {
		if (check_local_user(dest->s) == 0) {
			copy_status_mesg(html_text[161]);
			return (-1);
		} else {/*- make it an email address */
			if (!stralloc_append(dest, "@") ||
					!stralloc_cat(dest, &Domain) ||
					!stralloc_0(dest))
				die_nomem();
			dest->len--;
		}
	}

	/*- check that it's a valid email address */
	if (check_email_addr(dest->s)) {
		len = str_len(html_text[162]) + dest->len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[162], dest->s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		return (-1);
	}
	if (!stralloc_copyb(&TmpBuf, "&", 1) ||
			!stralloc_cat(&TmpBuf, dest) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	TmpBuf.len--;
	if (dotqmail_add_line(forwardname, TmpBuf.s)) {
		copy_status_mesg(html_text[150]);
		if (!stralloc_catb(&StatusMessage, " valias insert", 14) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		StatusMessage.len--;
		return (-1);
	}

	return (0);
}

void
deldotqmail()
{

	if (AdminType != DOMAIN_ADMIN) {
		out(html_text[142]);
		out("\n");
		flush();
		iclose();
		exit(0);
	}
	send_template("del_forward_confirm.html");

}

void
deldotqmailnow()
{
	int              len, plen;

	if (AdminType != DOMAIN_ADMIN && !(AdminType == USER_ADMIN && !str_diffn(ActionUser.s, Username.s, ActionUser.len > Username.len ? ActionUser.len : Username.len))) {
		copy_status_mesg(html_text[142]);
		show_menu();
		iclose();
		exit(0);
	}
	if (fixup_local_name(ActionUser.s)) {
		len = str_len(html_text[160]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[160], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
		deldotqmail();
		iclose();
		exit(0);
	}

	if (!dotqmail_delete_files(ActionUser.s)) {
		len = str_len(html_text[167]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[167], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	} else {
		len = str_len(html_text[168]) + ActionUser.len + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&StatusMessage, len))
				die_nomem();
			plen = snprinth(StatusMessage.s, len, "%s %H\n", html_text[168], ActionUser.s);
			if (plen < len) {
				StatusMessage.len = plen;
				break;
			}
			len = plen + 28;
		}
	}

	/*- don't display aliases/forwards if we just deleted the last one */
	count_forwards();
	if (CurForwards == 0 && CurBlackholes == 0) {
		show_menu();
	} else {
		if (!stralloc_copy(&SearchUser, &ActionUser) ||
				!stralloc_0(&SearchUser))
			die_nomem();
		SearchUser.len--;
		show_forwards(Username.s, Domain.s, mytime);
	}
}

char           *
dotqmail_alias_command(char *line)
{
	int             len, plen, i;
	static stralloc user = {0}, command = {0}, dot_qmail = {0};
	char           *s, *b;

	if (!line)
		return((char *) 0);
	if (!*line)
		return((char *) 0);
	if (*line == '#')
		return((char *) 0);

	/*- copy everything up to the first whitespace */
	for (len = 0; line[len] != 0 && !isspace(line[len]); len++);
	if (!stralloc_copyb(&command, line, len) ||
			!stralloc_0(&command))
		die_nomem();
	command.len--;
	/*
	 * If it ends with a slash and starts with a / or . then
	 * this is a Maildir delivery, local alias
	 */
	if (command.s[command.len - 1] == '/' && (command.s[0] == '/' || command.s[0] == '.')) {
		if (!stralloc_copy(&user, &command) ||
				!stralloc_0(&user))
			die_nomem();
		user.len--;
		user.s[user.len - 1] = 0;
		user.len--;
		b = (char *) 0; /* pointer to mailbox name */
		i = str_rchr(user.s, '/');
		if (!user.s[i])
			return((char *) 0);
		s = user.s + i;
		if (str_diffn(s, "/Maildir", 9)) {
			b = s + 2; /* point to name of submailbox. e.g. /mail/billyjoel/Maildir/.newsongs */
			*s = 0;
			i = str_rchr(user.s, '/');
			if (!user.s[i])
				return((char *) 0);
			s = user.s + i;
			if (str_diffn(s, "/Maildir", 9))
				return((char *) 0);
		}
		*s = 0;
		i = str_rchr(user.s, '/');
		if (!user.s[i])
			return((char *) 0);
		if (b) {
			i = str_len(s + 1) + str_len(b) + 28;
			for (plen = 0;;) {
				if (!stralloc_ready(&user, i))
					die_nomem();
				plen = snprinth(user.s, i, "%H <I>(%H)</I>", s + 1, b);
				if (plen < i) {
					user.len = plen;
					break;
				}
				i = plen + 28;
			}
		} else {
			i = str_len(s + 1) + 28;
			for (plen = 0;;) {
				if (!stralloc_ready(&user, i))
					die_nomem();
				plen = snprinth(user.s, i, "%H", s + 1);
				if (plen < i) {
					user.len = plen;
					break;
				}
				i = plen + 28;
			}
		}
		return (user.s);
	} else /*- if it's an email address then display the forward */
	if (!check_email_addr((command.s + 1))) {
		if (command.s[0] == '&')
			return (command.s + 1);
		else
			return (command.s);
	} else /*- if it is a program then */
	if (command.s[0] == '|') {

		/*- do not display ezmlm programs */
		if (str_str(command.s, "ezmlm"))
			return((char *) 0);
		/*- do not display autoresponder programs */
		if (str_str(command.s, "autoresponder"))
			return((char *) 0);
		/*
		 * otherwise, display the program 
		 * back up to pipe or first slash to remove path 
		 */
		while (line[len] != '/' && line[len] != '|')
			len--;
		len++; /* len is now first char of program name */
		i = str_len(line + len) + 28;
		for (plen = 0;;) {
			if (!stralloc_ready(&command, i))
				die_nomem();
			plen = snprinth(command.s, i, "<I>%H</I>", line + len);
			if (plen < i) {
				command.len = plen;
				break;
			}
			i = plen + 28;
		}
		return (command.s);
	} else /*- otherwise just report nothing */
	if (!check_indimail_alias(command.s + 1, &dot_qmail)) {
		if (command.s[0] == '&')
			return (dot_qmail.s);
		else
			return (command.s);
	} else
		return((char *) 0);
}
