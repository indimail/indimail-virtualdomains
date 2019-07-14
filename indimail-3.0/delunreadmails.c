/*
 * $Log: delunreadmails.c,v $
 * Revision 1.1  2019-04-18 08:37:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <strmsg.h>
#include <str.h>
#include <alloc.h>
#endif
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: delunreadmails.c,v 1.1 2019-04-18 08:37:45+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("delunreadmails: out of memory", 0);
	_exit(111);
}

long
delunreadmails(char *dir, int type, int days)
{
	static stralloc tmpbuf = {0}, file_name = {0};
	time_t          curtime;
	DIR            *entry;
	struct dirent  *dp;
	long            deleted;
	struct stat     statbuf;

	if (!stralloc_copys(&tmpbuf, dir) ||
			!stralloc_catb(&tmpbuf, "/Maildir/", 9) ||
			!stralloc_catb(&tmpbuf, type == 1 ? "new" : "cur", 3) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	tmpbuf.len--;

	if (!(entry = opendir(tmpbuf.s))) {
		if (errno == error_noent)
			return (0);
		strerr_warn3("delunreadmails: opendir: ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	curtime = time(0);
	for (deleted = 0l;;) {
		if (!(dp = readdir(entry)))
			break;
		if (!str_diffn(dp->d_name, ".", 2) || !str_diffn(dp->d_name, "..", 3))
			continue;
		if (!stralloc_copy(&file_name, &tmpbuf) ||
				!stralloc_append(&file_name, "/") ||
				!stralloc_cats(&file_name, dp->d_name) ||
				!stralloc_0(&file_name))
			die_nomem();
		file_name.len--;
		if (type == 2 && file_name.s[file_name.len - 2] != ',')
			continue;
		if (lstat(file_name.s, &statbuf)) {
			strerr_warn3("delunreadmails: lstat: ", file_name.s, ": ", &strerr_sys);
			continue;
		}
		if (!S_ISDIR(statbuf.st_mode) && ((curtime - statbuf.st_mtime) > days * 86400)) {
			if (verbose)
				strmsg_out3("Removing File ", file_name.s, "\n");
			if (!unlink(file_name.s))
				deleted += statbuf.st_size;
			else
				strerr_warn3("delunreadmails: unlink: ", file_name.s, ": ", &strerr_sys);
		}
	}
	closedir(entry);
	return (deleted);
}
