/*
 * $Log: parse_quota.c,v $
 * Revision 1.1  2019-04-14 18:31:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define XOPEN_SOURCE = 600
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if !defined(LLONG_MIN) && !defined(LLONG_MAX)
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#endif
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: parse_quota.c,v 1.1 2019-04-14 18:31:08+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("parse_quota: out of memory", 0);
	_exit(111);
}

mdir_t
parse_quota(char *quota, mdir_t *count)
{
	static stralloc tmpbuf = {0};
	mdir_t          per_user_limit;
	int             i;

	if (!stralloc_copys(&tmpbuf, quota) || !stralloc_0(&tmpbuf))
		die_nomem();
	i = str_chr(tmpbuf.s, ',');
	if (tmpbuf.s[i]) {
		tmpbuf.s[i] = 0;
		if (count) {
			*count = strtoll(tmpbuf.s + i + 1, 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
			if (*count == LLONG_MIN || *count == LLONG_MAX)
#else
			if (errno == ERANGE)
#endif
				return (-1);
		}
	} else
	if (count)	
		*count = 0;
	if (!str_diffn(tmpbuf.s, "NOQUOTA", 8))
		return (0); /*- NOQUOTA */
	per_user_limit = strtoll(tmpbuf.s, 0, 0);
#if defined(LLONG_MIN) && defined(LLONG_MAX)
	if (per_user_limit == LLONG_MIN || per_user_limit == LLONG_MAX)
#else
	if (errno == ERANGE)
#endif
		return (-1);
	for (i = 0; quota[i] != 0; ++i) {
		if (quota[i] == 'k' || quota[i] == 'K') {
			per_user_limit = per_user_limit * 1024;
			break;
		}
		if (quota[i] == 'm' || quota[i] == 'M') {
			per_user_limit = per_user_limit * 1048576;
			break;
		}
		if (quota[i] == 'g' || quota[i] == 'G') {
			per_user_limit = per_user_limit * 1073741824;
			break;
		}
	}
	return (per_user_limit);
}
