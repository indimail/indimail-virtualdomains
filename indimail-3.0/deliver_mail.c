/*
 * $Log: deliver_mail.c,v $
 * Revision 1.5  2019-06-18 09:57:41+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.4  2019-06-17 23:23:07+05:30  Cprogrammer
 * fixed SIGSEGV when tmpdate.s was null
 *
 * Revision 1.3  2019-04-22 23:10:19+05:30  Cprogrammer
 * fixed strptime format
 *
 * Revision 1.2  2019-04-21 16:13:42+05:30  Cprogrammer
 * remove '/' from the end
 *
 * Revision 1.1  2019-04-18 08:16:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ismaildup.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <substdio.h>
#include <subfd.h>
#include <getln.h>
#include <open.h>
#include <env.h>
#include <str.h>
#include <error.h>
#include <fmt.h>
#include <scan.h>
#include <getEnvConfig.h>
#endif
#include "dblock.h"
#include "recalc_quota.h"
#include "user_over_quota.h"
#include "parse_quota.h"
#include "update_quota.h"
#include "makeseekable.h"
#include "get_localtime.h"
#include "variables.h"
#include "maildir_to_email.h"
#include "runcmmd.h"
#include "sql_getpw.h"
#include "vset_lastdeliver.h"
#include "MailQuotaWarn.h"
#include "r_mkdir.h"
#include "udpopen.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: deliver_mail.c,v 1.5 2019-06-18 09:57:41+05:30 Cprogrammer Exp mbhangui $";
#endif

static stralloc tmpbuf = {0};
static char     strnum[FMT_ULONG];

static void
die_nomem()
{
	strerr_warn1("deliver_mail: out of memory", 0);
	_exit(111);
}

static void
getAlertConfig(char **mailalert_host, char **mailalert_port)
{
	static stralloc alert_host = {0}, alert_port = {0}, line = {0};
	char           *ptr, *sysconfdir, *controldir;
	int             fd, len, match;
	struct substdio ssin;
	char            inbuf[512];

	if (alert_host.len)
		*mailalert_host = alert_host.s;
	if (alert_port.len)
		*mailalert_port = alert_port.s;
	if (alert_host.len || alert_port.len)
		return;
	*mailalert_host = *mailalert_port = 0;
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/mailalert.cfg", 14) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&tmpbuf, sysconfdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/mailalert.cfg", 14) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "deliver_mail: open: ", tmpbuf.s, ": ");
		return;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("deliver_mail: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			_exit(111);
		}
		if (line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		if (!str_diffn(line.s, "host ", 5)) {
			ptr = line.s + 5;
			len = line.len - 5;
			for (; *ptr && isspace(*ptr); ptr++, len--);
			if (!stralloc_copyb(&alert_host, ptr, len) || !stralloc_0(&alert_host))
				die_nomem();
		} else
		if (!str_diffn(line.s, "port ", 5)) {
			ptr = line.s + 5;
			len = line.len - 5;
			for (; *ptr && isspace(*ptr); ptr++, len--);
			if (!stralloc_copyb(&alert_port, ptr, len) || !stralloc_0(&alert_port))
				die_nomem();
		}
	}
	close(fd);
	return;
}

static int
qmail_inject_open(char *address, int *write_fd)
{
	int             pim[2];
	long unsigned   pid;
	char           *sender;
#ifdef POSTFIXDIR
	char           *mta;
#endif
	static stralloc _address = {0};
	char           *binqqargs[6], *bin0;

	/*- skip over an & sign if there */
	*write_fd = -1;
	if (*address == '&')
		address++;
	if (pipe(pim) == -1)
		return (-1);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (-1);
	case 0:
		/*- unset only if QHPSI is set */
		if (env_get("QHPSI") && !env_unset("QQEH"))
			strerr_die1sys(111, "deliver_mail: env_unset: QQEH: ");
		close(pim[1]);
		if (dup2(pim[0], 0) == -1)
			_exit(111);
		getEnvConfigStr(&sender, "SENDER", "postmaster");
#ifdef POSTFIXDIR
		if (!(mta = env_get("MTA")))
			bin0 = PREFIX"/bin/qmail-inject";
		else
		{
			if (!str_diffn(mta, "Postfix", 8))
				bin0 = PREFIX"/bin/sendmail";
			else
				bin0 = PREFIX"/bin/qmail-inject";
		}
#else
		bin0 = PREFIX"/bin/qmail-inject";
#endif
		if (!stralloc_copys(&_address, address) || !stralloc_0(&_address)) {
			strerr_warn1("deliver_mail: child out of memory", 0);
			_exit(111);
		}
		binqqargs[0] = bin0;
		binqqargs[1] = "-f";
		binqqargs[2] = sender; 
		binqqargs[3] = _address.s;
		binqqargs[4] = 0;
		execv(*binqqargs, binqqargs);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	*write_fd = pim[1];
	close(pim[0]);
	return (pid);
}

#ifdef USE_MAILDIRQUOTA
char           *
read_quota(char *Maildir)
{
	static stralloc maildir = {0}, line = {0};
	char           *ptr;
	int             count, fd, match;
	char            inbuf[4096];
	struct substdio ssin;

	if (!stralloc_copys(&maildir, Maildir) || !stralloc_0(&maildir))
		die_nomem();
	if ((ptr = str_str(maildir.s, "/Maildir/")) && *(ptr + 9)) {
		*(ptr + 9) = 0;
		maildir.len = str_len(maildir.s);
	}
	if (maildir.s[maildir.len - 1] == '/')
		maildir.len--;
	if (!stralloc_copy(&tmpbuf, &maildir) ||
			!stralloc_catb(&tmpbuf, "/maildirsize", 12) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	for (count = 0;; count++) {
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno == error_noent)
				return ("NOQUOTA");
			strerr_die3sys(111, "deliver_mail: open: ", tmpbuf.s, ": ");
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		/*- get the first line */
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("deliver_mail: read: ", tmpbuf.s, ": ", 0);
			close(fd);
			_exit(111);
		}
		if (!match || !line.len) {
			close(fd);
#ifdef USE_MAILDIRQUOTA	
			if (recalc_quota(maildir.s, 0, 0, 0, 2) == -1)
				return ((char *) 0);
#else
			if (recalc_quota(maildir.s, 2) == -1)
				return ((char *) 0);
#endif
			if (!count)
				continue;
			strerr_warn3("deliver_mail: ", tmpbuf.s, " has invalid maildirquota specification", 0);
			return ((char *) 0);
		}
		close(fd);
		break;
	}
	if (!line.len) { /*- fix for courier imap mucking things up */
#ifdef USE_MAILDIRQUOTA	
		(void) recalc_quota(maildir.s, 0, 0, 0, 2);
#else
		(void) recalc_quota(maildir.s, 2);
#endif
		return ("NOQUOTA");
	}
	line.s[--line.len] = 0;
	if (!str_diffn(line.s, "0S", 2))
		return ("NOQUOTA");
	return (line.s);
}
#endif

