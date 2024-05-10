/*
 * $Log: qmailmrtg.c,v $
 * Revision 1.8  2024-05-10 11:44:24+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2023-09-17 22:36:34+05:30  Cprogrammer
 * removed leading white space to correct current concurrency
 *
 * Revision 1.6  2023-04-09 22:26:02+05:30  Cprogrammer
 * read from stdin if logdir is -
 *
 * Revision 1.5  2023-04-09 12:00:46+05:30  Cprogrammer
 * added case for generating status for inlookup Cache Hits
 *
 * Revision 1.4  2023-04-08 23:55:32+05:30  Cprogrammer
 * refactored code to print service uptime and status
 *
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
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <substdio.h>
#include <subfd.h>
#include <qprintf.h>
#include <stralloc.h>
#include <strerr.h>
#include <open.h>
#include <getln.h>
#include <str.h>
#include <error.h>
#include <scan.h>
#include <sgetopt.h>
#include <fmt.h>
#include <tai.h>
#include <no_of_days.h>

#ifndef lint
static char     sccsid[] = "$Id: qmailmrtg.c,v 1.8 2024-05-10 11:44:24+05:30 mbhangui Exp mbhangui $";
#endif

#define FATAL "qmailmrtg: fatal: "
#define WARN  "qmailmrtg: warn: "

int             BigTodo=0, ConfSplit=23, debug, process_full;
unsigned long   cconcurrency, tconcurrency, tallow, tdeny, ttotal, success,
				failure, deferral, unsub, local, remote, clicked, viewed,
				cfound, cerror, tspam, tclean, tcached, tquery, tuser,
				trelay, tpass, tlimit, talias, thost, tdomain, tcache_hits;
time_t          end_time, start_time;
static stralloc tmp, thefile;
unsigned long   bytes;
int             normallyup = 0;
static char    *usage = "usage: qmailmrtg [options] logdir [servicedir]";

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

unsigned long
get_counts(char *diri)
{
	int             i;
	unsigned long   count;

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
process_file(char *file_name, char type, char inquery_type)
{
	unsigned long   secs, tmpulong, t1, t2, t3, t4, t5, t6, t7, t8;
	int             fd, match, i;
	char           *p1, *p2;
	static stralloc line = {0};
	char            inbuf[4096];
	struct substdio ssin;

	t1 = t2 = t3 = t4 = t5 = t6 = t7 = t8 = 0;
	if (!file_name)
		fd = 0;
	else
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
			line.s[line.len] = 0;
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
		if (!process_full && (secs < start_time || secs > end_time))
			continue;
		switch (type)
		{
		case 'S':
			if ((p1 = str_str(line.s, "X-Bogosity: Yes")) != NULL)
				++tspam;
			else
			if ((p1 = str_str(line.s, "X-Bogosity: No")) != NULL)
				++tclean;
			else
				break;
			if (debug) {
				subprintf(subfderr, "tspam %lu tclean %lu\n", tspam, tclean);
				substdio_flush(subfderr);
			}
			break;
		case 'C':
			if ((p1 = str_str(line.s, "FOUND")) != NULL)
				++cfound;
			else
			if ((p1 = str_str(line.s, "ERROR")) != NULL)
				++cerror;
			else
				break;
			if (debug) {
				subprintf(subfderr, "cfound %lu cerror %lu\n", cfound, cerror);
				substdio_flush(subfderr);
			}
			break;
		case 't':
			if (!(p1 = str_str(line.s, "status:")))
				break;
			while (*p1 != ':')
				++p1;
			++p1;

			while (isspace(*p1)) p1++;
			for (p2 = p1 + 1; *p2 != '/'; ++p2);
			*p2 = 0;
			scan_ulong(p1, &tmpulong);
			if (tmpulong > cconcurrency)
				cconcurrency = tmpulong;
			scan_ulong(p2 + 1, &tmpulong);
			if (tmpulong > tconcurrency)
				tconcurrency = tmpulong;
			if (debug) {
				subprintf(subfderr, "cconcurrency %lu tconcurrency %lu\n", cconcurrency, tconcurrency);
				substdio_flush(subfderr);
			}
			break;
		case 'a':
			if ((p1 = str_str(line.s, " ok ")) != NULL)
				++tallow;
			else
			if ((p1 = str_str(line.s, " deny ")) != NULL)
				++tdeny;
			else
			if ((p1 = str_str(line.s, " rblsmtpd:")) != NULL)
				++tdeny;
			else
				break;
			if (debug) {
				subprintf(subfderr, "tallow %lu tdeny %lu\n", tallow, tdeny);
				substdio_flush(subfderr);
			}
			break;
		case 'c': /*- @40000000642a387d2dede194 status: local 0/10 remote 0/20 queue2 */
			if (!(p1 = str_str(line.s, " status: ")))
				break;
			while (*p1 != '/' && *p1 != 0)
				++p1;
			*p1 = 0;
			p2 = p1 + 1;
			--p1;
			while (*p1 != ' ')
				--p1;
			++p1;
			scan_ulong(p1, &tmpulong); /*- local concurrency */
			if (tmpulong > local)
				local = tmpulong;

			while (*p2 != '/' && *p2 != 0)
				++p2;
			*p2 = 0;
			--p2;
			while (*p2 != ' ')
				--p2;
			++p2;
			scan_ulong(p2, &tmpulong); /*- remote concurrency */
			if (tmpulong > remote)
				remote = tmpulong;
			if (debug) {
				subprintf(subfderr, "local %lu remote %lu\n", local, remote);
				substdio_flush(subfderr);
			}
			break;
		case 's':
		case 'm':
			if (str_str(line.s, "success:"))
				success++;
			else
			if (str_str(line.s, "failure:"))
				failure++;
			else
			if (str_str(line.s, "deferral:"))
				deferral++;
			else
				break;
			if (debug) {
				subprintf(subfderr, "success %lu failure %lu deferral %lu\n", success, failure, deferral);
				substdio_flush(subfderr);
			}
			break;
		case 'v':
			if (str_str(line.s, "viewed:"))
				viewed++;
			else
			if (str_str(line.s, "clicked:"))
				clicked++;
			else
				break;
			if (debug) {
				subprintf(subfderr, "viewed %lu clicked %lu\n", viewed, clicked);
				substdio_flush(subfderr);
			}
			break;
		case 'u':
			if (str_str(line.s, "unsub:"))
				unsub++;
			else
				break;
			if (debug) {
				subprintf(subfderr, "unsub %lu\n", unsub);
				substdio_flush(subfderr);
			}
			break;
		case 'l':
			++ttotal;
			if (debug) {
				subprintf(subfderr, "ttotal %lu\n", ttotal);
				substdio_flush(subfderr);
			}
			break;
		case 'd':
			if (!str_str(line.s, "cached") && !str_str(line.s, "query"))
				continue;
			if (str_str(line.s, "cached"))
				tcached++;
			if (str_str(line.s, "query"))
				tquery++;
			if (debug) {
				subprintf(subfderr, "%s\n", line.s);
				subprintf(subfderr, "cached %lu query %lu\n", tcached, tquery);
				substdio_flush(subfderr);
			}
			break;
		case 'b':
			if (!(p1 = str_str(line.s, ": bytes ")))
				break;
			p1 += 8;
			scan_ulong(p1, &tmpulong);
			bytes += tmpulong;
			if (debug) {
				subprintf(subfderr, "bytes %f\n", (float) bytes);
				substdio_flush(subfderr);
			}
			break;
		case 'i': /*- User:Relay:Password:Limit:Alias:Host:Domain 0 0 0 0 0 1 1 Cached Nodes 1 */
			if (inquery_type == 'C') {
				if (str_str(line.s, "Query ['User Query']") ||
						str_str(line.s, "Query ['Relay Query']") ||
						str_str(line.s, "Query ['Password Query']") ||
						str_str(line.s, "Query ['Host Query']") ||
						str_str(line.s, "Query ['Alias Query']") ||
						str_str(line.s, "Query ['Domain Query']") ||
						str_str(line.s, "Query ['Domain Limits Query']"))
					ttotal++;
				else
				if ((p1 = str_str(line.s, "cache hit")))
					tcache_hits++;
				else
					break;
				if (debug) {
					subprintf(subfderr, "%lu %lu %lu %lu %lu %lu %lu %lu %lu\n", t1, t2, t3, t4, t5, t6, t7, t8, tcache_hits);
					substdio_flush(subfderr);
				}
			}
			if ((p1 = str_str(line.s, "User:Relay:Password:Limit:Alias:Host:Domain")) ) {
				p1 += 44;
				i = scan_ulong(p1, &tmpulong);
				tuser += (t1 = tmpulong);
				p1 += (i + 1);
				i = scan_ulong(p1, &tmpulong);
				trelay += (t2 = tmpulong);
				p1 += (i + 1);
				i = scan_ulong(p1, &tmpulong);
				tpass += (t3 = tmpulong);
				p1 += (i + 1);
				i = scan_ulong(p1, &tmpulong);
				tlimit += (t4 = tmpulong);
				p1 += (i + 1);
				i = scan_ulong(p1, &tmpulong);
				talias += (t5 = tmpulong);
				p1 += (i + 1);
				i = scan_ulong(p1, &tmpulong);
				thost += (t6 = tmpulong);
				p1 += (i + 1);
				i = scan_ulong(p1, &tmpulong);
				tdomain += (t7 = tmpulong);
				p1 += (i + 14);
				i = scan_ulong(p1, &tmpulong);
				tcached += (t8 = tmpulong);
			} else
			if ((p1 = str_str(line.s, "cache hit")))
				tcache_hits++;
			else
				break;
			if (debug) {
				subprintf(subfderr, "%lu %lu %lu %lu %lu %lu %lu %lu %lu\n", t1, t2, t3, t4, t5, t6, t7, t8, tcache_hits);
				substdio_flush(subfderr);
			}
			break;
		} /* switch (type) */
	} /* for(;;) */
	close(fd);
	return;
}

