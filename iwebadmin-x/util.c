/*
 * $Id: util.c,v 1.12 2024-05-17 16:17:42+05:30 mbhangui Exp mbhangui $
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
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <open.h>
#include <stralloc.h>
#include <strerr.h>
#include <substdio.h>
#include <getln.h>
#include <env.h>
#include <case.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <error.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#include "alias.h"
#include "autorespond.h"
#include "forward.h"
#include "mailinglist.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "printh.h"
#include "user.h"
#include "util.h"
#include "common.h"

#ifdef SORT_TABLE_ENTRIES
#undef SORT_TABLE_ENTRIES
#endif
#define SORT_TABLE_ENTRIES 100000

/*
 * pointer to array of pointers 
 */
unsigned char **sort_list;
unsigned char  *sort_block[200];	/* memory blocks for sort data */
int             memleft, memindex, sort_entry;
unsigned char  *sort_ptr;
extern int      color_table;

int
sort_init()
{
	sort_entry = memindex = memleft = 0;
	sort_list = (unsigned char **) alloc(SORT_TABLE_ENTRIES * sizeof (char *));
	if (!sort_list)
		return -1;
	return 0;
}

/*
 * add entry to list for sorting, copies string until it gets to char 
 * 'end' 
 */
int
sort_add_entry(const char *entry, char end)
{
	int             len;

	if (sort_entry == SORT_TABLE_ENTRIES) {
		return -2;	/* table is full */
	}
	if (memleft < 256) {
		/*- allocate a 64K block of memory to store table entries */
		memleft = 65536;
		sort_ptr = sort_block[memindex++] = (unsigned char *) alloc(memleft);
	}
	if (!sort_ptr)
		return -1;
	sort_list[sort_entry++] = sort_ptr;
	len = 1; /* at least a terminator */
	while (*entry && *entry != end) {
		*sort_ptr++ = *entry++;
		len++;
	}
	*sort_ptr++ = 0; /* NULL terminator */
	memleft -= len;
	return 0;
}

unsigned char  *
sort_get_entry(int idx)
{
	if (idx < 0 || idx >= sort_entry)
		return ((unsigned char *) 0);
	return (sort_list[idx]);
}

void
sort_cleanup()
{
	while (memindex)
		alloc_free((char *) sort_block[--memindex]);
	if (sort_list) {
		alloc_free((char *) sort_list);
		sort_list = (unsigned char **) 0;
	}
}

/*
 * Comparison routine used in qsort for multiple functions 
 */
static int
sort_compare(const void *p1, const void *p2)
{
	return case_diffs((char *) p1, (char *) p2);
}

void
sort_dosort()
{
	qsort(sort_list, sort_entry, sizeof (char *), sort_compare);
}

void
str_replace(char *s, char orig, char repl)
{
	while (*s) {
		if (*s == orig) {
			*s = repl;
		}
		s++;
	}
}

void
qmail_button(const char *modu, const char *command, const char *user, const char *dom, time_t mytime, const char *png)
{
	out("<td align=center>");
	printh("<a href=\"%s&modu=%C\">", cgiurl(command), modu);
	out("<img src=\"");
	out(IMAGEURL);
	out("/");
	out(png);
	out("\" border=0></a>");
	out("</td>\n");
	flush();
}

int
check_local_user(const char *user)
{
	char           *ptr;

	/*- check for aliases and autoresponders */
	if (valias_select(user, Domain.s))
		return (-1);
	/*- check for mailing list */
	if (!stralloc_copyb(&TmpBuf, ".qmail-", 7) ||
			!stralloc_cats(&TmpBuf, user) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	for (ptr = TmpBuf.s + 7; *ptr; ptr++)
		if (*ptr == '.')
			*ptr = ':';
	if (!access(TmpBuf.s, F_OK))
		return (1);
	/*- check for POP/IMAP user */
	if (sql_getpw(user, Domain.s))
		return (1);
	return (0);
}

void
show_counts()
{
	char            strnum[FMT_ULONG];

	count_users();
	count_forwards();
	count_autoresponders();
	count_mailinglists();

	out(html_text[61]);
	out(" = ");
	strnum[fmt_int(strnum, CurPopAccounts)] = 0;
	out(strnum);
	out("<BR>\n");

	out(html_text[74]);
	out(" = ");
	strnum[fmt_int(strnum, CurForwards)] = 0;
	out(strnum);
	out("<BR>\n");

	out(html_text[77]);
	out(" = ");
	strnum[fmt_int(strnum, CurAutoResponders)] = 0;
	out(strnum);
	out("<BR>\n");

	out(html_text[80]);
	out(" = ");
	strnum[fmt_int(strnum, CurMailingLists)] = 0;
	out(strnum);
	out("<BR>\n");
}

int
check_indimail_alias(const char *addr, stralloc *dest)
{
	static stralloc line = {0};
	char            inbuf[1024];
	struct substdio ssin;
	int             fd, match;

	if (!stralloc_copys(&TmpBuf, INDIMAILDIR) ||
			!stralloc_catb(&TmpBuf, "/alias/.qmail-", 14) ||
			!stralloc_cats(&TmpBuf, addr) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) == -1) {
		if (errno != error_noent)
			return -1;
		strerr_warn3("check_indimail_alias: ", TmpBuf.s, ": ", &strerr_sys);
		return -1;
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for(;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			close(fd);
			return (-1);
		}
		if (!match || !line.len)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		if (dest) {
			if (!stralloc_copy(dest, &line) ||
					!stralloc_0(dest))
				die_nomem();
			dest->len--;
		}
		return (0);
	}
	return (1);
}

