/*
 * $Log: count_dir.c,v $
 * Revision 1.2  2020-10-18 11:29:34+05:30  Cprogrammer
 * use alloc() instead of alloc_re()
 *
 * Revision 1.1  2019-04-18 08:22:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include "indimail.h"
#include <stdlib.h>
#include <sys/stat.h>
#ifdef HAVE_QMAIL
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <str.h>
#include <alloc.h>
#include <strerr.h>
#endif
#include "skip_system_files.h"

#ifndef	lint
static char     sccsid[] = "$Id: count_dir.c,v 1.2 2020-10-18 11:29:34+05:30 Cprogrammer Exp mbhangui $";
#endif

mdir_t count_dir(char *dir_name, mdir_t *mailcount)
{
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	mdir_t         file_size, tmp, oldlen, newlen, dirname_len, filename_len, count;
	int             is_trash, i;
	char           *tmpstr, *ptr, *include_trash;
	char            strnum[FMT_ULONG];

	if (!dir_name || !*dir_name)
		return (0);
	if (!(entry = opendir(dir_name)))
		return (0);
	if ((include_trash = (char *) env_get("INCLUDE_TRASH")))
		is_trash = 0;
	else {
		if (str_str(dir_name, "/Maildir/.Trash"))
			is_trash = 1;
		else
			is_trash = 0;
	}
	if (mailcount)
		*mailcount = 0;
	dirname_len = str_len(dir_name);
	for (tmpstr = 0, file_size = oldlen = newlen = 0;;) {
		if (!(dp = readdir(entry)))
			break;
		if (!str_diffn(dp->d_name, ".", 2) || !str_diffn(dp->d_name, "..", 3))
			continue;
		else
		if (skip_system_files(dp->d_name))
			continue;
		filename_len = str_len(dp->d_name);
		newlen = sizeof(char) * (filename_len + dirname_len + 2);
		if (newlen > oldlen && oldlen)
			alloc_free(tmpstr);
		if (newlen > oldlen && !(tmpstr = alloc(newlen))) {
			strnum[i = fmt_uint(strnum, newlen)] = 0;
			strerr_warn3("count_dir: alloc: ", strnum, " bytes: ", &strerr_sys);
			closedir(entry);
			return (-1);
		}
		if (newlen > oldlen)
			oldlen = newlen;
		ptr = tmpstr;
		ptr += fmt_strn(ptr, dir_name, dirname_len);
		ptr += fmt_strn(ptr, "/", 1);
		ptr += fmt_strn(ptr, dp->d_name, filename_len);
		*ptr++ = 0;
		if ((ptr = str_str(dp->d_name, ",S=")) != NULL) {
			ptr += 3;
			scan_ulong(ptr, (unsigned long *) &tmp);
			file_size += tmp;
			if (mailcount)
				(*mailcount)++;
		} else
		if (!stat(tmpstr, &statbuf)) {
			if (S_ISDIR(statbuf.st_mode)) {
				file_size += count_dir(tmpstr, &count);
				if (mailcount)
					*mailcount += count;
			} else {
				if (!include_trash && (*(dp->d_name + filename_len - 1) == 'T' || is_trash))
					continue;
				if (mailcount)
					(*mailcount)++;
				file_size += statbuf.st_size;
			}
		}
	} /*- for (tmpstr = 0, file_size = oldlen = newlen = 0;;) { */
	closedir(entry);
	if (tmpstr)
		alloc_free(tmpstr);
	return (file_size);
}
