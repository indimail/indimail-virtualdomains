/*
 * $Log: vdelfiles.c,v $
 * Revision 1.3  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.2  2020-10-18 07:54:49+05:30  Cprogrammer
 * use alloc() instead of alloc_re()
 *
 * Revision 1.1  2019-04-18 08:14:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <alloc.h>
#include <strerr.h>
#include <error.h>
#include <fmt.h>
#include <subfd.h>
#endif
#include "common.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdelfiles.c,v 1.3 2023-01-22 10:40:03+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * vdelfiles : delete a directory tree
 *
 * input: directory to start the deletion
 * output: 
 *         0 on success
 *        -1 on failer
 */
int
vdelfiles(char *dir, char *user, char *domain)
{
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	int             dir_len, file_len, old_size, i;
	char           *file_name, *s;
	char            strnum[FMT_ULONG];

	if (!str_diffn(dir, "/", 2) || !str_diffn(dir, "/usr", 5) ||
			!str_diffn(dir, "/var", 5) || !str_diffn(dir, "/mail", 6)) /* safety */
		return (-1);
	if (user && *user) {
		if (!str_str(dir, user))
			return (-1);
	}
	if (domain && *domain) {
		if (!str_str(dir, domain))
			return (-1);
	}
	if (lstat(dir, &statbuf) == -1) {
		if (errno == error_noent)
			return (0);
		return (-1);
	}
	if (!S_ISDIR(statbuf.st_mode)) {
		if (verbose) {
			subprintfe(subfdout, "vdelfiles", "Removing File %s\n", dir);
			flush("vdelfiles");
		}
		if (unlink(dir)) {
			strerr_warn3("vdelfiles: unlink: ", dir, ": ", &strerr_sys);
			return (-1);
		}
		return (0);
	}
	if (!(entry = opendir(dir))) {
		strerr_warn3("vdelfiles: opendir: ", dir, ": ", &strerr_sys);
		return (-1);
	}
	dir_len = str_len(dir);
	for (file_name = 0, old_size = 0;;) {
		if (!(dp = readdir(entry)))
			break;
		if (!str_diffn(dp->d_name, ".", 2) || !str_diffn(dp->d_name, "..", 3))
			continue;
		file_len = str_len(dp->d_name);
		strnum[i = fmt_uint(strnum, dir_len + file_len + 2)] = 0;
		if (dir_len + file_len + 2 > old_size && old_size)
			alloc_free(file_name);
		if (dir_len + file_len + 2 > old_size && !(file_name = alloc(dir_len + file_len + 2))) {
			strerr_warn3("vdelfiles: alloc: ", strnum, " bytes", &strerr_sys);
			closedir(entry);
			return (-1);
		}
		if (dir_len + file_len + 2 > old_size)
			old_size = dir_len + file_len + 2;
		s = file_name;
		s += fmt_strn(s, dir, dir_len);
		s += fmt_strn(s, "/", 1);
		s += fmt_strn(s, dp->d_name, file_len);
		*s++ = 0;
		if (lstat(file_name, &statbuf)) {
			strerr_warn3("vdelfiles: lstat: ", file_name, ": ", &strerr_sys);
			continue;
		}
		if (!S_ISDIR(statbuf.st_mode)) {
			if (verbose) {
				subprintfe(subfdout, "vdelfiles", "Removing File %s\n", file_name);
				flush("vdelfiles");
			}
			if (unlink(file_name) == -1)
				strerr_warn3("vdelfiles: unlink: ", file_name, ": ", &strerr_sys);
			continue;
		} 
		if (vdelfiles(file_name, user, domain) == -1) {
			alloc_free(file_name);
			closedir(entry);
			return (-1);
		}
	}
	alloc_free(file_name);
	closedir(entry);
	if (verbose) {
		subprintfe(subfdout, "vdelfiles", "Removing Dir %s\n", dir);
		flush("vdelfiles");
	}
	if (rmdir(dir)) {
		strerr_warn3("vdelfiles: rmdir: ", dir, ": ", &strerr_sys);
		return (-1);
	}
	return (0);
}
