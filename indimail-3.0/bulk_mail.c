/*
 * $Log: bulk_mail.c,v $
 * Revision 1.1  2019-04-18 08:38:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
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
#include <str.h>
#include <env.h>
#include <strerr.h>
#endif
#include "CopyEmailFile.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: bulk_mail.c,v 1.1 2019-04-18 08:38:49+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("bulk_mail: out of memory", 0);
	_exit(111);
}

int
bulk_mail(const char *email, const char *domain, const char *homedir)
{
	DIR            *Dir;
	struct dirent  *Dirent;
	struct stat     statbuf;
	static stralloc TmpBuf = {0}, bulkdir = {0};
	char           *ptr;
	time_t          last_mtime;
	int             status;

	if (!stralloc_copys(&bulkdir, CONTROLDIR) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, (char *) domain) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, (ptr = env_get("BULK_MAILDIR")) ? ptr : BULK_MAILDIR) ||
			!stralloc_0(&bulkdir))
		die_nomem();
	bulkdir.len--;
	if (access(bulkdir.s, F_OK))
		return (0);
	if (!(Dir = opendir(bulkdir.s))) {
		strerr_warn3("bulk_mail: opendir: ", bulkdir.s, ": ", &strerr_sys);
		return (1);
	}
	if (!stralloc_copys(&TmpBuf, (char *) homedir) ||
			!stralloc_catb(&TmpBuf, "/Maildir/BulkMail", 17) ||
			!stralloc_0(&TmpBuf))
		return (1);
	if (stat(TmpBuf.s, &statbuf))
		last_mtime = 0;
	else
		last_mtime = statbuf.st_mtime;
	for (status = 1;;) {
		if (!(Dirent = readdir(Dir)))
			break;
		else
		if (!str_diffn(Dirent->d_name, ".", 1))
			continue;
		else
		if(!str_str(Dirent->d_name, ",all"))
			continue;
		if (!stralloc_copy(&TmpBuf, &bulkdir) ||
				!stralloc_append(&TmpBuf, "/") ||
				!stralloc_cats(&TmpBuf, Dirent->d_name) ||
				!stralloc_0(&TmpBuf))
			return (1);
		if (stat(TmpBuf.s, &statbuf)) {
			strerr_warn3("bulk_mail: stat: ", TmpBuf.s, ": ", &strerr_sys);
			continue;
		} else
		if (!S_ISREG(statbuf.st_mode) || !(statbuf.st_mtime > last_mtime))
			continue;
		if(!CopyEmailFile(homedir, TmpBuf.s, email, 0, 0, 0, 0, 1, statbuf.st_size))
			status = 0;
	}
	closedir(Dir);
	return (status);
}
