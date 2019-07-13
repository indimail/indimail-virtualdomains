/*
 * $Log: vmake_maildir.c,v $
 * Revision 1.1  2019-04-18 07:59:53+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <str.h>
#include <stralloc.h>
#include <strerr.h>
#endif
#include "variables.h"
#include "r_mkdir.h"

#ifndef	lint
static char     sccsid[] = "$Id: vmake_maildir.c,v 1.1 2019-04-18 07:59:53+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vmake_maildir: out of memory", 0);
	_exit(111);
}

int
vmake_maildir(char *dir, uid_t uid, gid_t gid, char *domain)
{
	int             i, fd;
	static stralloc tmp = {0};
	char           *MailDirNames[] = {
		"cur",
		"new",
		"tmp",
	};

	if (!stralloc_copys(&tmp, dir) ||
			!stralloc_catb(&tmp, "/Maildir", 8) || !stralloc_0(&tmp))
		die_nomem();
	if (access(tmp.s, F_OK) && r_mkdir(tmp.s, INDIMAIL_DIR_MODE, uid, gid))
		return (-1);
	for(i = 0;i < 3;i++) {
		if (!stralloc_copys(&tmp, dir) ||
				!stralloc_catb(&tmp, "/Maildir", 8) ||
				!stralloc_append(&tmp, "/") ||
				!stralloc_cats(&tmp, MailDirNames[i]) ||
				!stralloc_0(&tmp))
			die_nomem();
		if (access(tmp.s, F_OK)) {
			if (mkdir(tmp.s, INDIMAIL_DIR_MODE) == -1)
				strerr_die3sys(111, "vmake_maildir: mkdir: ", tmp.s, ": ");
			else
			if (chown(tmp.s, uid, gid) == -1)
				strerr_die3sys(111, "vmake_maildir: chown: ", tmp.s, ": ");
		} else
		if (chmod(tmp.s, INDIMAIL_DIR_MODE) == -1)
			strerr_die3sys(111, "vmake_maildir: chmod: ", tmp.s, ": ");
	}
	if (use_etrn)
		return (0);
	if (!stralloc_copys(&tmp, dir) ||
			!stralloc_catb(&tmp, "/Maildir/domain", 15) || !stralloc_0(&tmp))
		die_nomem();
	if ((fd = open_trunc(tmp.s)) == -1)
		strerr_die3sys(111, "vmake_maildir: open: ", tmp.s, ": ");
	if (domain && *domain) {
		if (write(fd, domain, str_len(domain)) == -1)
			strerr_die3sys(111, "vmake_maildir: write: ", tmp.s, ": ");
		if (write(fd, "\n", 1) == -1)
			strerr_die3sys(111, "vmake_maildir: write: ", tmp.s, ": ");
	} else {
		if (write(fd, "localhost\n", 10) == -1)
			strerr_die3sys(111, "vmake_maildir: write: ", tmp.s, ": ");
	}
	close(fd);
	if (chown(tmp.s, uid, gid))
		strerr_die3sys(111, "vmake_maildir: chown: ", tmp.s, ": ");
	/*- Prevent Current Bulletins to be delivered */
	if (!stralloc_copys(&tmp, dir) ||
			!stralloc_catb(&tmp, "/Maildir/BulkMail", 17) || !stralloc_0(&tmp))
		die_nomem();
	close(open(tmp.s, O_CREAT | O_TRUNC , 0644));
	if (chown(tmp.s, uid, gid) == -1)
		strerr_die3sys(111, "vmake_maildir: chown: ", tmp.s, ": ");
#ifdef CREATE_FLAG_ON_NEWUSER
	if (!stralloc_copys(&tmp, dir) ||
			!stralloc_append(&tmp, "/") ||
			!stralloc_cats(&tmp, MIGRATEFLAG))
		die_nomem();
	close(open(tmpbuf, O_TRUNC | O_CREAT , 0600));
	if (chown(tmpbuf, uid, gid) == -1)
		strerr_die3sys(111, "vmake_maildir: chown: ", tmp.s, ": ");
#endif
	return (0);
}
