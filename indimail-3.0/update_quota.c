/*
 * $Log: update_quota.c,v $
 * Revision 1.2  2019-04-21 16:14:27+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:33:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <fmt.h>
#include <substdio.h>
#include <strerr.h>
#endif
#include "get_indimailuidgid.h"
#include "variables.h"
#include "get_assign.h"
#include "maildir_to_domain.h"

#ifndef	lint
static char     sccsid[] = "$Id: update_quota.c,v 1.2 2019-04-21 16:14:27+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("update_quota: out of memory", 0);
	_exit(111);
}

int
update_quota(char *Maildir, mdir_t new_size)
{
	static stralloc tmpbuf = {0}, maildir = {0};
	char           *ptr;
	uid_t           uid;
	gid_t           gid;
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_WRLCK, SEEK_SET};
#else
	struct flock    fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
#endif
	char            strnum[FMT_ULONG], outbuf[512];
	struct substdio ssout;
	void            (*pstat[3]) ();
	int             fd, i;

	if (!stralloc_copys(&maildir, Maildir) || !stralloc_0(&maildir))
		die_nomem();
	maildir.len--;
	if (maildir.s[maildir.len - 1] == '/')
		maildir.len--;
#ifdef USE_MAILDIRQUOTA
	if (!stralloc_copy(&tmpbuf, &maildir) ||
			!stralloc_catb(&tmpbuf, "/maildirsize", 12) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (access(tmpbuf.s, F_OK))
		return (0);
#else
	if (!stralloc_copy(&tmpbuf, &maildir) ||
			!stralloc_catb(&tmpbuf, "/.current_size", 14) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
#endif
	if ((ptr = str_str(maildir.s, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
	/*- attempt to obtain uid, gid for the maildir */
	if (!(ptr = maildir_to_domain(maildir.s)) || !get_assign(ptr, 0, &uid, &gid)) {
		if (indimailuid == -1 || indimailgid == -1)
			get_indimailuidgid(&indimailuid, &indimailgid);
		uid = indimailuid;
		gid = indimailgid;
	} 
	if ((fd = open(tmpbuf.s, O_CREAT|O_WRONLY|O_APPEND, S_IWUSR | S_IRUSR)) == -1) {
		strerr_warn3("update_quota: ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	if (fchown(fd, uid, gid) == -1) {
		strerr_warn3("update_quota: fchown", tmpbuf.s, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
	if ((pstat[0] = signal(SIGINT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[1] = signal(SIGQUIT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[2] = signal(SIGTSTP, SIG_IGN)) == SIG_ERR)
		return (-1);
	for (;;) {
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
			if (errno == EINTR)
				continue;
			strerr_warn3("update_quota: fcntl: F_SETLKW", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			(void) signal(SIGINT, pstat[0]);
			(void) signal(SIGQUIT, pstat[1]);
			(void) signal(SIGTSTP, pstat[2]);
			return (-1);
		}
		break;
	}
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
#ifdef USE_MAILDIRQUOTA
	strnum[i = fmt_ulonglong(strnum, new_size)] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, " 1\n", 3) ||
			substdio_flush(&ssout)) {
		strerr_warn3("update_quota: write error: ", tmpbuf.s, ": ", &strerr_sys);
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);
		(void) signal(SIGINT, pstat[0]);
		(void) signal(SIGQUIT, pstat[1]);
		(void) signal(SIGTSTP, pstat[2]);
		return (-1);
	}
#else
	fprintf(fp, "%"PRIu64"\n", new_size);
	strnum[i = fmt_ulonglong(strnum, new_size)] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout)) {
		strerr_warn3("update_quota: write error: ", tmpbuf.s, ": ", &strerr_sys);
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);
		(void) signal(SIGINT, pstat[0]);
		(void) signal(SIGQUIT, pstat[1]);
		(void) signal(SIGTSTP, pstat[2]);
		return (-1);
	}
#endif
	fl.l_type = F_UNLCK;
	if (fcntl(fd, F_SETLK, &fl) == -1)
		strerr_warn3("update_quota: fcntl: F_SETLK", tmpbuf.s, ": ", &strerr_sys);
	close(fd);
	(void) signal(SIGINT, pstat[0]);
	(void) signal(SIGQUIT, pstat[1]);
	(void) signal(SIGTSTP, pstat[2]);
	return (1);
}
