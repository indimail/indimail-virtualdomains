/*
 * $Log: print_control.c,v $
 * Revision 1.1  2019-04-14 21:04:19+05:30  Cprogrammer
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
#include <getln.h>
#include <open.h>
#include <substdio.h>
#include <stralloc.h>
#include <error.h>
#include <strerr.h>
#include <fmt.h>
#endif
#include "common.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: print_control.c,v 1.1 2019-04-14 21:04:19+05:30 Cprogrammer Exp mbhangui $";
#endif

unsigned long
print_control(char *filename, char *domain, int max_users_per_level, int silent)
{
	char           *ptr;
	static stralloc line = {0};
	int             fd, match, users_per_level = 0;
	unsigned long   total = 0;
	char            inbuf[512], strnum[FMT_ULONG];
	struct substdio ssin;

	if ((fd = open_read(filename)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "print_control: open: ", filename, ": ");
		return (0);
	}
	users_per_level = max_users_per_level ? max_users_per_level : MAX_USERS_PER_LEVEL;
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("print_control: read: ", filename, ": ", &strerr_sys);
			break;
		}
		if (!match && line.len == 0)
			break;
		else {
			line.len--;
			line.s[line.len] = 0; /*- null terminate */
		}
		vread_dir_control(line.s, &vdir, domain);
		for (ptr = line.s; *ptr; ptr++)
			if (*ptr == '_')
				*ptr = '/';
		if (!silent) {
			out("print_control", "Dir Control     = ");
			out("print_control", line.s);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, vdir.cur_users)] = 0;
			out("print_control", "cur users       = ");
			out("print_control", strnum);
			out("print_control", "\n");

			out("print_control", "dir prefix      = ");
			out("print_control", vdir.the_dir);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, users_per_level)] = 0;
			out("print_control", "Users per level = ");
			out("print_control", strnum);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, vdir.level_cur)] = 0;
			out("print_control", "level_cur       = ");
			out("print_control", strnum);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, vdir.level_max)] = 0;
			out("print_control", "level_max       = ");
			out("print_control", strnum);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, vdir.level_index[0])] = 0;
			out("print_control", "level_index 0   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_index[1])] = 0;
			out("print_control", "            1   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_index[2])] = 0;
			out("print_control", "            2   = ");
			out("print_control", strnum);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, vdir.level_start[0])] = 0;
			out("print_control", "level_start 0   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_start[1])] = 0;
			out("print_control", "            1   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_start[2])] = 0;
			out("print_control", "            2   = ");
			out("print_control", strnum);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, vdir.level_end[0])] = 0;
			out("print_control", "level_end   0   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_end[1])] = 0;
			out("print_control", "            1   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_end[2])] = 0;
			out("print_control", "            2   = ");
			out("print_control", strnum);
			out("print_control", "\n");

			strnum[fmt_uint(strnum, vdir.level_mod[0])] = 0;
			out("print_control", "level_mod   0   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_mod[1])] = 0;
			out("print_control", "            1   = ");
			out("print_control", strnum);
			out("print_control", "\n");
			strnum[fmt_uint(strnum, vdir.level_mod[2])] = 0;
			out("print_control", "            2   = ");
			out("print_control", strnum);
			out("print_control", "\n\n");
		}
		total += vdir.cur_users;
	}
	close(fd);
	flush("print_control");
	return (total);
}
