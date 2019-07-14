/*
 * $Log: purge_files.c,v $
 * Revision 1.1  2019-04-18 08:31:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <substdio.h>
#include <subfd.h>
#endif
#include "skip_system_files.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: purge_files.c,v 1.1 2019-04-18 08:31:52+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("purge_files: out of memory", 0);
	_exit(111);
}

int
purge_files(char *dirname, int days)
{
	static stralloc tmpbuf = {0};
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	time_t          tmval;

	if (chdir(dirname)) {
		strerr_warn3("purge_files: chdir: ", dirname, ": ", &strerr_sys);
		return (1);
	}
	if (!(entry = opendir(dirname))) {
		strerr_warn3("purge_files: opendir: ", dirname, ": ", &strerr_sys);
		return (1);
	}
	tmval = time(0);
	for (;;) {
		if (!(dp = readdir(entry)))
			break;
		if (!str_diffn(dp->d_name, ".", 2) || !str_diffn(dp->d_name, "..", 3))
			continue;
		else
		if (str_diffn(dp->d_name, ".Trash", 7) && skip_system_files(dp->d_name))
			continue;
		if (!stralloc_copys(&tmpbuf, dirname) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, dp->d_name) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		tmpbuf.len--;
		if (stat(tmpbuf.s, &statbuf)) {
			strerr_warn3("purge_files: stat: ", tmpbuf.s, ": ", &strerr_sys);
			continue;
		}
		if (S_ISDIR(statbuf.st_mode))
			purge_files(tmpbuf.s, days);
		else
		if (((tmval - statbuf.st_mtime) / (3600 * 24)) >= days) {
			if (unlink(tmpbuf.s) == -1) {
				strerr_warn3("purge_files: unlink: ", tmpbuf.s, ": ", &strerr_sys);
				continue;
			}
			if (verbose) {
				if (substdio_put(subfdout, "removed file ", 13) ||
						substdio_put(subfdout, tmpbuf.s, tmpbuf.len))
					strerr_warn1("purge_file: unable to write to stdout", &strerr_sys);
			}
		}
	}
	if (substdio_flush(subfdout))
		strerr_warn1("purge_file: unable to write to stdout", &strerr_sys);
	closedir(entry);
	return (0);
}
