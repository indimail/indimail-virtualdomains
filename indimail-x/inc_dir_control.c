/*
 * $Log: inc_dir_control.c,v $
 * Revision 1.1  2019-04-18 08:23:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_QMAIL
#include <str.h>
#endif
#include "variables.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: inc_dir_control.c,v 1.1 2019-04-18 08:23:40+05:30 Cprogrammer Exp mbhangui $";
#endif

int
inc_dir_control(vdir_type *vdir, int max_users_per_level)
{
	int             users_per_level, len;

	++vdir->cur_users;
	users_per_level = max_users_per_level ? max_users_per_level : MAX_USERS_PER_LEVEL;
	if (vdir->cur_users % users_per_level == 0) {
		if (!*(vdir->the_dir)) {
			vdir->the_dir[0] = dirlist[vdir->level_start[0]];
			vdir->the_dir[1] = 0;
			return (0);
		}

		if (vdir->level_index[vdir->level_cur] == vdir->level_end[vdir->level_cur]) {
			switch (vdir->level_cur)
			{
			case 0:
				inc_dir(vdir, vdir->level_cur);
				++vdir->level_cur;
				if ((len = str_len(vdir->the_dir)) < MAX_DIR_NAME)
					str_copy(vdir->the_dir + len, "/");
				break;
			case 1:
				if (vdir->level_index[0] == vdir->level_end[0] && vdir->level_index[1] == vdir->level_end[1]) {
					inc_dir(vdir, vdir->level_cur);
					++vdir->level_cur;
					if ((len = str_len(vdir->the_dir)) < MAX_DIR_NAME)
						str_copy(vdir->the_dir + len, "/");
				}
				break;
			}
		}
		inc_dir(vdir, vdir->level_cur);
	}
	return (0);
}
