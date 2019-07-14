/*
 * $Log: qmail_remote.c,v $
 * Revision 1.2  2019-04-22 23:18:40+05:30  Cprogrammer
 * replaced exit with _exit
 *
 * Revision 1.1  2019-04-18 08:36:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: qmail_remote.c,v 1.2 2019-04-22 23:18:40+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <substdio.h>
#include <subfd.h>
#include <fmt.h>
#endif
#include "getEnvConfig.h"
#include "deliver_mail.h"
#include "get_message_size.h"

static void
die_nomem()
{
	strerr_warn1("ismaildup: out of memory", 0);
	_exit(111);
}

static int  decode(int);

static int
decode(int read_fd)
{
	int             result, bytes, j, k, orr;
	char            inbuf[128];
	static stralloc msgbuf = {0};
	char           *ptr = 0;

	for (;;) {
		bytes = read(read_fd, inbuf, 128);
#ifdef ERESTART
		if (bytes == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (bytes == -1 && errno == EINTR)
#endif
			continue;
		else
		if (bytes == -1) {
			strerr_warn1("qmail_remote: decode: read: ", &strerr_sys);
			return (111);
		} else
		if (!bytes)
			break;
		if (!stralloc_catb(&msgbuf, inbuf, bytes))
			die_nomem();
	}
	if (!msgbuf.len) {
		strerr_warn1("qmail-remote produced no output.", 0);
		return (111);
	}
	if (!stralloc_0(&msgbuf))
		die_nomem();
	msgbuf.len--;
	ptr = msgbuf.s;
	result = -1;
	j = 0;
	for (k = 0; k < msgbuf.len + 1; ++k) {
		if (!ptr[k]) {
			if (ptr[j] == 'K') {
				result = 1;
				break;
			}
			if (ptr[j] == 'Z') {
				result = 0;
				break;
			}
			if (ptr[j] == 'D')
				break;
			j = k + 1;
		}
	}
	orr = result;
	switch (ptr[0])
	{
	case 's':
		orr = 0;
		break;
	case 'h':
		orr = -1;
	}
	for (k = 0; k < msgbuf.len + 1;) {
		if (!ptr[k++]) {
			if (substdio_puts(subfdoutsmall, ptr + 1)) {
				strerr_warn1("qmail_remote: unable to write to stdout: ", &strerr_sys);
				return (111);
			}
			if (result <= orr && k < msgbuf.len + 1)
			{
				switch (ptr[k])
				{
				case 'Z':
				case 'D':
				case 'K':
					if (substdio_puts(subfdoutsmall, ptr + k + 1)) {
						strerr_warn1("qmail_remote: unable to write to stdout: ", &strerr_sys);
						return (111);
					}
				}
			}
			break;
		}
	} /*- for (k = 1; k < msgbuf.len + 1;) */
	if (substdio_flush(subfdoutsmall) == -1) {
		strerr_warn1("qmail_remote: unable to write to stdout: ", &strerr_sys);
		return (111);
	}
	switch (orr)
	{
	case 1:
		return (0);
		break;
	case 0:
		return (111);
		break;
	case -1:
		return (100);
		break;
	}
	return (111);
}

/* 
 * Deliver an email to an address
 * Return 0 on success
 * Return less than zero on failure
 * 
 */
int
qmail_remote(char *user, char *domain)
{
	int             pim1[2], wait_status, i, err, tmperrno;
	char            msg_size[FMT_ULONG];
	mdir_t          msize;
	pid_t           pid;
	char           *ptr, *binqqargs[7];
	static stralloc recipient = {0};

	if (pipe(pim1) == -1)
		return (-2);
	if (!stralloc_copys(&recipient, user) ||
			!stralloc_append(&recipient, "@") ||
			!stralloc_cats(&recipient, domain) ||
			!stralloc_0(&recipient))
		die_nomem();
	if ((msize = get_message_size()) == -1)
		return (-2);
	msg_size[i = fmt_ulong(msg_size, (unsigned long) msize)] = 0;
	switch (pid = vfork())
	{
	case -1:
		tmperrno = errno;
		close(pim1[0]);
		close(pim1[1]);
		errno = tmperrno;
		return (-2);
	case 0:
		close(pim1[0]);
		if (dup2(pim1[1], 1) == -1)
			_exit(111);
		if (dup2(1, 2) == -1)
			_exit(111);
		binqqargs[0] = "qmail-remote";
		binqqargs[1] = domain;
		getEnvConfigStr(&ptr, "SENDER", "");
		binqqargs[2] = ptr;
		getEnvConfigStr(&ptr, "QQEH", "");
		binqqargs[3] = ptr;
		binqqargs[4] = msg_size;
		binqqargs[5] = recipient.s;
		binqqargs[6] = 0;
		execv(PREFIX"/sbin/qmail-remote", binqqargs);
		if(error_temp(errno))
			_exit(111);
		_exit(100);
	}
	close(pim1[1]);
	err = decode(pim1[0]);
	close(pim1[0]);
	for(;;) {
		pid = wait(&wait_status);
#ifdef ERESTART
		if(pid == -1 && (errno == EINTR || errno == ERESTART))
#else
		if(pid == -1 && errno == EINTR)
#endif
			continue;
		else
		if(pid == -1)
			return (-2);
		break;
	}
	if(WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status)) {
		strerr_warn1("qmail-remote crashed.", 0);
		return (111);
	} else
	if(WIFEXITED(wait_status)) {
		switch (WEXITSTATUS(wait_status))
		{
		case 0:
			break;
		case 111:
			return (111);
		default:
			return (100);
		}
	}
	return (err);
}