/* 
 * Taken from daemontools/run_init.c
 *
 * adapt /run filesystem for supervise
 * returns
 *  0 - chdir to /run/svscan/service_name
 *  1 - no run filesystem
 * -1 - unable to get cwd
 * -2 - unable to chdir to service_name
 * -3 - name too long
 */
int
run_init(const char *service_dir)
{
	char           *run_dir, *p, *s;
	char            buf[256], dirbuf[256];
	int             i;

	if (!access("/run/svscan", F_OK))
		run_dir = "/run";
	else
	if (!access("/private/var/run/svscan", F_OK))
		run_dir = "/private/var/run";
	else
	if (!access("/var/run/svscan", F_OK))
		run_dir = "/var/run";
	else
		return 1;
	/*- e.g. /service/qmail-smtpd.25 */
	if ((i = str_len(service_dir)) > 255)
		return -3;
	s = buf;
	s += fmt_str(s, service_dir);
	*s++ = 0;
	p = basename(buf);
	if (!str_diff(p, ".")) {
		if (!getcwd(buf, 255))
			return -1;
		i = str_rchr(buf, '/');
		if (buf[i])
			p = buf + i + 1;
		else
			return -2;
		/*- e.g. doing svstat . in
		 * 1. /run/svscan/qmail-smtpd.25
		 * 2. /run/svscan/qmail-smtpd.25/log
		 */
		i = fmt_str(0, run_dir) + 9 + fmt_str(0, p);
		if (i > 255)
			return 1;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		*s++ = 0;
		if (access(dirbuf, F_OK)) {
			for (p -= 2;*p && *p != '/';p--);
			if (*p != '/')
				return -2;
			i = fmt_str(0, run_dir) + 9 + fmt_str(0, p + 1);
			if (i > 255)
				return -3;
			s = dirbuf;
			s += fmt_str(s, run_dir);
			s += fmt_strn(s, "/svscan/", 8);
			s += fmt_str(s, p + 1);	
			*s++ = 0;
		}
	} else
	if (!str_diff(p, "log")) {
		if (!getcwd(buf, 255)) /*- /service/name/log */
			return -1;
		i = str_rchr(buf, '/'); /*- /log */
		if (!buf[i])
			return -2;
		buf[i] = 0;
		i = str_rchr(buf, '/'); /*- /name/log */
		if (!buf[i])
			return -2;
		p = buf + i + 1; /*- name/log */
		i = fmt_str(0, run_dir) + 13 + fmt_str(0, p);
		if (i > 255)
			return -3;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		s += fmt_strn(s, "/log", 4);
		*s++ = 0;
	} else {
		/*- e.g. /run/svscan/qmail-smtpd.25 */
		i = fmt_str(0, run_dir) + 9 + fmt_str(0, p);
		if (i > 255)
			return -3;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		*s++ = 0;
	}
	/*-
	 * we do chdir to /run/svscan/qmail-smtpd.25
	 * instead of
	 * /service/qmail-stmpd.25
	 */
	if (chdir(dirbuf) == -1)
		return -2;
	return 0;
}