static int
recordMailcount(char *maildir, mdir_t curmsgsize, mdir_t *dailyMsgSize, mdir_t *dailyMsgCount)
{
	static stralloc tmpdate = {0}, fileName = {0}, line = {0};
	char            datebuf[28], inbuf[128], outbuf[128];
	int             fd, match, len;
	char           *ptr;
	unsigned long   pos, count, size, mail_limit, size_limit;
	time_t          tmval;
	struct tm      *tmptr;
	substdio        ssin, ssout;
#ifdef FILE_LOCKING
	int             lockfd;
#endif

	tmval = time(0);
	if (!(tmptr = localtime(&tmval))) {
		strerr_warn1("deliver_mail: localtime: ", &strerr_sys);
		return (-2);
	}
	ptr = datebuf;
	if (tmptr->tm_mday < 10)
		ptr += fmt_strn(ptr, "0", 1);
	ptr += fmt_uint(ptr, tmptr->tm_mday);
	ptr += fmt_strn(ptr, "-", 1);
	if ((tmptr->tm_mon + 1) < 10)
		ptr += fmt_strn(ptr, "0", 1);
	ptr += fmt_uint(ptr, tmptr->tm_mon + 1);
	ptr += fmt_strn(ptr, "-", 1);
	ptr += fmt_uint(ptr, tmptr->tm_year + 1900);
	ptr += fmt_strn(ptr, ":", 1);
	if (tmptr->tm_hour < 10)
		ptr += fmt_strn(ptr, "0", 1);
	ptr += fmt_uint(ptr, tmptr->tm_hour);
	ptr += fmt_strn(ptr, ":", 1);
	if (tmptr->tm_min < 10)
		ptr += fmt_strn(ptr, "0", 1);
	ptr += fmt_uint(ptr, tmptr->tm_min);
	ptr += fmt_strn(ptr, ":", 1);
	if (tmptr->tm_sec < 10)
		ptr += fmt_strn(ptr, "0", 1);
	ptr += fmt_uint(ptr, tmptr->tm_sec);
	*ptr++ = 0;
	if (!stralloc_copys(&fileName, maildir) ||
			!stralloc_catb(&fileName, "/deliveryCount", 14) ||
			!stralloc_0(&fileName))
		die_nomem();
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(fileName.s, 1)) == -1) {
		strerr_warn1("deliver_mail: getDbLock: ", &strerr_sys);
		return (-2);
	}
#endif
	if (access(fileName.s, F_OK))
		fd = open(fileName.s, O_CREAT|O_RDWR, 0644);
	else
		fd = open(fileName.s, O_RDWR);
	if (fd == -1) {
		strerr_warn3("deliver_mail: open: ", fileName.s, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, fileName.s, 1);
#endif
		return (-2);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	/*
	 * read lines with the following format
	 * date total_mailcount total_mailsize
	 */
	for (pos = 0, tmpdate.len = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("deliver_mail: read: ", fileName.s, ": ", &strerr_sys);
			close(fd);
#ifdef FILE_LOCKING
			delDbLock(lockfd, fileName.s, 1);
#endif
			return (-2);
		}
		if (!line.len || !match)
			break;
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
		for (len = 0, ptr = line.s;*ptr && !isspace(*ptr); len++, ptr++);
		if (!*ptr) /*- invalid line */
			continue;
		if (!stralloc_copyb(&tmpdate, line.s, len) || !stralloc_0(&tmpdate))
			die_nomem();
		for (; *ptr && isspace(*ptr); ptr++);
		if (!*ptr) /*- invalid line */
			continue;
		scan_ulong(ptr, &count);
		for (; *ptr && !isspace(*ptr); ptr++);
		if (!*ptr) /*- invalid line */
			continue;
		for (; *ptr && isspace(*ptr); ptr++);
		if (!*ptr) /*- invalid line */
			continue;
		scan_ulong(ptr, &size);
		pos = line.len;
	}
	/*-
	 * match indicates that mail has already been
	 * delivered on a date. Else this is
	 * the first time mail is being delivered
	 */
	if (tmpdate.len && !str_diffn(tmpdate.s, datebuf, 10)) {
		if (lseek(fd, 0 - pos, SEEK_END) == -1) {
			strerr_warn3("deliver_mail: lseek: ", fileName.s, ": ", &strerr_sys);
			close(fd);
#ifdef FILE_LOCKING
			delDbLock(lockfd, fileName.s, 1);
#endif
			return (-2);
		}
	} else 
		size = count = 0;
	if (dailyMsgCount)
		*dailyMsgCount = count;
	if (dailyMsgSize)
		*dailyMsgSize = size;

	getEnvConfiguLong(&mail_limit, "MAILCOUNT_LIMIT", MAILCOUNT_LIMIT);
	getEnvConfiguLong(&size_limit, "MAILSIZE_LIMIT", MAILSIZE_LIMIT);
	count++;
	size += curmsgsize;
	if ((mail_limit > 0 && count > mail_limit) || (size_limit > 0 && size > size_limit)) {
		close(fd);
		if (count > mail_limit && size > size_limit) {
			if (substdio_put(subfderr, "Dir has insufficient quota. Mail count and size exceeded. ", 58) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, count)) ||
					substdio_put(subfderr, "/", 1) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, mail_limit)) ||
					substdio_put(subfderr, " ", 1) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, size)) ||
					substdio_put(subfderr, "/", 1) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, size_limit)) ||
					substdio_put(subfderr, ". indimail (#5.1.4)", 19))
			{
				strerr_warn1("deliver_mail: unable to write to stderr", &strerr_sys);
				close(fd);
#ifdef FILE_LOCKING
				delDbLock(lockfd, fileName.s, 1);
#endif
				return (-2);
			}
		} else {
			if (count > mail_limit) {
				if (substdio_put(subfderr, "Dir has insufficient quota. Mail count exceeded. ", 49) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, count)) ||
					substdio_put(subfderr, "/", 1) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, mail_limit)) ||
					substdio_put(subfderr, ". indimail (#5.1.4)", 19))
				{
					strerr_warn1("deliver_mail: unable to write to stderr", &strerr_sys);
					close(fd);
#ifdef FILE_LOCKING
					delDbLock(lockfd, fileName.s, 1);
#endif
					return (-2);
				}
			}
			if (size > size_limit) {
				if (substdio_put(subfderr, "Dir has insufficient quota. Mail size exceeded. ", 48) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, size)) ||
					substdio_put(subfderr, "/", 1) ||
					substdio_put(subfderr, strnum, fmt_ulong(strnum, size_limit)) ||
					substdio_put(subfderr, ". indimail (#5.1.4)", 19))
				{
					strerr_warn1("deliver_mail: unable to write to stderr", &strerr_sys);
					close(fd);
#ifdef FILE_LOCKING
					delDbLock(lockfd, fileName.s, 1);
#endif
					return (-2);
				}
			}
		}
		if (substdio_flush(subfderr)) {
			strerr_warn1("deliver_mail: unable to write to stderr", &strerr_sys);
			close(fd);
#ifdef FILE_LOCKING
			delDbLock(lockfd, fileName.s, 1);
#endif
			return (-2);
		}
