/*
 * $Log: qmailmrtg.c,v $
 * Revision 1.3  2023-04-07 22:32:47+05:30  Cprogrammer
 * converted queue messages to messages/hour
 *
 * Revision 1.2  2023-04-07 22:24:37+05:30  Cprogrammer
 * refactored to use libqmail
 *
 * Revision 1.1  2019-04-18 08:36:13+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 *
 * kbo@inter7.com
 * Wrote everything except what is listed below.
 *
 * Richard A. Secor <rsecor@seqlogic.com>
 * Added support for rblsmtpd deny
 *
 * Modified for IndiMail by Manvendra Bhangui <manvendra@indimail.org>
 */
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <substdio.h>
#include <subfd.h>
#include <qprintf.h>
#include <stralloc.h>
#include <strerr.h>
#include <open.h>
#include <getln.h>
#include <str.h>
#include <env.h>
#include <scan.h>

#ifndef lint
static char     sccsid[] = "$Id: qmailmrtg.c,v 1.3 2023-04-07 22:32:47+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL "qmailmrtg: fatal: "
#define WARN  "qmailmrtg: warn: "

int             BigTodo=1, ConfSplit=151, cconcurrency, tconcurrency,
				tallow, tdeny, ttotal, success, failure, deferral, unsub,
				local, remote, max_files, clicked, viewed, cfound, cerror,
				tspam, tclean, tcached, tquery, mess_count, todo_count;
time_t          end_time, start_time;
static stralloc tmp, thedir, thefile;
unsigned long   bytes, tmpulong;
int             debug;
char            TheType;
static char    *usage =
	"usage: qmailmrtg type dir\n"
	"where type is one of t, a, m, c, s, b, q, Q, l, u, v, S, C, d\n"
	"and dir is a directory containing multilog files\n"
	"for q and Q option dir is the qmail queue base dir"
	;

int
count_files(char *diri)
{
	DIR            *mydir;
	int             count;
	struct dirent  *mydirent;

	if (!(mydir = opendir(diri)))
		return (0);
	for (count = 0; (mydirent = readdir(mydir)) != NULL;) {
		if (str_diffn(mydirent->d_name, ".", 2) && str_diffn(mydirent->d_name, "..", 3))
			++count;
	}
	closedir(mydir);
	return (count);
}

int
get_counts(char *diri)
{
	int             i, count;

	for (i = 0, count = 0; i < ConfSplit; ++i) {
		qsprintf(&tmp, "%s/%d", diri, i);
		count += count_files(tmp.s);
	}
	return (count);
}

unsigned long
get_tai(char *tmpstr)
{
	unsigned long   secs;
	unsigned long   nanosecs;
	unsigned long   u;

	secs = 0;
	nanosecs = 0;

	for (; *tmpstr != 0; ++tmpstr) {
		u = *tmpstr - '0';
		if (u >= 10) {
			u = *tmpstr - 'a';
			if (u >= 6)
				break;
			u += 10;
		}
		secs <<= 4;
		secs += nanosecs >> 28;
		nanosecs &= 0xfffffff;
		nanosecs <<= 4;
		nanosecs += u;
	}
	secs -= 4611686018427387914ULL;

	return (secs);
}