void
print_status(const char *dir, char status[])
{
	unsigned long   pid;
	unsigned char   want, paused;
	struct tai      when, now;
	static short    waiting;
	short          *s;
	int             retval = 0;
	char            strnum[FMT_LONG];

	pid = (unsigned char) status[15];
	pid <<= 8;
	pid += (unsigned char) status[14];
	pid <<= 8;
	pid += (unsigned char) status[13];
	pid <<= 8;
	pid += (unsigned char) status[12];

	if ((paused = status[16]))
		retval = 3;
	want = status[17];
	s = (short *) (status + 18);
	waiting = *s;
	tai_unpack(status, &when);
	tai_now(&now);
	if (tai_less(&now, &when))
		when = now;
	tai_sub(&when, &now, &when);
	substdio_puts(subfdoutsmall, no_of_days(tai_approx(&when)));
	substdio_put(subfdoutsmall, "\n", 1);

	substdio_puts(subfdoutsmall, dir);
	substdio_puts(subfdoutsmall, ": ");
	if (waiting)
		substdio_puts(subfdoutsmall, "wait ");
	else
	if (status[20])
		substdio_puts(subfdoutsmall, "up ");
	else
		substdio_puts(subfdoutsmall, "down ");
	substdio_put(subfdoutsmall, strnum, fmt_long(strnum, tai_approx(&when)));
	substdio_puts(subfdoutsmall, " seconds");

	if (status[20] && !normallyup)
		substdio_puts(subfdoutsmall, ", normally down");
	if (!pid && normallyup)
		substdio_puts(subfdoutsmall, ", normally up");
	if (status[20] && paused)
		substdio_puts(subfdoutsmall, ", paused");
	if (!pid && (want == 'u')) {
		retval = 4;
		substdio_puts(subfdoutsmall, ", want up");
	}
	if (status[20] && (want == 'd')) {
		retval = 5;
		substdio_puts(subfdoutsmall, ", want down");
	}

	if (pid && status[20]) {
		if (retval != 3 && retval != 5)
			retval = 0;
		substdio_puts(subfdoutsmall, " pid ");
		substdio_put(subfdoutsmall, strnum, fmt_ulong(strnum, pid));
		substdio_puts(subfdoutsmall, " ");
	} else
	if (pid) {
		retval = 1;
		substdio_puts(subfdoutsmall, " spid ");
		substdio_put(subfdoutsmall, strnum, fmt_ulong(strnum, pid));
		substdio_puts(subfdoutsmall, " ");
	} else
		retval = 1;
	if (waiting > 0 && (waiting - when.x) > 0) {
		substdio_puts(subfdoutsmall, "remaining ");
		when.x = waiting - when.x;
		substdio_put(subfdoutsmall, strnum, fmt_ulong(strnum, tai_approx(&when)));
		substdio_puts(subfdoutsmall, " seconds");
	}
	substdio_puts(subfdoutsmall, "\n");
}

