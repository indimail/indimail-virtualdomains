/*
 * $Id: print_control.c,v 1.4 2025-05-13 20:02:10+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <getln.h>
#include <open.h>
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <error.h>
#include <strerr.h>
#endif
#include "common.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: print_control.c,v 1.4 2025-05-13 20:02:10+05:30 Cprogrammer Exp mbhangui $";
#endif

unsigned long
print_control(char *filename, char *domain, int max_users_per_level, int silent)
{
	char           *ptr;
	static stralloc line = {0};
	int             fd, match, users_per_level = 0;
	unsigned long   total = 0;
	char            inbuf[512];
	struct substdio ssin;

	if ((fd = open_read(filename)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "print_control: open: ", filename, ": ");
		return (0);
	}
	users_per_level = max_users_per_level ? max_users_per_level : MAX_USERS_PER_LEVEL;
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("print_control: read: ", filename, ": ", &strerr_sys);
			break;
		}
		if (!line.len)
			break;
		if (!line.len)
			strerr_warn2("print_control: incomplete line: ", filename, 0);
		if (match) {
			line.len--;
			if (!line.len)
				strerr_warn2("print_control: incomplete line: ", filename, 0);
			line.s[line.len] = 0; /*- null terminate */
		} else {
			if (!stralloc_0(&line))
				strerr_die1sys(111, "print_control: out of memory");
			line.len--;
		}
		vread_dir_control(line.s, &vdir, domain);
		for (ptr = line.s; *ptr; ptr++)
			if (*ptr == '_')
				*ptr = '/';
		if (!silent) {
			subprintfe(subfdout, "print_control", "Dir Control     = %s\n", line.s);
			subprintfe(subfdout, "print_control", "cur users       = %lu\n", vdir.cur_users);
			subprintfe(subfdout, "print_control", "dir prefix      = %s\n", vdir.the_dir);
			subprintfe(subfdout, "print_control", "Users per level = %d\n", users_per_level);
			subprintfe(subfdout, "print_control", "level_cur       = %d\n", vdir.level_cur);
			subprintfe(subfdout, "print_control", "level_max       = %d\n", vdir.level_max);
			subprintfe(subfdout, "print_control", "level_index 0   = %d\n", vdir.level_index[0]);
			subprintfe(subfdout, "print_control", "            1   = %d\n", vdir.level_index[1]);
			subprintfe(subfdout, "print_control", "            2   = %d\n", vdir.level_index[2]);
			subprintfe(subfdout, "print_control", "level_start 0   = %d\n", vdir.level_start[0]);
			subprintfe(subfdout, "print_control", "            1   = %d\n", vdir.level_start[1]);
			subprintfe(subfdout, "print_control", "            2   = %d\n", vdir.level_start[2]);
			subprintfe(subfdout, "print_control", "level_end   0   = %d\n", vdir.level_end[0]);
			subprintfe(subfdout, "print_control", "            1   = %d\n", vdir.level_end[1]);
			subprintfe(subfdout, "print_control", "            2   = %d\n", vdir.level_end[2]);
			subprintfe(subfdout, "print_control", "level_mod   0   = %d\n", vdir.level_mod[0]);
			subprintfe(subfdout, "print_control", "            1   = %d\n", vdir.level_mod[1]);
			subprintfe(subfdout, "print_control", "            2   = %d\n", vdir.level_mod[2]);
			out("print_control", "\n");
		}
		total += vdir.cur_users;
	}
	close(fd);
	flush("print_control");
	return (total);
}
/*
 * $Log: print_control.c,v $
 * Revision 1.4  2025-05-13 20:02:10+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.3  2023-03-20 10:15:39+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.1  2019-04-14 21:04:19+05:30  Cprogrammer
 * Initial revision
 *
 */
