/*
 * $Log: SendWelcomeMail.c,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2020-10-19 12:46:58+05:30  Cprogrammer
 * use /var/indomain/domains for domain/bulk_mail
 *
 * Revision 1.1  2019-04-18 08:37:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <strerr.h>
#endif
#include "indimail.h"
#include "CopyEmailFile.h"

#ifndef	lint
static char     sccsid[] = "$Id: SendWelcomeMail.c,v 1.3 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("SendWelcomeMail: out of memory", 0);
	_exit(111);
}

void
SendWelcomeMail(const char *homedir, const char *username, const char *domain, int inactFlag, const char *subject)
{
	static stralloc email = {0}, tmpbuf = {0}, bulkdir = {0};
	char           *ptr;
	struct stat     statbuf;

	if (!stralloc_copys(&bulkdir, INDIMAILDIR) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, domain) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, (ptr = env_get("BULK_MAILDIR")) ? ptr : BULK_MAILDIR) ||
			!stralloc_0(&bulkdir))
		die_nomem();
	bulkdir.len--;
	if (!access(bulkdir.s, F_OK)) {
		if (!stralloc_copy(&tmpbuf, &bulkdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf,
					inactFlag ? ((ptr = env_get("ACTIVATEMAIL")) ? ptr : ACTIVATEMAIL) : ((ptr = env_get("WELCOMEMAIL")) ? ptr : WELCOMEMAIL)) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (!stat(tmpbuf.s, &statbuf)) {
			if (!stralloc_copys(&email, username) ||
					!stralloc_append(&email, "@") ||
					!stralloc_cats(&email, domain) ||
					!stralloc_0(&email))
				die_nomem();
			CopyEmailFile(homedir, tmpbuf.s, email.s, 0, 0, subject, 1, 0, statbuf.st_size);
		}
	}
	return;
}
