/*
 * $Log: update_rules.c,v $
 * Revision 1.2  2020-04-01 18:58:16+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:33:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: update_rules.c,v 1.2 2020-04-01 18:58:16+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <fmt.h>
#include <str.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <getEnvConfig.h>
#endif
#include "dblock.h"
#include "get_indimailuidgid.h"
#include "vupdate_rules.h"
#include "variables.h"

static unsigned long tcprules_open(int *);

static void
die_nomem()
{
	strerr_warn1("update_rules: out of memory", 0);
	_exit(111);
}

int
update_rules(lock)
	int             lock;
{
#ifdef FILE_LOCKING
	int             lockfd = -1;
#endif
	unsigned long   pid;
	int             wstat, tcpfd, fdm, match;
	char           *ptr, *tcp_file, *open_smtp;
	static stralloc tmpbuf = {0}, line = {0};
	struct substdio ssin;
	char            inbuf[4096];

	getEnvConfigStr(&open_smtp, "OPEN_SMTP", OPEN_SMTP);
#ifdef FILE_LOCKING
	if (lock && ((lockfd = getDbLock(open_smtp, 1)) == -1))
		return (-1);
#endif
	if ((pid = tcprules_open(&fdm)) < 0) {
#ifdef FILE_LOCKING
		if (lock)
			delDbLock(lockfd, open_smtp, 1);
#endif
		close(lockfd);
		return (1);
	}
	if (vupdate_rules(fdm)) { /*- write rules from RELAY table */
#ifdef FILE_LOCKING
		if (lock)
			delDbLock(lockfd, open_smtp, 1);
#endif
		close(lockfd);
		close(fdm);
		while (wait(&wstat) != pid);
		return (1);
	}
	getEnvConfigStr(&tcp_file, "TCP_FILE", TCP_FILE);
	if ((tcpfd = open_read(tcp_file)) == -1) {
		if (errno == error_noent) {
#ifdef FILE_LOCKING
		if (lock)
			delDbLock(lockfd, open_smtp, 1);
#endif
			close(lockfd);
			close(fdm);
			return (0);
		}
#ifdef FILE_LOCKING
		if (lock)
			delDbLock(lockfd, open_smtp, 1);
#endif
		close(lockfd);
		close(fdm);
		return (-1);
	}
	substdio_fdbuf(&ssin, read, tcpfd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("update_rules: read: ", tcp_file, ": ", &strerr_sys);
#ifdef FILE_LOCKING
			if (lock)
				delDbLock(lockfd, open_smtp, 1);
#endif
			close(lockfd);
			close(fdm);
			close(tcpfd);
			return (-1);
		}
		if (line.len == 0)
			break;
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (write(fdm, line.s, line.len) == -1) {/*- write rules in the file tcp.smtp */
#ifdef FILE_LOCKING
			if (lock)
				delDbLock(lockfd, open_smtp, 1);
#endif
			break;
		}
	}
	close(fdm);
	close(tcpfd);
#ifdef FILE_LOCKING
	if (lock)
		delDbLock(lockfd, open_smtp, 1);
#endif
	close(lockfd);
	/*- wait until tcprules finishes so we don't have zombies */
	while (wait(&wstat) != pid);
	/*- Set the ownership of the file */
	if (!stralloc_copys(&tmpbuf, tcp_file) ||
			!stralloc_catb(&tmpbuf, ".cdb", 4) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	if (!getuid() || !geteuid())
		if (chown(tmpbuf.s, indimailuid, indimailgid) == -1) {
			strerr_warn3("update_rules: chown: ", tmpbuf.s, ": ", &strerr_sys);
			return (-1);
		}
	return (0);
}

static unsigned long
tcprules_open(int *fd)
{
	int             pim[2], i;
	unsigned long   pid;
	char            strnum[FMT_ULONG];
	char           *tcp_file;
	char           *binqqargs[4];
	static stralloc bin0 = {0}, bin1 = {0}, bin2 = {0};

	*fd = -1;
	if (pipe(pim) == -1)
		return (-1);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (-1);
	case 0:
		close(pim[1]);
		if (dup2(pim[0], 0) == -1) {
			close(pim[0]);
			_exit(120);
		}
		umask(INDIMAIL_TCPRULES_UMASK);
		getEnvConfigStr(&tcp_file, "TCP_FILE", TCP_FILE);
		if (!stralloc_copys(&bin0, TCPRULES_PROG) || !stralloc_0(&bin0))
			die_nomem();
		if (!stralloc_copys(&bin1, tcp_file) ||
				!stralloc_catb(&bin1, ".cdb", 4) ||
				!stralloc_0(&bin1))
			die_nomem();
		strnum[i = fmt_ulong(strnum, getpid())] = 0;
		if (!stralloc_copys(&bin2, tcp_file) ||
				!stralloc_catb(&bin2, ".tmp.", 5) ||
				!stralloc_catb(&bin2, strnum, i) ||
				!stralloc_0(&bin2))
			die_nomem();
		binqqargs[0] = bin0.s;
		binqqargs[1] = bin1.s;
		binqqargs[2] = bin2.s;
		binqqargs[3] = 0;
		execv(*binqqargs, binqqargs);
	}
	*fd = pim[1];
	close(pim[0]);
	return (pid);
}
#endif