void
process_file(char *file_name)
{
	unsigned long   secs;
	int             fd, match, tmpint;
	char           *t1, *t2;
	static stralloc line = {0};
	char            inbuf[4096];
	struct substdio ssin;

	if ((fd = open_read(file_name)) == -1) {
		strerr_warn4(WARN, "error opening file ", file_name, "for reading: ", &strerr_sys);
		return;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn4(WARN, "read: ", file_name, ": ", &strerr_sys);
			close(fd);
			return;
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
		} else {
			if (!stralloc_0(&line)) {
				strerr_warn2(WARN, "out of memory", 0);
				close(fd);
				return;
			}
			line.len--;
		}
		if (line.s[0] != '@')
			continue;
		secs = get_tai(line.s + 1);
		if (secs < start_time || secs > end_time)
			continue;
		switch (TheType)
		{
		case 'S':
			if ((t1 = str_str(line.s, "X-Bogosity: Yes")) != NULL)
				++tspam;
			else
			if ((t1 = str_str(line.s, "X-Bogosity: No")) != NULL)
				++tclean;
			if (debug) {
				subprintf(subfderr, "tspam %d tclean %d\n", tspam, tclean);
				substdio_flush(subfderr);
			}
			break;
		case 'C':
			if ((t1 = str_str(line.s, "FOUND")) != NULL)
				++cfound;
			else
			if ((t1 = str_str(line.s, "ERROR")) != NULL)
				++cerror;
			if (debug) {
				subprintf(subfderr, "cfound %d cerror %d\n", cfound, cerror);
				substdio_flush(subfderr);
			}
			break;
		case 't':
			if ((t1 = str_str(line.s, "status:")) != NULL) {
				while (*t1 != ':')
					++t1;
				++t1;

				for (t2 = t1 + 1; *t2 != '/'; ++t2);
				*t2 = 0;
				scan_int(t1, &tmpint);
				if (tmpint > cconcurrency)
					cconcurrency = tmpint;
				scan_int(t2 + 1, &tmpint);
				if (tmpint > tconcurrency)
					tconcurrency = tmpint;
			}
			if (debug) {
				subprintf(subfderr, "cconcurrency %d toncurrency %d\n", cconcurrency, tconcurrency);
				substdio_flush(subfderr);
			}
			break;
		case 'a':
			if ((t1 = str_str(line.s, " ok ")) != NULL)
				++tallow;
			else
			if ((t1 = str_str(line.s, " deny ")) != NULL)
				++tdeny;
			else
			if ((t1 = str_str(line.s, " rblsmtpd:")) != NULL)
				++tdeny;
			if (debug) {
				subprintf(subfderr, "tallow %d tdeny %d\n", tallow, tdeny);
				substdio_flush(subfderr);
			}
			break;
		case 'c': /*- @40000000642a387d2dede194 status: local 0/10 remote 0/20 queue2 */
			if ((t1 = str_str(line.s, " status: ")) != NULL) {
				while (*t1 != '/' && *t1 != 0)
					++t1;
				*t1 = 0;
				t2 = t1 + 1;
				--t1;
				while (*t1 != ' ')
					--t1;
				++t1;
				scan_int(t1, &tmpint); /*- local concurrency */
				if (tmpint > local)
					local = tmpint;

				while (*t2 != '/' && *t2 != 0)
					++t2;
				*t2 = 0;
				--t2;
				while (*t2 != ' ')
					--t2;
				++t2;
				scan_int(t2, &tmpint); /*- remote concurrency */
				if (tmpint > remote)
					remote = tmpint;
			}
			if (debug) {
				subprintf(subfderr, "local %d remote %d\n", local, remote);
				substdio_flush(subfderr);
			}
			break;
		case 's':
		case 'm':
			if (str_str(line.s, "success:"))
				success++;
			if (str_str(line.s, "failure:"))
				failure++;
			if (str_str(line.s, "deferral:"))
				deferral++;
			if (debug) {
				subprintf(subfderr, "success %d failure %d deferral %d\n", success, failure, deferral);
				substdio_flush(subfderr);
			}
			break;
		case 'v':
			if (str_str(line.s, "viewed:"))
				viewed++;
			if (str_str(line.s, "clicked:"))
				clicked++;
			if (debug) {
				subprintf(subfderr, "viewed %d clicked %d\n", viewed, clicked);
				substdio_flush(subfderr);
			}
			break;
		case 'u':
			if (str_str(line.s, "unsub:"))
				unsub++;
			if (debug) {
				subprintf(subfderr, "unsub %d\n", unsub);
				substdio_flush(subfderr);
			}
			break;
		case 'l':
			++ttotal;
			if (debug) {
				subprintf(subfderr, "ttotal %d\n", ttotal);
				substdio_flush(subfderr);
			}
			break;
		case 'd':
			if (str_str(line.s, "cached"))
				tcached++;
			if (str_str(line.s, "query"))
				tquery++;
			if (debug) {
				subprintf(subfderr, "cached %d query %d\n", tcached, tquery);
				substdio_flush(subfderr);
			}
			break;
		case 'b':
			if ((t1 = str_str(line.s, ": bytes ")) != NULL) {
				t1 += 8;
				scan_ulong(t1, &tmpulong);
				bytes += tmpulong;
			}
			if (debug) {
				subprintf(subfderr, "bytes %f\n", (float) bytes);
				substdio_flush(subfderr);
			}
			break;
		} /* switch (TheType) */
	} /* for(;;) */
	close(fd);
	return;
}

int
main(int argc, char **argv)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	unsigned long   secs;
	int             i;

	if (argc != 3)
		strerr_die2x(100, WARN, usage);
	if (env_get("DEBUG"))
		debug = 1;
	qsprintf(&thedir, "%s", argv[2]);
	TheType = *argv[1];
	switch (TheType)
	{
	case 'C':
	case 't':
	case 'a':
	case 'm':
	case 'c':
	case 's':
	case 'S':
	case 'b':
	case 'q':
	case 'Q':
	case 'l':
	case 'v':
	case 'u':
	case 'd':
		break;
	default:
		strerr_die2x(100, WARN, usage);
	}
	end_time = time(0);
	start_time = end_time - 300;