#ifdef FILE_LOCKING
		delDbLock(lockfd, fileName.s, 1);
#endif
		return (-1);
	}
	if (substdio_put(&ssout, datebuf, 19) ||
			substdio_put(&ssout, " ", 1) ||
			substdio_put(&ssout, strnum, fmt_ulong(strnum, count)) ||
			substdio_put(&ssout, " ", 1) ||
			substdio_put(&ssout, strnum, fmt_ulong(strnum, size)) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_flush(&ssout)) {
		strerr_warn3("deliver_mail: write error: ", fileName.s, ": ", &strerr_sys);
		return (-1);
	}
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, fileName.s, 1);
#endif
	return (0);
}

static int
dateFolder(time_t tmval, stralloc *buffer, char *format)
{
	struct tm      *tmptr;
	char            buf[1024];

	if (!(tmptr = localtime(&tmval)))
		return (1);
	if (!strftime(buf, sizeof(buf) - 1, format, tmptr))
		return (1);
	if (!stralloc_copys(buffer, buf) || !stralloc_0(buffer))
		die_nomem();
	buffer->len--;
	return (0);
}

/*
 * Check for a looping message
 * This is done by checking for a matching line
 * in the email headers for Delivered-To: which
 * we put in each email
 * 
 * Return  1 if looping
 * Return  0 if not looping
 */
static int
is_looping(char *address)
{
	static stralloc line = {0};
	int             i, found, match;
	char           *dtline, *ptr;

	if (*address == '&')
		address++;
	/*- check the DTLINE */
	if ((dtline = (char *) env_get("DTLINE")) != (char *) 0)
		ptr = str_str(dtline, address);
	else
		ptr = 0;
	if (dtline && ptr && !str_diff(ptr, address))
		return (1);
	if (!(ptr = env_get("MAKE_SEEKABLE")) || *ptr == '0')
		return (0);
	if (lseek(0, 0L, SEEK_SET) < 0) {
		strerr_warn1("deliver_mail: is_looping: lseek: ", 0);
		return (-1);
	}
	for (;;) {
		if (getln(subfdin, &line, &match, '\n') == -1) {
			strerr_warn1("deliver_mail: read-email: ", &strerr_sys);
			return (-1);
		}
		if (!match || !line.len)
			break;
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
		if (!str_diffn(line.s, "Delivered-To:", 13)) {
			if (str_str(line.s, address))
				return (1);
		} else {
			/*
			 * check for the start of the body, we only need
			 * to check the headers. 
			 *
			 * walk through the charaters in the body 
			 */
			for (i = 0, found = 0; line.s[i] && !found; ++i) {
				switch (line.s[i])
				{
					/*- skip blank spaces and new lines */
				case ' ':
				case '\n':
				case '\t':
				case '\r':
					break;
				default:
					/*
					 * found a non blank, so we are still
					 * in the headers
					 * set the found non blank char flag 
					 */
					found = 1;
					break;
				}
			}
			/*
			 * if the line only had blanks, then it is the
			 * delimiting line between the headers and the
			 * body. We don't need to check the body for
			 * the duplicate Delivered-To: line. Hence, we
			 * are done with our search and can return the
			 * looping not found value return not found looping 
			 * message value 
			 */
			if (found == 0)
				return (0);
		}
	}
	/*
	 * if we get here then the there is either no body 
	 * or the logic above failed and we scanned
	 * the whole email, headers and body. 
	 */
	return (0);
}

/* 
 * open a pipe to a command 
 * return the pid or -1 if error
 */

