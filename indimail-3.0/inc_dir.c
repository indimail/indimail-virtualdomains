/*
 * $Log: inc_dir.c,v $
 * Revision 1.1  2019-04-18 08:23:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: inc_dir.c,v 1.1 2019-04-18 08:23:34+05:30 Cprogrammer Exp mbhangui $";
#endif

static char next_char(char, int, int);

char           *
inc_dir(vdir_type *vdir, int in_level)
{
	if (vdir->the_dir[vdir->level_mod[in_level]] == dirlist[vdir->level_end[in_level]]) {
		vdir->the_dir[vdir->level_mod[in_level]] = dirlist[vdir->level_start[in_level]];
		vdir->level_index[in_level] = vdir->level_start[in_level];
		if (in_level > 0)
			inc_dir(vdir, in_level - 1);
	} else {
		vdir->the_dir[vdir->level_mod[in_level]] =
			next_char(vdir->the_dir[vdir->level_mod[in_level]], vdir->level_start[in_level], vdir->level_end[in_level]);
		++vdir->level_index[in_level];
	}
	return (vdir->the_dir);
}

static char
next_char(char in_char, int in_start, int in_end)
{
	int             i;

	for (i = in_start; i < in_end + 1 && dirlist[i] != in_char; ++i);
	++i;
	if (i >= in_end + 1)
		i = in_start;
	return (dirlist[i]);
}