/*
 * check_email_addr( char *addr )
 *
 * Make sure 'addr' is a valid email address.  Returns 1 if it's bad,
 * 0 if it's good.
 */
int
check_email_addr(char *addr)
{
	const char     *atpos = 0;
	int             i, dotpos, len;
	char            allowed_char[] = ".-+=_&";

	len = str_len(allowed_char);
	for (; *addr; ++addr) {
		if (*addr == '@') {
			if (atpos)
				return 1; /* double @ */
			atpos = addr;
		} else {
			i = str_chr(allowed_char, *addr);
			if (!isalnum(*addr) && i == len)
				return 1;
		}
	}
	/*- if just a user name with no @domain.com then bad */
	if (!atpos)
		return 1;
	/*- Look for a sub domain */
	dotpos = str_chr(atpos, '.');
	/*- no '.' in the domain part */
	if (!atpos[dotpos])
		return 1;
	/*- once we know it's good, convert it to lowercase */
	lowerit(addr);
	return 0;
}

int
fixup_local_name(char *addr)
{
	char            allowed_char[] = ".-+=_&";
	int             i, len;

	/*- don't allow zero length user names */
	if (!str_len(addr))
		return (1);
	/*- force it to lower case */
	lowerit(addr);
	/*- check for valid email address */
	len = str_len(allowed_char);
	for (; *addr != 0; ++addr) {
		if (!isalnum(*addr) && !ispunct(*addr))
			return (1);
		if (isspace(*addr))
			return (1);
		i = str_chr(allowed_char, *addr);
		if (ispunct(*addr) && i == len) {
			return (1);
		}
	}
	/*- if we made it here, everything is okay */
	return (0);
}

void
ack(const char *msg, const char *extra)
{
	out(get_html_text(msg));
	out(" ");
	out(extra);
	out("\n");
	out("</BODY></HTML>\n");
	flush();
	iclose();
	iweb_exit(0);
}

void
upperit(char *instr)
{
	while (*instr != 0) {
		if (islower(*instr))
			*instr = toupper(*instr);
		++instr;
	}
}

const char     *
safe_getenv(const char *var)
{
	char           *s;

	if (!(s = env_get(var)))
		return ("");
	return (s);
}

const char     *
strstart(const char *sstr, const char *tstr)
{
	const char     *ret_str;

	ret_str = sstr;
	if (!sstr || !tstr)
		return ((char *) 0);
	while (*sstr && *tstr) {
		if (*sstr != *tstr)
			return ((char *) 0);
		++sstr;
		++tstr;
	}
	if (!*tstr)
		return (ret_str);
	return ((char *) 0);

}

int
open_lang(char *lang)
{
	static stralloc langfile = {0};
	static char    *langpath = (char *) 0;
	struct stat     mystat;
	int             i;
	extern int      lang_fd;

	/*- do not read lang files with path control characters */
	i = str_chr(lang, '.');
	if (lang[i])
		return (-1);
	i = str_chr(lang, '/');
	if (lang[i])
		return (-1);
	/*- convert to lower case (using lowerit() from libvpopmail) */
	lowerit(lang);
	/*- close previous language if still open */
	if (lang_fd != -1) {
		close(lang_fd);
		lang_fd = -1;
	}
	if (!langpath) {
		if (!(langpath = env_get(QMAILADMIN_TEMPLATEDIR)))
			langpath = HTMLLIBDIR;
	}
	if (!stralloc_copys(&langfile, langpath) ||
			!stralloc_catb(&langfile, "/lang/", 6) ||
			!stralloc_cats(&langfile, lang) ||
			!stralloc_0(&langfile))
		die_nomem();
	/*- do not open symbolic links */
	if (lstat(langfile.s, &mystat) == -1 || S_ISLNK(mystat.st_mode))
		return (-1);
	if ((lang_fd = open_read(langfile.s)) == -1)
		return (-1);
	return (lang_fd);
}

const char     *
get_html_text(const char *index)
{
	int             i;

	scan_int(index, &i);
	return (html_text[i]);
}

