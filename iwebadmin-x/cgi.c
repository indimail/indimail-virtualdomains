/*
 * $Id: cgi.c,v 1.4 2020-11-01 23:15:39+05:30 Cprogrammer Exp mbhangui $
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <env.h>
#include <stralloc.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <alloc.h>
#include <strerr.h>
#endif
#include "indimail.h"
#include "indimail_compat.h"
#include "iwebadminx.h"
#include "common.h"

void
get_cgi()
{
	int             count, i, j, qslen = 0;
	char           *qs, *s;
	extern char    *TmpCGI;

	if ((qs = env_get("QUERY_STRING")))
		qslen = str_len(qs);
	if (!(s = env_get("CONTENT_LENGTH")))
		count = 0;
	else
		scan_int(s, &count);
	TmpCGI = alloc(count + qslen + 2);
	i = 0;
	do {
		if ((j = read(0, TmpCGI + i, count - i)) == -1) {
			strerr_warn1("get_cgi: ", &strerr_sys);
			alloc_free(TmpCGI);
			TmpCGI = (char *) 0;
			return;
		} else
		if (!j)
			break;
		i += j;
	} while (j > 0 && i < count);
	TmpCGI[i] = 0;
	s = TmpCGI + i;
	/*- append query string to end */
	if (qslen) {
		s += fmt_strn(s, "&", 1);
		s += fmt_strn(s, qs, qslen);
		*s++ = 0;
	}
	/*- fprintf(stderr, "getcgidump[%s]\n", TmpCGI); -*/
}

/*
 * source is encoded cgi parameters, name is "fieldname="
 * copies value of fieldname into dest
 */
int
GetValue(char *source, stralloc *dest, char *name)
{
	int             i, j, k, l, len;
	char           *s;

	dest->len = 0;
	if (!stralloc_ready(dest, 1) || !stralloc_0(dest))
		die_nomem();
	dest->len = 0;
	for (i = 0; source[i]; i++)
		if ((!i || source[i - 1] == '&') && str_start(source + i, name))
				break;
	if (source[i])
		i += str_len(name);
	else
		return (-1);
	dest->len = 0;
	/*- get the length */
	for (k = 0, j = i; source[j] != '&' && source[j]; ++k, ++j) {
		if (source[j] == '%') {
			if (source[j + 1] == '0' && source[j + 2] == 'D')
				--k;
			j += 2;
		}
	}
	if (!stralloc_ready(dest, k + 1)) {
		if (stralloc_copys(&StatusMessage, html_text[201]))
			stralloc_0(&StatusMessage);
		iclose();
		exit(0);
	}
	dest->len--;
	s = dest->s;
	for (len = k = 0, j = i; source[j] != '&' && source[j]; ++k, ++j) {
		if (source[j] == '%') {
			if (source[j + 1] == '0' && source[j + 2] == 'D')
				--k;
			else
			if (source[j + 1] == '0' && source[j + 2] == 'A') {
				s += fmt_strn(s, "\n", 1);
				len += 1;
			} else {
				l = (CGIValues[(int) source[j + 1]] << 4) + CGIValues[(int) source[j + 2]];
				s += fmt_strn(s, (char *) &l, 1);
				len += 1;
			}
			j += 2;
		} else
		if (source[j] == '+') {
			s += fmt_strn(s, " ", 1);
			len += 1;
		} else {
			s += fmt_strn(s, source + j, 1);
			len += 1;
		}
	}
	dest->len = len;
	*s++ = 0;
	while (isspace(dest->s[k])) {
		dest->len--;
		--k;
	}
	if (!stralloc_0(dest)) {
		if (stralloc_copys(&StatusMessage, html_text[201]))
			stralloc_0(&StatusMessage);
		iclose();
		exit(0);
	}
	dest->len--;
	/*- uncomment next line to dump cgi values to error log */
	/*- fprintf (stderr, "cgidump->[%s][%s][%d]\n", name, dest->s, dest->len); -*/
	return (0);
}
