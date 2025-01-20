/*
 * $Log: valiasinfo.c,v $
 * Revision 1.7  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2023-03-20 10:32:54+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.5  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.4  2021-07-08 11:46:52+05:30  Cprogrammer
 * removed QMAILDIR setting through env variable
 *
 * Revision 1.3  2020-04-01 18:58:29+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 14:23:56+05:30  Cprogrammer
 * fixed directory length
 * added missing new line
 *
 * Revision 1.1  2019-04-15 12:04:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <error.h>
#include <open.h>
#include <getln.h>
#include <substdio.h>
#include <subfd.h>
#include <getEnvConfig.h>
#endif
#include "common.h"
#include "get_assign.h"
#include "sql_getpw.h"
#include "valias_select.h"

#ifndef	lint
static char     sccsid[] = "$Id: valiasinfo.c,v 1.7 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("valiasinfo: out of memory", 0);
	_exit(111);
}

int
valiasinfo(const char *user, const char *domain)
{
	int             flag1, fd, match;
	static stralloc tmpbuf = {0}, Dir = {0}, line = {0};
	struct passwd  *pw;
	char           *tmpalias, *ptr;
	struct substdio ssin;
	char            inbuf[4096];
#ifdef VALIAS
	int             flag2;
#endif
	uid_t           uid;
	gid_t           gid;

	if (!domain || !*domain) {
		if (!stralloc_copys(&Dir, QMAILDIR) ||
				!stralloc_catb(&Dir, "/alias", 6) ||
				!stralloc_0(&Dir))
			die_nomem();
		Dir.len--;
	} else
	if (!get_assign(domain, &Dir, &uid, &gid))
		Dir.len = 0;
	if (Dir.len) {
		if (!stralloc_copy(&tmpbuf, &Dir) ||
				!stralloc_catb(&tmpbuf, "/.qmail-", 8) ||
				!stralloc_cats(&tmpbuf, user) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		/* replace all dots with ':' */
		for (ptr = tmpbuf.s + Dir.len + 8; *ptr; ptr++) {
			if (*ptr == '.')
				*ptr = ':';
		}
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno != error_noent) {
				strerr_warn3("valiasinfo: ", tmpbuf.s, ": ", &strerr_sys);
				return (-1);
			} else
				flag1 = 0;
		}
		if (fd != -1) {
			substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
			for (flag1 = 0;;) {
				if (getln(&ssin, &line, &match, '\n') == -1) {
					strerr_warn3("valiasinfo: read: ", tmpbuf.s, ": ", &strerr_sys);
					break;
				}
				if (!line.len)
					break;
				if (match) {
					line.len--;
					if (!line.len) {
						strerr_warn3("valiasinfo", tmpbuf.s, ": incomplete line", 0);
						continue;
					}
					line.s[line.len] = 0;
				} else {
					if (!stralloc_0(&line))
						die_nomem();
					line.len--;
				}
				match = str_chr(line.s, '#');
				if (line.s[match])
					line.s[match] = 0;
				for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
				if (!*ptr)
					continue;
				subprintfe(subfdout, "valiasinfo", !flag1++ ? "Forwarding    : %s\n" : "              : %s\n",
						*ptr == '&' ? ptr + 1 : ptr);
			}
			close(fd);
			if (!flag1++)
				subprintfe(subfdout, "valiasinfo", "Forwarding    : %s/Maildir/\n", Dir.s);
			flush("valiasinfo");
		}
		if ((pw = sql_getpw(user, domain)) != (struct passwd *) 0) {
			if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
					!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			if ((fd = open_read(tmpbuf.s)) == -1) {
				if (errno != error_noent) {
					strerr_warn3("valiasinfo: ", tmpbuf.s, ": ", &strerr_sys);
					return (-1);
				}
			}
			if (fd != -1) {
				substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
				for (;;) {
					if (getln(&ssin, &line, &match, '\n') == -1) {
						strerr_warn3("valiasinfo: read: ", tmpbuf.s, ": ", &strerr_sys);
						break;
					}
					if (!line.len)
						break;
					if (match) {
						line.len--;
						if (!line.len) {
							strerr_warn3("valiasinfo", tmpbuf.s, ": incomplete line", 0);
							continue;
						}
						line.s[line.len] = 0;
					} else {
						if (!stralloc_0(&line))
							die_nomem();
						line.len--;
					}
					match = str_chr(line.s, '#');
					if (line.s[match])
						line.s[match] = 0;
					for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
					if (!*ptr)
						continue;
					subprintfe(subfdout, "valiasinfo", !flag1++ ? "Forwarding    : %s\n" : "              : %s\n",
							*ptr == '&' ? ptr + 1 : ptr);
				}
				flush("valiasinfo");
				close(fd);
			}
		}
	} else
		flag1 = 0;
#ifdef VALIAS
	for (flag2 = 0;;) {
		if (!(tmpalias = valias_select(user, domain)))
			break;
		if (*tmpalias == '&')
			tmpalias++;
		subprintfe(subfdout, "valiasinfo", !flag2++ ? "Forwarding    : %s\n" : "              : %s\n", tmpalias);
	}
	flush("valiasinfo");
	return(flag1 + flag2);
#else
	return(flag1);
#endif
}
