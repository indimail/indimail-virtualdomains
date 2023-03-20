/*
 * $Log: maildir_to_email.c,v $
 * Revision 1.4  2023-03-20 10:13:16+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.3  2020-04-01 18:56:55+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-04-21 16:14:02+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:27:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <str.h>
#include <strerr.h>
#include <error.h>
#include <getEnvConfig.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: maildir_to_email.c,v 1.4 2023-03-20 10:13:16+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("maildir_to_email: out of memory", 0);
	_exit(111);
}

char           *
maildir_to_email(char *maildir, char *domain)
{
	static stralloc email = {0}, line = {0}, tmpbuf = {0};
	char           *ptr, *cptr, *base_path;
	int             fd, i, len, match;
	struct substdio ssin;
	char            inbuf[512];

	len = str_len(maildir);
	if (maildir[len - 1] == '/')
		len--;
	if (!stralloc_copyb(&tmpbuf, maildir, len) ||
			!stralloc_catb(&tmpbuf, "/email", 6) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "maildir_to_email: open: ", tmpbuf.s, ": ");
	}
	if (fd != -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &email, &match, '\n') == -1) {
			strerr_warn3("maildir_to_email: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return ((char *) 0);
		}
		close(fd);
		if (!email.len) {
			strerr_warn2("maildir_to_email: incomplete line: ", tmpbuf.s, 0);
			return ((char *) 0);
		}
		if (match) {
			email.len--;
			if (!email.len) {
				strerr_warn2("maildir_to_email: incomplete line: ", tmpbuf.s, 0);
				return ((char *) 0);
			}
			email.s[email.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		return (email.s);
	}

	if (!(ptr = str_str(maildir, "/Maildir")))
		return ((char *) 0);
	for (ptr--; *ptr && ptr != maildir && *ptr != '/'; ptr--);
	if (ptr == maildir || !*ptr)
		return ((char *) 0);
	cptr = ptr + 1;

	for (i = 0, ptr++; *ptr && *ptr != '/'; i++, ptr++);
	if (!stralloc_copyb(&email, cptr, i))
		die_nomem();
	if(domain && *domain) {
		if (!stralloc_append(&email, "@") ||
				!stralloc_cats(&email, domain) ||
				!stralloc_0(&email))
			die_nomem();
		return (email.s);
	} else {
		if (!stralloc_copyb(&tmpbuf, maildir, len) ||
				!stralloc_catb(&tmpbuf, "/domain", 7) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno != error_noent)
				strerr_die3sys(111, "maildir_to_email: open: ", tmpbuf.s, ": ");
		}
		if (fd != -1) {
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("maildir_to_email: read: ", tmpbuf.s, ": ", &strerr_sys);
				close(fd);
				return ((char *) 0);
			}
			close(fd);
			if (!line.len) {
				strerr_warn2("maildir_to_email: incomplete line: ", tmpbuf.s, 0);
				return ((char *) 0);
			}
			if (match) {
				line.len--;
				if (!line.len) {
					strerr_warn2("maildir_to_email: incomplete line: ", tmpbuf.s, 0);
					return ((char *) 0);
				}
				line.s[line.len] = 0; /*- remove newline */
			} else {
				if (!stralloc_0(&line))
					die_nomem();
				line.len--;
			}
			if (!stralloc_append(&email, "@") ||
					!stralloc_cat(&email, &line) ||
					!stralloc_0(&email))
				die_nomem();
			return (email.s);
		}
		getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
		ptr = maildir + str_len(base_path);
		for (ptr++; *ptr && *ptr != '/'; ptr++);
		cptr = ptr + 1;
		for (i = 0, ptr++; *ptr && *ptr != '/'; i++, ptr++);
		if (!stralloc_append(&email, "@") ||
				!stralloc_catb(&email, cptr, i) ||
				!stralloc_0(&email))
			die_nomem();
		return (email.s);
	}
}
