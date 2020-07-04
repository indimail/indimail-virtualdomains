/*
 * $Log: MoveFile.c,v $
 * Revision 1.2  2020-07-04 22:54:01+05:30  Cprogrammer
 * replaced utime() with utimes()
 *
 * Revision 1.1  2019-04-18 08:36:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <error.h>
#endif
#include "r_mkdir.h"
#include "fappend.h"

#ifndef	lint
static char     sccsid[] = "$Id: MoveFile.c,v 1.2 2020-07-04 22:54:01+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("MoveFile: out of memory", 0);
	_exit(111);
}

int
MoveFile(char *src_dir, char *dest_dir)
{
	DIR            *entry;
	struct dirent  *dp;
	struct timeval  ubuf1[2] = {0}, ubuf2[2] = {0};
	struct stat     statbuf;
	static stralloc tmpbuf = {0}, nsrc_dir = {0}, ndest_dir = {0};
	int             status;

	if (stat(src_dir, &statbuf)) {
		strerr_warn3("MoveFile: stat: ", src_dir, ": ", &strerr_sys);
		return (-1);
	} else
	if (!(entry = opendir(src_dir))) {
		strerr_warn3("MoveFile: opendir: ", src_dir, ": ", &strerr_sys);
		return (-1);
	} 
	ubuf1[0].tv_sec = statbuf.st_atime;
	ubuf1[1].tv_sec = statbuf.st_mtime;
	if (access(dest_dir, F_OK) && r_mkdir((char *) dest_dir, statbuf.st_mode, 
		statbuf.st_uid, statbuf.st_gid)) {
		strerr_warn3("MoveFile: r_mkdir: ", dest_dir, ": ", &strerr_sys);
		closedir(entry);
		return (-1);
	}
	if (!rename(src_dir, dest_dir)) {
		closedir(entry);
		return (0);
	} else
	if (errno != EXDEV) {
		strerr_warn5("MoveFile: rename: ", src_dir, " --> ", dest_dir, ": ", &strerr_sys);
		closedir(entry);
		return (-1);
	}
	for(status = 0;;) {
		if (!(dp = readdir(entry)))
			break;
		if (!str_diff(dp->d_name, ".") || !str_diff(dp->d_name, ".."))
			continue;
		if (!stralloc_copys(&nsrc_dir, src_dir) ||
				!stralloc_append(&nsrc_dir, "/") ||
				!stralloc_cats(&nsrc_dir, dp->d_name) ||
				!stralloc_0(&nsrc_dir))
			die_nomem();
		if (!stralloc_copys(&ndest_dir, dest_dir) ||
				!stralloc_append(&ndest_dir, "/") ||
				!stralloc_cats(&ndest_dir, dp->d_name) ||
				!stralloc_0(&ndest_dir))
			die_nomem();
		if (lstat(nsrc_dir.s, &statbuf)) {
			strerr_warn3("MoveFile: stat: ", nsrc_dir.s, ": ", &strerr_sys);
			continue;
		}
		ubuf2[0].tv_sec = statbuf.st_atime;
		ubuf2[1].tv_sec = statbuf.st_mtime;
		if (S_ISDIR(statbuf.st_mode))
			status = MoveFile(nsrc_dir.s, ndest_dir.s);
		else
		if (S_ISREG(statbuf.st_mode))
			status = fappend(nsrc_dir.s, ndest_dir.s, "w", statbuf.st_mode, 
				statbuf.st_uid, statbuf.st_gid);
		if (S_ISCHR(statbuf.st_mode))
			status = mknod(ndest_dir.s, statbuf.st_mode|S_IFCHR, statbuf.st_dev);
		else
		if (S_ISBLK(statbuf.st_mode))
			status = mknod(ndest_dir.s, statbuf.st_mode|S_IFBLK, statbuf.st_dev);
		else
		if (S_ISFIFO(statbuf.st_mode))
			status = mknod(ndest_dir.s, statbuf.st_mode|S_IFIFO, statbuf.st_dev);
		else
		if (S_ISLNK(statbuf.st_mode)) {
			if (!stralloc_ready(&tmpbuf, statbuf.st_size + 1))
				die_nomem();
			if ((readlink(nsrc_dir.s, tmpbuf.s, statbuf.st_size + 1)) == -1) {
				strerr_warn3("MoveFile: readlink ", nsrc_dir.s, ": ", &strerr_sys);
				continue;
			}
			if (!stralloc_0(&tmpbuf))
				die_nomem();
			tmpbuf.len--;
			if ((status = symlink(tmpbuf.s, ndest_dir.s)) == -1)
				strerr_warn5("MoveFile: readlink ", tmpbuf.s, " --> ", ndest_dir.s, ": ", &strerr_sys);
		}
		if (status)
			strerr_warn5("MoveFile: copy: ", nsrc_dir.s, " --> ", ndest_dir.s, ": ", &strerr_sys);
		else {
			if (!S_ISDIR(statbuf.st_mode)) {
				if ((status = unlink(nsrc_dir.s)) == -1)
					strerr_warn3("MoveFile: unlink: ", nsrc_dir.s, ": ", &strerr_sys);
			}
		}
		if (!S_ISLNK(statbuf.st_mode) && utimes(ndest_dir.s, ubuf2))
			strerr_warn3("MoveFile: utime: ", ndest_dir.s, ": ", &strerr_sys);
	}
	if (utimes(dest_dir, ubuf1))
		strerr_warn3("MoveFile: utime: ", dest_dir, ": ", &strerr_sys);
	if ((status = rmdir(src_dir) == -1))
		strerr_warn3("MoveFile: rmdir: ", src_dir, ": ", &strerr_sys);
	closedir(entry);
	return (status);
}
