/*
 * $Id: user_over_quota.c,v 1.6 2025-05-13 20:35:58+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef linux
#include <sys/file.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <error.h>
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#include <getln.h>
#include <substdio.h>
#include <env.h>
#endif
#include "indimail.h"
#include "parse_quota.h"
#include "recalc_quota.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: user_over_quota.c,v 1.6 2025-05-13 20:35:58+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("user_over_quota: out of memory", 0);
	_exit(111);
}

/*
 * Check if the user is over quota
 * Do all quota recalculation needed
 * Return 1 if user is over quota
 * Return 0 if user is not over quota
 */
int
user_over_quota(char *Maildir, char *quota, int cur_msgsize)
{
#ifdef USE_MAILDIRQUOTA
	mdir_t          mail_count_limit, cur_mailbox_count;
	static stralloc tmpbuf = {0}, line = {0};
	char           *tmpQuota;
	int             fd, match;
	struct substdio ssin;
	static char     inbuf[4096];
#else
	int             i;
#endif
	static stralloc maildir = {0};
	char           *ptr;
	mdir_t          mail_size_limit, cur_mailbox_size;

	if (!stralloc_copys(&maildir, Maildir) || !stralloc_0(&maildir))
		die_nomem();
	maildir.len--;
	if ((ptr = str_str(maildir.s, "/Maildir/")) && *(ptr + 9)) {
		*(ptr + 9) = 0;
		maildir.len = str_len(maildir.s);
	}
	if (maildir.s[maildir.len - 1] == '/')
		maildir.len--;
#ifdef USE_MAILDIRQUOTA
	if (!quota || !*quota) {
		if (!stralloc_copy(&tmpbuf, &maildir) ||
				!stralloc_catb(&tmpbuf, "/maildirsize", 12) ||
				!stralloc_0(&maildir))
			die_nomem();
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno == error_noent)
				return (0);
			return (-1);
		}
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("user_over_quota: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		close(fd);
		if (!line.len) {
			strerr_warn3("user_over_quota: ", tmpbuf.s, ": incomplete line", 0);
			close(fd);
			return (-1);
		}
		if (match) {
			line.len--;
			if (!line.len) {
				strerr_warn3("user_over_quota: ", tmpbuf.s, ": incomplete line", 0);
				close(fd);
				return (-1);
			}
			line.s[line.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		tmpQuota = line.s;
	} else
		tmpQuota = quota;
	if (!str_diffn(tmpQuota, "0S", 2))
		return (0);
	if ((mail_size_limit = parse_quota(tmpQuota, &mail_count_limit)) == -1)
		return (-1);
#else
	if (!quota || !*quota)
		return (0);
	/*
	 * translate the quota to a number
	 */
	scan_ulong(quota, (unsigned long *) &mail_size_limit);
	for (i = 0; quota[i] != 0; ++i) {
		if (quota[i] == 'k' || quota[i] == 'K') {
			mail_size_limit = mail_size_limit * 1024;
			break;
		}
		if (quota[i] == 'm' || quota[i] == 'M') {
			mail_size_limit = mail_size_limit * 1048576;
			break;
		}
		if (quota[i] == 'g' || quota[i] == 'G') {
			mail_size_limit = mail_size_limit * 1073741824;
			break;
		}
	}
#endif
	/*
	 * Get their current total from maildirsize
	 */
#ifdef USE_MAILDIRQUOTA
	if ((CurBytes = cur_mailbox_size = recalc_quota(maildir.s, &cur_mailbox_count,
		mail_size_limit, mail_count_limit, 0)) == -1)
		return (-1);
	CurCount = cur_mailbox_count;
	if (cur_mailbox_size + cur_msgsize > mail_size_limit) /*- if over quota recalculate quota */
	{
		if (env_get("FAST_QUOTA"))
			return (1);
		if ((CurBytes = cur_mailbox_size = recalc_quota(maildir.s, &cur_mailbox_count,
					mail_size_limit, mail_count_limit, 2)) == -1)
			return (-1);
		if (cur_mailbox_size + cur_msgsize > mail_size_limit)
			return (1);
	}
	if (mail_count_limit && (cur_mailbox_count + 1 > mail_count_limit))
		return (1);
#else
	if ((CurBytes = cur_mailbox_size = recalc_quota(maildir.s, 0)) == -1)
		return (-1);
	/*- Check if this email would bring them over quota */
	if (cur_mailbox_size + cur_msgsize > mail_size_limit) {
		/*-
		 * recalculate their quota since they might have
		 * deleted email
		 */
		if ((CurBytes = cur_mailbox_size = recalc_quota(maildir.s, 2)) == -1)
			return (-1);
		if (cur_mailbox_size + cur_msgsize > mail_size_limit)
			return (1);
	}
#endif
	return (0);
}
/*
 * $Log: user_over_quota.c,v $
 * Revision 1.6  2025-05-13 20:35:58+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.5  2023-03-20 10:20:48+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.4  2019-07-26 09:41:45+05:30  Cprogrammer
 * added FAST_QUOTA env variable to avoid costly disk read for quota calculations
 *
 * Revision 1.3  2019-04-22 23:16:08+05:30  Cprogrammer
 * replaced atol() with scan_ulong()
 *
 * Revision 1.2  2019-04-21 16:14:36+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:33:52+05:30  Cprogrammer
 * Initial revision
 *
 */
