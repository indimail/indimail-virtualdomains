/*
 * $Log: trashpurge.c,v $
 * Revision 1.1  2019-04-18 08:33:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
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
#include <scan.h>
#include <error.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: trashpurge.c,v 1.1 2019-04-18 08:33:44+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("trashpurge: out of memory", 0);
	_exit(111);
}

long
mailboxpurge(char *dir, char *mailbox, long age, int fast_option)
{
	static stralloc tmpbuf = {0}, file_name = {0};
	DIR            *entry;
	struct dirent  *dp;
	struct stat     statbuf;
	int             i, j;
	long            deleted, tmpdate;
	time_t          tmval;
	char           *MaildirNames[] = {
		"cur",
		"new",
		"tmp",
	};

	if (access(dir, F_OK))
		return(0);
	tmval = time(0);
	for (deleted = 0l, i = 0; i < 3; i++) {
		if (!stralloc_copys(&tmpbuf, dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir/", 9) ||
				!stralloc_cats(&tmpbuf, mailbox) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_catb(&tmpbuf, MaildirNames[i], 3) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (!(entry = opendir(tmpbuf.s))) {
			if (errno != error_noent)
				strerr_warn3("trashpurge: opendir: ", tmpbuf.s, ": ", &strerr_sys);
			continue;
		}
		for (;;) {
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
			if (!fast_option) {
				if (lstat(file_name.s, &statbuf)) {
					strerr_warn3("trashpurge: lstat: ", file_name.s, ": ", &strerr_sys);
					continue;
				}
				if (S_ISDIR(statbuf.st_mode))
					continue;
			}
			if (age) {
				/*-
				 * filename's initial component is the creation time e.g.
				 * 1553625007.I22a06d3Vfd02M861387P9174.indimail.org:2,S
				 * if filename does not start with number, tmpdate will be 0
				 */
				scan_ulong(dp->d_name, (unsigned long *) &tmpdate); /*- If the this fails do not delete the file */
				if ((age && ((tmval - tmpdate)/86400 <= age)) || (tmpdate == 0L))
					continue;
			}
			if (!unlink(file_name.s)) {
				if (fast_option) {
					j = str_chr(dp->d_name, '=');
					if (dp->d_name[j]) {
						scan_ulong(dp->d_name + j + 1, (unsigned long *) &tmpdate);
						deleted += tmpdate;
					} else {
						if (lstat(file_name.s, &statbuf)) {
							strerr_warn3("trashpurge: lstat: ", file_name.s, ": ", &strerr_sys);
							continue;
						}
						deleted += statbuf.st_size;
					}
				} else
					deleted += statbuf.st_size;
			} else
				strerr_warn3("trashpurge: unlink: ", file_name.s, ": ", &strerr_sys);
		} /*- for (;;) */
		closedir(entry);
	}
	return (deleted);
}

long
trashpurge(char *dir)
{
	return(mailboxpurge(dir, ".Trash", -1, 0));
}