#if 0
	cconcurrency = tconcurrency = tallow = tdeny = ttotal = unsub =
		success = failure = deferral = bytes = local = remote =
		max_files = clicked = viewed = cfound = cerror = tspam =
		tclean = tquery = tcached = 0;
#endif

	if (TheType == 'q' || TheType == 'Q') {
		for (i = 1, mess_count = todo_count = 0;;i++) {
			qsprintf(&tmp, "%s/queue%d/%s", thedir.s, i, TheType == 'Q' ? "local" : "mess");
			if (access(tmp.s, F_OK))
				break;
			max_files = 0;
			mess_count += get_counts(tmp.s);
			qsprintf(&tmp, "%s/queue%d/%s", thedir.s, i, TheType == 'Q' ? "remote" : "todo");
			max_files = 0;
			todo_count += get_counts(tmp.s);
		}
		qsprintf(&tmp, "%s/nqueue/%s", thedir.s, TheType == 'Q' ? "local" : "mess");
		max_files = 0;
		mess_count += get_counts(tmp.s);
		qsprintf(&tmp, "%s/nqueue/%s", thedir.s, TheType == 'Q' ? "local" : "todo");
		max_files = 0;
		todo_count += get_counts(tmp.s);
		subprintf(subfdoutsmall, "%d\n%d\n\n\n", mess_count * 12, todo_count * 12);
		substdio_flush(subfdoutsmall);
		return (0);
	}
	if (debug) {
		subprintf(subfderr, "opening dir %s\n", thedir.s);
		substdio_flush(subfderr);
	}
	if (!(mydir = opendir(thedir.s)))
		strerr_die4sys(111, FATAL, "failed to open dir ", thedir.s, ": ");
	while ((mydirent = readdir(mydir)) != NULL) {
		if (mydirent->d_name[0] == '@') {
			secs = get_tai(&mydirent->d_name[1]);
			if (secs > start_time) {
				qsprintf(&thefile, "%s/%s", thedir.s, mydirent->d_name);
				if (debug) {
					subprintf(subfderr, "processing file %s/%s\n", thedir.s, mydirent->d_name);
					substdio_flush(subfderr);
				}
				process_file(thefile.s);
			}
		} else
		if (!str_diffn(mydirent->d_name, "current", 8)) {
			qsprintf(&thefile, "%s/%s", thedir.s, mydirent->d_name);
			if (debug) {
				subprintf(subfderr, "processing file %s/%s\n", thedir.s, mydirent->d_name);
				substdio_flush(subfderr);
			}
			process_file(thefile.s);
		}
	}
	closedir(mydir);
	/*
	 * remember that mrtg gets called by cron every 5 minutes
	 * To get per hour we multply the number by 12
	 */
	switch (TheType)
	{
	case 'S': /*- bogofilter per/hr */
		subprintf(subfdoutsmall, "%i\n%i\n\n\n", tclean * 12, tspam * 12);
		break;
	case 'C': /*- clamav per/hr */
		subprintf(subfdoutsmall,"%i\n%i\n\n\n", cfound * 12, cerror * 12);
		break;
	case 't': /*- tcpserver concurrency */
		subprintf(subfdoutsmall, "%d\n%d\n\n\n", cconcurrency, tconcurrency);
		break;
	case 'a': /*- tcpserver allow/deny per/hr */
		subprintf(subfdoutsmall, "%d\n%d\n\n\n", tallow * 12, tdeny * 12);
		break;
	case 's': /*- success/failures per/hr */
		subprintf(subfdoutsmall, "%i\n%i\n\n\n", success * 12, failure * 12);
		break;
	case 'm': /*- messages per/hr */
		subprintf(subfdoutsmall, "%d\n%d\n\n\n", success * 12, (success + deferral) * 12);
		break;
	case 'c': /*- remote/local concurrency */
		subprintf(subfdoutsmall, "%d\n%d\n\n\n", local, remote);
		break;
	case 'v': /*- clicked/viewed per/hr */
		subprintf(subfdoutsmall, "%i\n%i\n\n\n", viewed * 12, clicked * 12);
		break;
	case 'l': /*- lines per/hr */
		subprintf(subfdoutsmall, "%i\n%i\n\n\n", ttotal * 12, ttotal * 12);
		break;
	case 'u': /*- lines per/hr */
		subprintf(subfdoutsmall, "%i\n%i\n\n\n", unsub * 12, unsub * 12);
		break;
	case 'b': /*- bits per/sec */
		subprintf(subfdoutsmall, "%.0f\n%.0f\n\n\n", ((float) bytes * 8.0) / 300.0, ((float) bytes * 8.0) / 300.0);
		break;
	case 'd': /*- dnscache per/hr */
		subprintf(subfdoutsmall, "%i\n%i\n\n\n", tcached * 12, tquery * 12);
		break;
	}
	substdio_flush(subfdoutsmall);
	return (0);
}