void
print_uptime(const char *sdir, char status[], int len)
{
	const char     *x;
	int             fd, r;

	if (!sdir) {
		subprintf(subfdoutsmall, "\n\n");
		return;
	}
	if (chdir(sdir) == -1) {
		strerr_warn4(WARN, "unable to to chdir to ", sdir, ": ", &strerr_sys);
		return;
	}
	normallyup = 0;
	if (access("down", F_OK)) {
		if (errno != error_noent) {
			strerr_warn4(WARN, "unable to stat ", sdir, "/down: ", &strerr_sys);
			return;
		}
		normallyup = 1;
	}
	if ((fd = open_write("supervise/ok")) == -1) {
		if (errno == error_nodevice) {
			strerr_warn4(WARN, "unable to open ", sdir, "/supervise/ok: supervise not running: ", &strerr_sys);
			return;
		} else
		if (errno != error_noent) {
			strerr_warn4(WARN, "unable to open ", sdir, "/supervise/ok: ", &strerr_sys);
			return;
		}
		switch (run_init(sdir))
		{
		case 0: /*- cwd changed to /run/svscan/service_name */
			break;
		case 1: /*- /run, /var/run doesn't exist */
			strerr_warn4(WARN, "unable to open ", sdir, "/supervise/ok: supervise not running: ", &strerr_sys);
		case -1:
			strerr_warn2(WARN, "unable to get current working directory: ", &strerr_sys);
			return;
		case -2:
			strerr_warn4(WARN, "No run state information for ", sdir, ": ", &strerr_sys);
			return;
		case -3:
			strerr_warn3(WARN, sdir, ": name too long", 0);
			return;
		}
	}
	if (fd == -1 && (fd = open_write("supervise/ok")) == -1) {
		if (errno == error_nodevice || errno == error_noent) {
			strerr_warn4(WARN, "unable to open ", sdir, "/supervise/ok: supervise not running: ", &strerr_sys);
			return;
		}
		strerr_warn4(WARN, "unable to open ", sdir, "/supervise/ok: ", &strerr_sys);
		return;
	}
	close(fd);
	if ((fd = open_read("supervise/status")) == -1) {
		strerr_warn4(WARN, "unable to open ", sdir, "/supervise/status: ", &strerr_sys);
		return;
	}
	r = read(fd, status, len);
	close(fd);
	if (r < len) {
		if (r == -1)
			x = error_str(errno);
		else
			x = "bad format";
		strerr_warn5(WARN, "unable to read ", sdir, "/supervise/status: ", x, 0);
		return;
	}
	print_status(sdir, status);
}

