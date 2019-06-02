/*
 * $Log: addressToken.c,v $
 * Revision 1.1  2019-04-18 08:37:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: addressToken.c,v 1.1 2019-04-18 08:37:41+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#endif

static int      idx;
static stralloc email = {0};

static void
die_nomem()
{
	strerr_warn1("addrToken: out of memory", 0);
	_exit(111);
}

char           *
addressToken(char *email_list, int *email_len)
{
	int             skip, len;
	char           *ptr, *cptr;

	ptr = email_list + idx;
	email.len = skip = 0;
	if (email_len)
		*email_len = email.len;
	if (!*ptr)
		return ((char *) 0);
	for (; *ptr && *ptr != ','; ptr++, idx++) {
		if (isspace((int) *ptr) || *ptr == '<' || *ptr == '>')
			continue;
		if (!skip && *ptr == '\"') {
			skip = 1;
			continue;
		}
		if (!skip) {
			cptr = ptr;
			for (len = 0; *ptr && *ptr != ',' && *ptr != '\"'; ptr++, idx++, len++);
			if (!stralloc_copyb(&email, cptr, len))
				die_nomem();
			if (*ptr == ',' || *ptr == '\"')
				break;
		}
		if (skip && *ptr == '\"')
			skip = 0;
	}
	if (!stralloc_0(&email))
		die_nomem();
	email.len--;
	if (email_len)
		*email_len = email.len;
	if (*ptr == ',')
		idx++;
	return (email.s);
}

void
rewindAddrToken()
{
	idx = 0;
	email.len = 0;
}
#endif