static int
open_command(char *command, int *write_fd)
{
	int             pim[2];
	int             i, len;
	long unsigned   pid;
	static stralloc cmmd = {0}, ncmmd = {0};
	char           *p, *r, *binqqargs[4];
	char          **q;
	char *special_programs[] = { 
		"autoresponder",
		"condredirect",
		"condtomaildir",
		"dot-forward",
		"fastforward",
		"filterto",
		"forward",
		"maildirdeliver",
		"preline",
		"qnotify",
		"qsmhook",
		"replier",
		"rrforward",
		"serialcmd",
		0
	};

	/*- skip over an | sign if there */
	*write_fd = -1;
	if (*command == '|')
		++command;
	if (pipe(pim) == -1)
		return (-1);
	if (!stralloc_copys(&cmmd, command) || !stralloc_0(&cmmd))
		die_nomem();
	switch (pid = fork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (-1);
	case 0:
		if (env_get("QHPSI") && !env_unset("QQEH"))
			strerr_die1sys(111, "deliver_mail: env_unset: QQEH: ");
		for (r = cmmd.s; *r && isspace(*r);r++);
		/*- copy only the first command argument (i.e. argv[0]) */
		for (len = 0; *r && !isspace(*r); len++, r++);
		if (!stralloc_copyb(&ncmmd, cmmd.s, len) || !stralloc_0(&ncmmd))
			die_nomem();
		i = str_rchr(ncmmd.s, '/');
		if (ncmmd.s[i])
			p = ncmmd.s + i + 1;
		else
			p = ncmmd.s;
		for (q = special_programs; *q; q++) {
			if (!str_diff(p, *q))
				break;
		}
		if (!*q) { /*- preserve RPLINE, DTLINE for special programs */
			if (env_get("RPLINE") && !env_unset("RPLINE"))
				strerr_die1sys(111, "deliver_mail: env_unset: RPLINE: ");
			if (env_get("DTLINE") && !env_unset("DTLINE"))
				strerr_die1sys(111, "deliver_mail: env_unset: DTLINE: ");
		}
		close(pim[1]);
		if (dup2(pim[0], 0) == -1)
			_exit(-1);
#ifdef MAKE_SEEKABLE
		if ((p = env_get("MAKE_SEEKABLE")) && *p != '0' && makeseekable(0)) {
			strerr_warn1("deliver_mail: makeseekable: ", &strerr_sys);
			_exit(111);
		}
#endif
		binqqargs[0] = "/bin/sh";
		binqqargs[1] = "-c";
		binqqargs[2] = cmmd.s;
		binqqargs[3] = 0;
		execv(*binqqargs, binqqargs);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	*write_fd = pim[1];
	close(pim[0]);
	return (pid);
}

static void
format_delivered_to1(stralloc *delivered_to, char *rpline, char *dtline,
	char *qqeh, char *xfilter, char *domain, char *email, char *address)
{
	if (!stralloc_copys(delivered_to, rpline))
		die_nomem();
	if (dtline) {
		if (!stralloc_cats(delivered_to, dtline))
			die_nomem();
	} else {
		if (!stralloc_catb(delivered_to, "Delivered-To: ", 14) ||
				!stralloc_cats(delivered_to, domain) ||
				!stralloc_append(delivered_to, "-") ||
				!stralloc_cats(delivered_to, (email && *email) ? email : address) ||
				!stralloc_append(delivered_to, "\n"))
			die_nomem();
	}
	if (qqeh && !stralloc_cats(delivered_to, qqeh))
		die_nomem();
	if (!stralloc_cats(delivered_to, xfilter ? xfilter : "X-Filter: None") ||
			!stralloc_append(delivered_to, "\n") ||
			!stralloc_catb(delivered_to, "Received: (indimail ", 20) ||
			!stralloc_catb(delivered_to, strnum, fmt_ulong(strnum, getpid())) ||
			!stralloc_catb(delivered_to, " invoked by uid ", 16) ||
			!stralloc_catb(delivered_to, strnum, fmt_ulong(strnum, getuid())) ||
			!stralloc_catb(delivered_to, "); ", 3) ||
			!stralloc_cats(delivered_to, get_localtime()) ||
			!stralloc_append(delivered_to, "\n") ||
			!stralloc_0(delivered_to))
		die_nomem();
	delivered_to->len--; /*- substract the length due to null byte */
	return;
}

static void
format_delivered_to2(stralloc *delivered_to, char *faddr, char *dtline,
	char *qqeh, char *xfilter)
{
	if (!stralloc_copyb(delivered_to, "X-Forwarded-To: ", 16) ||
			!stralloc_cats(delivered_to, faddr) ||
			!stralloc_append(delivered_to, "\n") ||
			!stralloc_catb(delivered_to, "X-Forwarded-For: ", 17) ||
			!stralloc_cats(delivered_to, dtline) ||
			!stralloc_append(delivered_to, "\n"))
		die_nomem();
	if (qqeh && !stralloc_cats(delivered_to, qqeh))
		die_nomem();
	if (!stralloc_cats(delivered_to, xfilter ? xfilter : "X-Filter: None") ||
			!stralloc_append(delivered_to, "\n") ||
			!stralloc_catb(delivered_to, "Received: (indimail ", 20) ||
			!stralloc_catb(delivered_to, strnum, fmt_ulong(strnum, getpid())) ||
			!stralloc_catb(delivered_to, " invoked by uid ", 16) ||
			!stralloc_catb(delivered_to, strnum, fmt_ulong(strnum, getuid())) ||
			!stralloc_catb(delivered_to, "); ", 3) ||
			!stralloc_cats(delivered_to, get_localtime()) ||
			!stralloc_append(delivered_to, "\n") ||
			!stralloc_0(delivered_to))
		die_nomem();
	delivered_to->len--; /*- substract the length due to null byte */
	return;
}

static void
format_local_filename(stralloc *file, stralloc *file_new, char *address,
	stralloc *hostname, time_t tm, pid_t pid, mdir_t MsgSize)
{
	int             i;

	if (!stralloc_copys(file, address) ||
			!stralloc_catb(file, "tmp/", 4))
		die_nomem();
	i = file->len;
	if (!stralloc_catb(file, strnum, fmt_ulong(strnum, tm)) ||
			!stralloc_append(file, ".") ||
			!stralloc_catb(file, strnum, fmt_ulong(strnum, pid)) ||
			!stralloc_append(file, ".") ||
			!stralloc_cat(file, hostname) ||
			!stralloc_catb(file, ",S=", 3) ||
			!stralloc_catb(file, strnum, fmt_ulong(strnum, MsgSize)) ||
			!stralloc_0(file))
		die_nomem();
	if (!stralloc_copy(file_new, file))
		die_nomem();
	str_copyb(file_new->s + i - 4, "new", 3);
}

/* 
 * Deliver an email to an address
 * Return 0 on success
 * Return less than zero on failure
 * 
 * -1 user is over quota
 * -2 system errors
 * -3 mail is looping 
 * -4 mail is over quota due to limits (MAILSIZE_LIMIT or MAILCOUNT_LIMIT)
 * -5 defer over quota mail instead of bouncing
 */
int
deliver_mail(char *address, mdir_t MsgSize, char *quota, uid_t uid, gid_t gid, 
	char *Domain, mdir_t *QuotaCount, mdir_t *QuotaSize)
{
	time_t          tm;
	int             i, wait_status, write_fd, is_injected, is_file, sfd, code,
					ret, fd, match;
	static stralloc local_file = {0}, local_file_new = {0}, hostname = {0},
	                DeliveredTo = {0}, user = {0}, homedir = {0},
					date_dir = {0};
	substdio        ssin, ssout;
	char            inbuf[512], outbuf[512];
	char           *maildirquota, *domain, *email, *rpline, *dtline, *xfilter, *ptr,
				   *alert_host = 0, *alert_port = 0, *cmmd, *controldir, *sysconfdir,
				   *ptr1, *ptr2, *qqeh, *faddr;
	char           *MailDirNames[] = {
		"cur",
		"new",
		"tmp",
	};
	mdir_t         *MailQuotaCount, *MailQuotaSize;
	mdir_t          _MailQuotaCount, _MailQuotaSize, quota_mailsize, dailyMsgCount, dailyMsgSize;
	static int      counter;
	struct stat     statbuf;
	struct passwd  *pw;
	long unsigned   pid;

	if (QuotaCount)
		MailQuotaCount = QuotaCount;
	else
		MailQuotaCount = &_MailQuotaCount;
	*MailQuotaCount = 0;
	if (QuotaSize)
		MailQuotaSize = QuotaSize;
	else
		MailQuotaSize = &_MailQuotaSize;
	is_injected = 0;
	/*- check if the email is looping to this user */
	if ((i = is_looping(address)) == 1) {
		strerr_warn2(address, " is looping", 0);
		return (-3);
	} else
	if (i == -1)
		return (-2);
	CurBytes = CurCount = 0;
	/*- This is a directory/Maildir location */
	if (*address == '/') {
		cmmd = "";
		is_file = 1;
		getEnvConfigStr(&rpline, "RPLINE", "Return-PATH: <>\n");
		dtline = env_get("DTLINE");
		xfilter = env_get("XFILTER");
		qqeh = env_get("QQEH");
		if (stat(address, &statbuf)) {
			strerr_warn3("address ", address, ": ", &strerr_sys);
			return (-2);
		}
		if (statbuf.st_mode & S_ISVTX) {
			errno = EAGAIN;
			strerr_warn3("dir ", address, " is sticky ", 0);
			return (-2);
		}
		email = maildir_to_email(address, Domain);
		format_delivered_to1(&DeliveredTo, rpline, dtline, qqeh, xfilter, Domain, email, address);
		MsgSize += DeliveredTo.len;
		ptr1 = env_get("MAILSIZE_LIMIT");
		ptr2 = env_get("MAILCOUNT_LIMIT");
		if ((ptr1 && *ptr1) || (ptr2 && *ptr2)) {
			if ((ret = recordMailcount(address, MsgSize, &dailyMsgSize, &dailyMsgCount)) == -1) {
				getEnvConfigStr(&ptr, "OVERQUOTA_CMD", LIBEXECDIR"/overquota.sh");
				if (!access(ptr, X_OK)) {
					/*
					 * Call overquota command with 5 arguments
					 * address currMessSize dailyMsgSize dailyMsgCount "dailySizeLimit"S,"dailyCountLimit"C
					 */
					if (!stralloc_copys(&local_file, ptr) ||
							!stralloc_append(&local_file, " ") ||
							!stralloc_cats(&local_file, address) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					strnum[i = fmt_ulong(strnum, (unsigned long) MsgSize)] = 0;
					if (!stralloc_catb(&local_file, strnum, i) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					strnum[i = fmt_ulong(strnum, (unsigned long) dailyMsgSize)] = 0;
					if (!stralloc_catb(&local_file, strnum, i) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					strnum[i = fmt_ulong(strnum, (unsigned long) dailyMsgCount)] = 0;
					if (!stralloc_catb(&local_file, strnum, i) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					if (!stralloc_cats(&local_file, ptr1 ? ptr1 : "0S") ||
							!stralloc_append(&local_file, ",") ||
							!stralloc_cats(&local_file, ptr2 ? ptr2 : "0C") ||
							!stralloc_0(&local_file))
						die_nomem();
					runcmmd(local_file.s, 0);
				}
				return (-4);
			} else
			if (ret) /*- system error */
				return (ret);
		}
		domain = 0;
		if (email && *email) {
			if (!stralloc_copys(&user, email) || !stralloc_0(&user))
				die_nomem();
			i = str_chr(user.s, '@');
			if (user.s[i]) {
				user.s[i] = 0;
				user.len = i;
				domain = user.s + i + 1;
			} else
				getEnvConfigStr(&domain, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		}
		if (!str_diffn(quota, "AUTO", 5)) {
			maildirquota = (char *) 0;
#ifdef USE_MAILDIRQUOTA
			/*- if the user has a quota set */
			i = str_len(address);
			if (address[i - 1] == '/')
				i--;
			if (!stralloc_copyb(&homedir, address, i) ||
					!stralloc_catb(&homedir, "/maildirsize", 12) ||
					!stralloc_0(&homedir))
				die_nomem();
			homedir.len--;
			if (!access(homedir.s, F_OK)) {
				if (!(maildirquota = read_quota(homedir.s)))
					return (-2);
			}  else
#endif
			/*
			 * Try to figure out the quota from the username
			 */
			if (!email || !*email || !(pw = sql_getpw(user.s, domain)))
				maildirquota = "NOQUOTA";
			else
				maildirquota = pw->pw_shell;
			if ((ptr = str_str(homedir.s, "/maildirsize"))) {
				*ptr = 0;
				homedir.len -= 12;
			}
		} else {
			maildirquota = quota;
			if (!stralloc_copys(&homedir, address) || !stralloc_0(&homedir))
				die_nomem();
			homedir.len--;
		}
		if (str_diffn(maildirquota, "NOQUOTA,", 8)) {
			/*
			 * If the user has insufficient quota to accept
			 * the current message and the msg size < OVERQUOTA_MAILSIZE bytes
			 * accept it. Else bounce it back.
			 * If the user has already exceeded quota, update
			 * userquota table and set the BOUNCE_MAIL flag
			 * if MAILDROP is defined use the format - MAILDIRQUOTA="5000000S,200C"
			 */
			if ((ret = user_over_quota(address, maildirquota, MsgSize)) == -1)
				return (-2);
			if (ret == 1) {
				getEnvConfigStr(&ptr, "OVERQUOTA_CMD", LIBEXECDIR"/overquota.sh");
				if (!access(ptr, X_OK)) {
					/*
					 * Call overquota command with 6 arguments (including last dummy)
					 * email message_size curr_usage curr_count maildirquota "dummy"
					 */
					if (!stralloc_copys(&local_file, ptr) ||
							!stralloc_append(&local_file, " ") ||
							!stralloc_cats(&local_file, address) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					strnum[i = fmt_ulong(strnum, (unsigned long) MsgSize)] = 0;
					if (!stralloc_catb(&local_file, strnum, i) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					strnum[i = fmt_ulong(strnum, (unsigned long) CurBytes)] = 0;
					if (!stralloc_catb(&local_file, strnum, i) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					strnum[i = fmt_ulong(strnum, (unsigned long) CurCount)] = 0;
					if (!stralloc_catb(&local_file, strnum, i) ||
							!stralloc_append(&local_file, " "))
						die_nomem();
					if (!stralloc_cats(&local_file, maildirquota) ||
							!stralloc_catb(&local_file, " dummy", 6) ||
							!stralloc_0(&local_file))
						die_nomem();
					runcmmd(local_file.s, 0);
				}
				/*
				 * Defer Mail if holdoverquota file is present
				 */
				getEnvConfigStr(&ptr, "HOLDOVERQUOTA", "holdoverquota");
				if (ptr && *ptr == '/') {
					if (!stralloc_copys(&local_file, ptr) || !stralloc_0(&local_file))
						die_nomem();
				} else {
					if (!stralloc_copy(&local_file, &homedir) ||
							!stralloc_append(&local_file, "/") ||
							!stralloc_cats(&local_file, ptr) ||
							!stralloc_0(&local_file))
						die_nomem();
				}
				if (!access(local_file.s, F_OK))
					return (-5); /*- defer overquota mail */
				/*- Update quota in userquota and BOUNCE_MAIL in indimail */
				if (email && *email) {
#ifdef USE_MAILDIRQUOTA
					if ((*MailQuotaSize = parse_quota(maildirquota, MailQuotaCount)) == -1) {
						strerr_warn3("deliver_mail: parse_quota: ", maildirquota, ": ", &strerr_sys);
						return (-2);
					} else
					if (*MailQuotaSize) {
						if (CurBytes > *MailQuotaSize || (*MailQuotaCount && (CurCount > *MailQuotaCount)))
							vset_lastdeliver(user.s, domain, CurBytes);
					}
#else
					scan_ulong(maildirquota, MailQuotaSize);
					if (*MailQuotaSize && CurBytes > *MailQuotaSize)
						vset_lastdeliver(user.s, domain, CurBytes); 
#endif
				}
				/*- 
				 * Bounce if Message size is greater 
				 * than quota_mailsize bytes
				 */
				if ((ptr = env_get("OVERQUOTA_MAILSIZE")) && *ptr)
					scan_ulong(ptr, (unsigned long *) &quota_mailsize);
				else
					quota_mailsize = OVERQUOTA_MAILSIZE;
				if (MsgSize > quota_mailsize && email && *email) {
					MailQuotaWarn(user.s, domain, address, maildirquota);
					return (-1);
				}
			} 
			/*
			 * If we are going to deliver it, then add in the size 
			 */
			update_quota(address, MsgSize);
			if (email && *email) {
				MailQuotaWarn(user.s, domain, address, maildirquota);
				getAlertConfig(&alert_host, &alert_port);
				if (!alert_host)
					alert_host = env_get("MAILALERT_HOST");
				if (!alert_port)
					alert_port = env_get("MAILALERT_PORT");
				if (alert_host && alert_port && *alert_host && *alert_port) {
					if ((sfd = udpopen(alert_host, alert_port)) != -1) {
						substdio_fdbuf(&ssout, write, sfd, outbuf, sizeof(outbuf));
						strnum[i = fmt_ulong(strnum, MsgSize)] = 0;
						if (substdio_put(&ssout, user.s, user.len) ||
								substdio_put(&ssout, "@", 1) ||
								substdio_puts(&ssout, domain) ||
								substdio_put(&ssout, strnum, i) ||
								substdio_put(&ssout, "\n", 1) ||
								substdio_flush(&ssout) == -1)
							strerr_warn1("deliver_mail: write error: alert_host: ", &strerr_sys);
						close(sfd);
					}
				}
			}
		} else
			CurBytes = CurCount = 0;
#ifdef HAVE_SSL 
		if (env_get("ELIMINATE_DUPS") && ismaildup(address)) {
			strerr_warn1("deliver_mail: discarding duplicate msg", 0);
			return (0);
		}
#endif
		/*- Format the email file name */
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		if (*controldir == '/') {
			if (!stralloc_copys(&tmpbuf, controldir) ||
					!stralloc_catb(&tmpbuf, "/me", 3) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
		} else {
			getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
			if (!stralloc_copys(&tmpbuf, sysconfdir) ||
					!stralloc_append(&tmpbuf, "/") ||
					!stralloc_cats(&tmpbuf, controldir) ||
					!stralloc_catb(&tmpbuf, "/me", 3) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
		}
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno != error_noent)
				strerr_die3sys(111, "deliver_mail: open: ", tmpbuf.s, ": ");
			return (-2);
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &hostname, &match, '\n') == -1) {
			strerr_warn2(tmpbuf.s, ": EOF. indimail (#5.1.2): ", &strerr_sys);
			close(fd);
			return (-2);
		}
		if (!match || !hostname.len) {
			strerr_warn2(tmpbuf.s, ": bad line. indimail (#5.1.2): ", &strerr_sys);
			close(fd);
			return (-2);
		}
		hostname.len--;
		hostname.s[hostname.len] = 0; /*- remove newline */
		close(fd);
		pid = getpid() + counter++;
		umask(0077);
		time(&tm);
		if (!stralloc_copys(&local_file, address) ||
				!stralloc_catb(&local_file, "folder.dateformat", 17) ||
				!stralloc_0(&local_file))
			die_nomem();
		if (!access(local_file.s, R_OK)) {
			if ((fd = open_read(local_file.s)) == -1) {
				if (errno != error_noent)
					strerr_warn3("deliver_mail: open: ", local_file.s, ": ", &strerr_sys);
				return (-2);
			}
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &date_dir, &match, '\n') == -1) {
				strerr_warn2(local_file.s, ": EOF. indimail (#5.1.2): ", &strerr_sys);
				close(fd);
				return (-2);
			}
			if (!match || !date_dir.len) {
				strerr_warn2(local_file.s, ": bad line. indimail (#5.1.2): ", &strerr_sys);
				close(fd);
				return (-2);
			}
			date_dir.len--;
			date_dir.s[date_dir.len] = 0; /*- remove newline */
			close(fd);
			if (!(ret = dateFolder(tm, &tmpbuf, date_dir.s))) {
				if (!stralloc_copys(&local_file, address) ||
						!stralloc_cat(&local_file, &tmpbuf) ||
						!stralloc_0(&local_file))
					die_nomem();
				if (access(local_file.s, F_OK)) {
					if (r_mkdir(local_file.s, INDIMAIL_DIR_MODE, uid, gid)) {
						strerr_warn5("deliver_mail: r_mkdir: ", local_file.s, ": ", error_str(errno), ": indimail (#5.1.2)", 0);
						return (-2);
					} else {
						for (i = 0; i < 3; i++) {
							if (!stralloc_copys(&local_file, address) ||
									!stralloc_cat(&local_file, &tmpbuf) ||
									!stralloc_append(&local_file, "/") ||
									!stralloc_catb(&local_file, MailDirNames[i], 3) ||
									!stralloc_0(&local_file))
								die_nomem();
							if (r_mkdir(local_file.s, INDIMAIL_DIR_MODE, uid, gid)) {
								strerr_warn5("deliver_mail: r_mkdir: ", local_file.s, ": ", error_str(errno), ": indimail (#5.1.2)", 0);
								return (-2);
							}
						}
					}
				} else {
					for (i = 0; i < 3; i++) {
						if (!stralloc_copys(&local_file, address) ||
								!stralloc_cat(&local_file, &tmpbuf) ||
								!stralloc_append(&local_file, "/") ||
								!stralloc_catb(&local_file, MailDirNames[i], 3) ||
								!stralloc_0(&local_file))
							die_nomem();
						if (access(local_file.s, F_OK)) {
							if (r_mkdir(local_file.s, INDIMAIL_DIR_MODE, uid, gid)) {
								strerr_warn5("deliver_mail: r_mkdir: ", local_file.s, ": ", error_str(errno), ": indimail (#5.1.2)", 0);
								return (-2);
							}
						}
					}
				}
			} else {
				strerr_warn3("deliver_mail: dateFolder: failed to get date format: ", error_str(errno), ": indimail (#5.1.2)", 0);
				return (-2);
			}
		}
		format_local_filename(&local_file, &local_file_new, address, &hostname, tm, pid, MsgSize);
		if ((write_fd = open(local_file.s, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
			strerr_warn3("deliver_mail: open: ", local_file.s, ": ", &strerr_sys);
			return (-2);
		}
		if ((!getuid() || !geteuid()) && fchown(write_fd, uid, gid)) {
			strerr_warn3("deliver_mail: fchown: ", local_file.s, ": ", &strerr_sys);
			return (-2);
		}
		/*- write the Return-Path: and Delivered-To: headers */
		if (write(write_fd, DeliveredTo.s, DeliveredTo.len) != DeliveredTo.len) {
			/*- Check if the user is over quota */
			if (errno == EDQUOT) {
				close(write_fd);
				return (-1);
			} else {
				strerr_warn1("deliver_mail: write: delivered to line: ", &strerr_sys);
				close(write_fd);
				return (-2);
			}
		}
	} else
	if (*address == '|') {
		is_file = 0;
		/*- This is an command */
		is_injected = 1;
		/*- open up a pipe to a command */
		if ((pid = open_command(address, &write_fd)) < 0) {
			strerr_warn3("deliver_mail: open_cmmd: ", address, ": ", &strerr_sys);
			return (-2);
		}
		for (cmmd = address + 1;*cmmd && isspace((int) *cmmd);cmmd++);
	} else { /*- must be an email address */
		is_file = 0;
		is_injected = 1;
		cmmd = "qmail-inject";
		/*- exec qmail-inject and connect the file descriptors */
		if ((pid = qmail_inject_open(address, &write_fd)) < 0) {
			strerr_warn3("deliver_mail: qmail-inject: ", address, ": ", &strerr_sys);
			return (-2);
		}
	}
	if (is_injected) {
		char           *tstr = 0;
		if ((dtline = env_get("DTLINE"))) { /*- original address */
			for (;*dtline && *dtline != ':';dtline++);
			if (*dtline)
				dtline++;
			for (;*dtline && isspace((int) *dtline);dtline++);
			for (tstr = dtline;*tstr && *tstr != '\n';tstr++);
			if (*tstr == '\n')
				*tstr = 0; /*- newline removed from dtline */
			if (!*dtline)
				dtline = (*address == '&') ? address + 1: address;
		} else
		if (!(dtline = env_get("RECIPIENT")))
			dtline = "pipe";
		faddr = *address == '&' ? address + 1 : address;
		xfilter = env_get("XFILTER");
		qqeh = env_get("QQEH");
		format_delivered_to2(&DeliveredTo, faddr, dtline, qqeh, xfilter);
		if (tstr) /*- replace the newline in DTLINE environment variable */
			*tstr = '\n';
		if (write(write_fd, DeliveredTo.s, DeliveredTo.len) != DeliveredTo.len) {
			/*- Check if the user is over quota */
			if (errno == EDQUOT) {
				close(write_fd);
				return (-1);
			} else {
				strerr_warn1("deliver_mail: write: delivered to line: ", &strerr_sys);
				close(write_fd);
				return (-2);
			}
		}
	}
	/*- start the the begining of the email file */
	if ((ptr = env_get("MAKE_SEEKABLE")) && *ptr != '0' && lseek(0, 0L, SEEK_SET) < 0) {
		strerr_warn1("deliver_mail: lseek: ", &strerr_sys);
		close(write_fd);
		return (-2);
	}
	substdio_fdbuf(&ssout, write, write_fd, outbuf, sizeof(outbuf));
	for (;;) {
		if ((i = read(0, inbuf, sizeof(inbuf))) == -1) {
			strerr_warn1("deliver_mail: read error: ", &strerr_sys);
			close(write_fd);
			if (is_file == 1 && unlink(local_file.s)) {
				strerr_warn3("deliver_mail: unlink-", local_file.s, ": ", &strerr_sys);
				return (-2);
			}
			return (-2);
		} else
		if (!i)
			break;
		if (substdio_put(&ssout, inbuf, i) == -1) {
			strerr_warn1("deliver_mail: write error: ", &strerr_sys);
			close(write_fd);
			if (is_file == 1 && unlink(local_file.s)) {
				strerr_warn3("deliver_mail: unlink-", local_file.s, ": ", &strerr_sys);
				return (-2);
			}
			return (-2);
		}
	}
	if (substdio_flush(&ssout) == -1) {
		strerr_warn1("deliver_mail: write error: ", &strerr_sys);
		close(write_fd);
		if (is_file == 1 && unlink(local_file.s)) {
			strerr_warn3("deliver_mail: unlink-", local_file.s, ": ", &strerr_sys);
			return (-2);
		}
		return (-2);
	}
	/*- if we are writing to a Maildir, move it into the new directory */
	if (is_file == 1) {
		/*- sync the data to disk and close the file */
		errno = 0;
		if (
#ifdef FILE_SYNC
#ifdef HAVE_FDATASYNC
			   fdatasync(write_fd) == 0 &&
#else
			   fsync(write_fd) == 0 &&
#endif
#endif
			   close(write_fd) == 0)
		{
#ifdef USE_LINK
			/*- if this succeeds link the file to the new directory */
			if (link(local_file.s, local_file_new.s)) {
				/*- coda fs has problems with link, check for that error */
				if (errno == EXDEV) {
					/*- try to rename the file instead */
					if (rename(local_file.s, local_file_new.s) != 0) {
						/*- even rename failed, time to give up */
						strerr_warn5("deliver_mail: rename ", local_file.s, " --> ", local_file_new.s, ": ", &strerr_sys);
						unlink(local_file.s);

						return (-2);
						/*- rename worked, so we are okay now */
					} else
						errno = 0;
				} else /*- link failed and we are not on coda */
					strerr_warn5("deliver_mail: link ", local_file.s, " --> ", local_file_new.s, ": ", &strerr_sys);
				if (unlink(local_file.s))
					strerr_warn3("deliver_mail: unlink-", local_file.s, ": ", &strerr_sys);
			} else
			if (unlink(local_file.s))
				strerr_warn3("deliver_mail: unlink-", local_file.s, ": ", &strerr_sys);
#else
			if (rename(local_file.s, local_file_new.s) != 0) {
				strerr_warn5("deliver_mail: rename ", local_file.s, " --> ", local_file_new.s, ": ", &strerr_sys);
				return (-2);
			} else
				errno = 0;
#endif /*- #ifdef USE_LINK */
		} /* Data Sync */
		/*- if any errors occured then return the error */
		if (errno != 0)
			return (-2);
	} else
	if (is_injected == 1) {
		/*
		 * If we were writing it to qmail-inject
		 * then wait for qmail-inject to finish 
		 */
		close(write_fd);
		for (;;) {
			pid = wait(&wait_status);
#ifdef ERESTART
			if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else
			if (pid == -1 && errno == EINTR)
#endif
				continue;
			else
			if (pid == -1) {
				strerr_warn1("qmail-inject crashed. indimail bug", 0);
				return (111);
			}
			break;
		}
		if (WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status)) {
			strerr_warn1("qmail-inject crashed.", 0);
			return (111);
		} else
		if (WIFEXITED(wait_status)) {
			switch (code = WEXITSTATUS(wait_status))
			{
			case 0:
				return (0);
			case 99:
				return (99);
			case 64:
			case 65:
			case 70:
			case 76: 
			case 77: 
			case 78: 
			case 112:
			case 100:
				strnum[fmt_uint(strnum, code)] = 0;
				strerr_warn5("deliver_mail: ", *address == '|' ? cmmd : address, " failed code(", strnum, "). Permanent error", 0);
				_exit(100);
			default:
				strnum[fmt_uint(strnum, code)] = 0;
				strerr_warn5("deliver_mail: ", *address == '|' ? cmmd : address, " failed code(", strnum, "). Temporary error", 0);
				_exit(111);
			}
		}
	}
	return (0);
}