int
main(int argc, char **argv)
{
	DIR            *mydir;
	struct dirent  *mydirent;
	unsigned long   secs, mess_count, todo_count, interval = 300;
	char           *logdir = NULL, *servicedir = NULL;
	char            status[21];
	int             i, TheType = 0, inquery_type = 0;

	while ((i = getopt(argc, argv, "abBcCdDE:flmp:qQsStuvi:I:")) != opteof) {
		switch (i)
		{
		case 'a':
		case 'b':
		case 'c':
		case 'C':
		case 'd':
		case 'l':
		case 'm':
		case 'q':
		case 'Q':
		case 's':
		case 'S':
		case 't':
		case 'u':
		case 'v':
			TheType = i;
			break;
		case 'B':
			BigTodo = 1;
		case 'D':
			debug = 1;
			break;
		case 'E':
			scan_long(optarg, &end_time);
			break;
		case 'f':
			process_full = 1;
			break;
		case 'I':
			scan_ulong(optarg, &interval);
			break;
		case 'i':
			TheType = i;
			inquery_type = *optarg;
			break;
		case 'p':
			scan_int(optarg, &ConfSplit);
			break;
		} /*- switch (TheType) */
	} /*- while ((TheType = getopt(argc, argv, "abcCdlmqQsStuvi:")) != opteof) */

	if (argc < optind + 1)
		strerr_die1x(100, usage);
	if (optind < argc)
		logdir = argv[optind++];
	if (optind < argc)
		servicedir = argv[optind++];

	if (!process_full) {
		if (!end_time)
			end_time = time(0);
		start_time = end_time - interval;
	}
	if (TheType == 'q' || TheType == 'Q') {
		for (i = 1, mess_count = todo_count = 0;;i++) {
			qsprintf(&tmp, "%s/queue%d/%s", logdir, i, TheType == 'Q' ? "local" : "mess");
			if (access(tmp.s, F_OK))
				break;
			mess_count += get_counts(tmp.s);
			qsprintf(&tmp, "%s/queue%d/%s", logdir, i, TheType == 'Q' ? "remote" : "todo");
			todo_count += get_counts(tmp.s);
		}
		qsprintf(&tmp, "%s/nqueue/%s", logdir, TheType == 'Q' ? "local" : "mess");
		mess_count += get_counts(tmp.s);
		qsprintf(&tmp, "%s/nqueue/%s", logdir, TheType == 'Q' ? "local" : "todo");
		todo_count += get_counts(tmp.s);
		subprintf(subfdoutsmall, "%lu\n%lu\n", mess_count * 12, todo_count * 12);
		print_uptime(servicedir, status, sizeof status);
		substdio_flush(subfdoutsmall);
		return (0);
	}
	if (str_diff(logdir, "-")) {
		if (debug) {
			subprintf(subfderr, "opening dir %s\n", logdir);
			substdio_flush(subfderr);
		}
		if (!(mydir = opendir(logdir)))
			strerr_die4sys(111, FATAL, "failed to open dir ", logdir, ": ");
		while ((mydirent = readdir(mydir)) != NULL) {
			if (mydirent->d_name[0] == '@') {
				secs = get_tai(&mydirent->d_name[1]);
				if (secs > start_time) {
					qsprintf(&thefile, "%s/%s", logdir, mydirent->d_name);
					if (debug) {
						subprintf(subfderr, "processing file %s/%s\n", logdir, mydirent->d_name);
						substdio_flush(subfderr);
					}
					process_file(thefile.s, TheType, inquery_type);
				}
			} else
			if (!str_diffn(mydirent->d_name, "current", 8)) {
				qsprintf(&thefile, "%s/%s", logdir, mydirent->d_name);
				if (debug) {
					subprintf(subfderr, "processing file %s/%s\n", logdir, mydirent->d_name);
					substdio_flush(subfderr);
				}
				process_file(thefile.s, TheType, inquery_type);
			}
		}
		closedir(mydir);
	} else
		process_file(0, TheType, inquery_type);
	/*
	 * remember that mrtg gets called by cron every 5 minutes
	 * To get per hour we multply the number by 12
	 */
	switch (TheType)
	{
	case 'S': /*- bogofilter per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", tclean * 12, tspam * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'C': /*- clamav per/hr */
		subprintf(subfdoutsmall,"%lu\n%lu\n", cfound * 12, cerror * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 't': /*- tcpserver concurrency */
		subprintf(subfdoutsmall, "%lu\n%lu\n", cconcurrency, tconcurrency);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'a': /*- tcpserver allow/deny per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", tallow * 12, tdeny * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 's': /*- success/failures per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", success * 12, failure * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'm': /*- messages per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", success * 12, (success + deferral) * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'c': /*- remote/local concurrency */
		subprintf(subfdoutsmall, "%lu\n%lu\n", local, remote);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'v': /*- clicked/viewed per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", viewed * 12, clicked * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'l': /*- lines per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", ttotal * 12, ttotal * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'u': /*- lines per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", unsub * 12, unsub * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'b': /*- bits per/sec */
		subprintf(subfdoutsmall, "%.0f\n%.0f\n", ((float) bytes * 8.0) / 300.0, ((float) bytes * 8.0) / 300.0);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'd': /*- dnscache per/hr */
		subprintf(subfdoutsmall, "%lu\n%lu\n", tcached * 12, tquery * 12);
		print_uptime(servicedir, status, sizeof status);
		break;
	case 'i':
		/* User:Relay:Password:Limit:Alias:Host:Domain */
		if (inquery_type != 'C')
			ttotal = tuser + trelay + tpass + tlimit + talias + thost + tdomain;
		switch (inquery_type)
		{
		case 'u':
			subprintf(subfdoutsmall, "%lu\n%lu\n", tuser, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'r':
			subprintf(subfdoutsmall, "%lu\n%lu\n", trelay, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'p':
			subprintf(subfdoutsmall, "%lu\n%lu\n", tpass, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'l':
			subprintf(subfdoutsmall, "%lu\n%lu\n", tlimit, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'a':
			subprintf(subfdoutsmall, "%lu\n%lu\n", talias, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'h':
			subprintf(subfdoutsmall, "%lu\n%lu\n", thost, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'd':
			subprintf(subfdoutsmall, "%lu\n%lu\n", tdomain, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'c':
			subprintf(subfdoutsmall, "%lu\n%lu\n", tcached, ttotal);
			print_uptime(servicedir, status, sizeof status);
			break;
		case 'C':
			subprintf(subfdoutsmall, "%lu\n%lu\n", tcache_hits * 12, ttotal * 12);
			print_uptime(servicedir, status, sizeof status);
			break;
		}
		break;
	}
	substdio_flush(subfdoutsmall);
	return (0);
}