int
open_colortable()
{
	char           *tmpstr;

	if (!(tmpstr = env_get(QMAILADMIN_TEMPLATEDIR)))
		tmpstr = HTMLLIBDIR;

	if (!stralloc_copys(&TmpBuf, tmpstr) ||
			!stralloc_catb(&TmpBuf, "/html/colortable", 16) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((color_table = open_read(TmpBuf.s)) == -1)
		return (-1);
	return (0);
}

const char     *
get_color_text(const char *index)
{
	static stralloc line = {0};
	char            inbuf[1024];
	struct substdio ssin;
	int             i, match;

	if (color_table == -1)
		return ("");
	lseek(color_table, 0, SEEK_SET);
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, color_table, inbuf, sizeof(inbuf));
	ssin.p = 0;
	ssin.n = sizeof(inbuf);
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn1("get_color_text: read: ", &strerr_sys);
			close(color_table);
			color_table = -1;
			return ("");
		}
		if (!match || !line.len)
			break;
		i = str_chr(line.s, ' ');
		if (line.s[i])
			line.s[i] = 0;
		else
			continue;
		if (!str_diffn(line.s, index, i)) {
			return (line.s + i + 1);
		}
	}
	return ("");
}

/*
 * quota_to_bytes: used to convert user entered quota (given in MB)
 * back to bytes for vpasswd file
 * return value: 0 for success, 1 for failure
 */
int
quota_to_bytes(char returnval[], const char *quota)
{
	mdir_t          tmp;
	int             i;

	if (!quota)
		return 1;
	tmp = strtoll(quota, NULL, 10);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (tmp == LLONG_MIN || tmp == LLONG_MAX) {
#else
	if (errno == ERANGE) {
#endif
		str_copy(returnval, "");
		return 1;
	}
	tmp *= 1048576;
	returnval[i = fmt_double(returnval, tmp, 0)] = 0;
	returnval[i - 1] = 0; /*- remove . */
	return 0;
}

/*
 * quota_to_megabytes: used to convert vpasswd representation of quota
 * to number of megabytes.
 * return value: 0 for success, 1 for failure
 */
int
quota_to_megabytes(char returnval[], const char *quota)
{
	mdir_t          tmp;
	int             i;

	if (!quota)
		return 1;
	i = str_len(quota);
	tmp = strtoll(quota, NULL, 10);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (tmp == LLONG_MIN || tmp == LLONG_MAX) {
#else
	if (errno == ERANGE) {
#endif
		str_copy(returnval, "");
		return 1;
	}
	switch(quota[i - 1])
	{
	case 'm':
	case 'M':
		break;
	case 'k':
	case 'K':
		tmp /= 1024.0; /*- convert kilobytes to megabytes */
		break;
	default:
		tmp /= 1048576.0; /*- convert bytes to megabytes */
	}
	returnval[i = fmt_double(returnval, tmp, 2)] = 0;
	return 0;
}
void
print_user_index(const char *action, int colspan, const char *user, const char *dom, time_t mytime)
{
#ifdef USER_INDEX
	int             k;
	char            strnum[FMT_ULONG];

	out("<tr bgcolor=");
	out(get_color_text("000"));
	out(">");
	out("<td colspan=");
	strnum[fmt_int(strnum, colspan)] = 0;
	out(strnum);
	out(" align=\"center\">");
	out("<hr>");
	out("<b>");
	out(html_text[133]);
	out("</b> &nbsp; ");
	for (k = 0; k < 10; k++)
		printh("<a href=\"%s&searchuser=%d\">%d</a>\n", cgiurl(action), k, k);
	for (k = 'a'; k <= 'z'; k++)
		printh("<a href=\"%s&searchuser=%c\">%c</a>\n", cgiurl(action), k, k);
	out("</td>");
	out("</tr>\n");

	out("<tr bgcolor=");
	out(get_color_text("000"));
	out(">");
	out("<td colspan=");
	strnum[fmt_int(strnum, colspan)] = 0;
	out(strnum);
	out(">");
	out("<table border=0 cellpadding=3 cellspacing=0 width=\"100%\"><tr><td align=\"center\"><br>");
	out("<form method=\"get\" action=\"");
	out(CGIPATH);
	out("/com/");
	out(action);
	out("\">");
	printh("<input type=\"hidden\" name=\"user\" value=\"%H\">", user);
	printh("<input type=\"hidden\" name=\"dom\" value=\"%H\">", dom);
	out("<input type=\"hidden\" name=\"time\" value=\"");
	strnum[fmt_ulong(strnum, (unsigned long) mytime)] = 0;
	out(strnum);
	out("\">");
	printh("<input type=\"text\" name=\"searchuser\" value=\"%H\">&nbsp;", SearchUser.s);
	out("<input type=\"submit\" value=\"");
	out(html_text[204]);
	out("\">");
	out("</form>");
	out("</td></tr></table>");
	out("<hr>");
	out("</td></tr>\n");
	flush();
#endif
}

char           *
cgiurl(const char *action)
{
	int             len, plen;
	static stralloc url = {0};

	len = str_len(CGIPATH) + str_len(action) + Username.len + Domain.len + FMT_ULONG + 28;
	for (;;) {
		if (!stralloc_ready(&url, len))
			die_nomem();
		plen = snprinth(url.s, len, "%s/com/%s?user=%C&dom=%C&time=%d", CGIPATH, action,
			Username.s, Domain.s, mytime);
		if (plen < len) {
			url.len = plen;
			break;
		}
		len = plen + 28;
	}
	return (url.s);
}
