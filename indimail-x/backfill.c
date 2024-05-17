/*
 * $Log: backfill.c,v $
 * Revision 1.4  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2023-03-20 09:48:42+05:30  Cprogrammer
 * documented backfill code
 *
 * Revision 1.2  2019-07-04 09:19:02+05:30  Cprogrammer
 * initialize filename by using stralloc_copys insteadd of stralloc_cats
 *
 * Revision 1.1  2019-04-18 08:25:25+05:30  Cprogrammer
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
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <open.h>
#include <getln.h>
#include <substdio.h>
#include <strerr.h>
#include <error.h>
#include <str.h>
#endif
#include "get_assign.h"
#include "dir_control.h"
#include "remove_line.h"
#include "GetPrefix.h"
#include "dblock.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: backfill.c,v 1.4 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("backfill: out of memory", 0);
	_exit(111);
}

char           *
backfill(const char *username, const char *domain, const char *path, int operation)
{
	char           *filesys_prefix, *ptr = (char *) 0;
	static stralloc filename = {0}, line = {0};
	int             match, len, fd;
	struct substdio ssin;
	char            inbuf[512];
	uid_t           uid;
	gid_t           gid;

	if (!domain || !*domain)
		return ((char *) 0);
	if (!(ptr = get_assign(domain, 0, &uid, &gid))) {
		strerr_warn2(domain, ": No such domain", 0);
		return ((char *) 0);
	}
	if (!(filesys_prefix = GetPrefix(username, path)))
		return ((char *) 0);
	/*
	 * open
	 * /var/indimail/domains/example.com/._home_A2E
	 * /var/indimail/domains/example.com/._home_F2K
	 * /var/indimail/domains/example.com/._home_L2P
	 * /var/indimail/domains/example.com/._home_Q2S
	 * /var/indimail/domains/example.com/._home_T2Z
	 */
	if (!stralloc_copys(&filename, ptr) ||
			!stralloc_catb(&filename, "/.", 2) ||
			!stralloc_cats(&filename, filesys_prefix) ||
			!stralloc_0(&filename))
		die_nomem();
	if (operation == 1) {/*- add */
		if ((fd = open_read(filename.s)) == -1) {
			if (errno != error_noent)
				strerr_warn3("backfill: ", filename.s, ": ", &strerr_sys);
			return ((char *) 0);
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("backfill: read: ", filename.s, ": ", &strerr_sys);
				close(fd);
				return ((char *) 0);
			}
			if (!line.len) {
				close(fd);
				strerr_warn3("backfill: ", filename.s, "incomplete line", 0);
				return ((char *) 0);
			}
			if (match) {
				line.len--;
				if (!line.len) {
					close(fd);
					strerr_warn3("backfill: ", filename.s, "incomplete line", 0);
					return ((char *) 0);
				}
				line.s[line.len] = 0;
			} else {
				if (!stralloc_0(&line))
					die_nomem();
				line.len--;
			}
			break;
		}
		close(fd);
		if (remove_line(line.s, filename.s, 1, INDIMAIL_QMAIL_MODE) == 1) {
			/*- backfill an existing entry and return original directory */
			vread_dir_control(filesys_prefix, &vdir, domain);
			if (vdir.cur_users)
				++vdir.cur_users;
			vwrite_dir_control(filesys_prefix, &vdir, domain, uid, gid);
			return (line.s);
		}
	} else
	if (operation == 2) {/*- delete */
		/* /home/mail/A2E/example.com */
		if (!stralloc_copys(&line, path) || !stralloc_0(&line))
			die_nomem();
		if ((ptr = str_str(line.s, username))) {
			if (ptr != line.s)
				ptr--;
			if (*ptr == '/')
				*ptr = 0;
		}
		if ((ptr = str_str(line.s, domain))) {
			ptr += str_len(domain);
			if (*ptr == '/')
				ptr++;
			if (ptr && *ptr) { /* open _home_A2E */
				/*-
				 * append entry to existing list
				 * to be backfilled later
				 */
#ifdef FILE_LOCKING
				if ((fd = getDbLock(filename.s, 1)) == -1) {
					strerr_warn3("backfill: getDbLock: ", filename.s, ": ", &strerr_sys);
					return ((char *) 0);
				}
#endif
				if ((fd = open_append(filename.s)) == -1) {
					if (errno != error_noent)
						strerr_warn3("backfill: ", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
					delDbLock(fd, filename.s, 1);
#endif
					return ((char *) 0);
				}
				len = str_len(ptr);
				if (write(fd, ptr, len) != len) {
					strerr_warn3("backfill: write", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
					delDbLock(fd, filename.s, 1);
#endif
					return ((char *) 0);
				}
				if (close(fd) == -1) {
					strerr_warn3("backfill: write", filename.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
					delDbLock(fd, filename.s, 1);
#endif
					return ((char *) 0);
				}
#ifdef FILE_LOCKING
				delDbLock(fd, filename.s, 1);
#endif
				return (ptr);
			}
		}
	} /*- if (operation == 2) */
	return ((char *) 0);
}
