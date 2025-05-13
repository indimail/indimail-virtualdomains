/*
 * $Id: vacation.c,v 1.8 2025-05-13 20:36:01+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <str.h>
#include <case.h>
#include <subfd.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#include <error.h>
#include <getEnvConfig.h>
#endif
#include "add_vacation.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "sql_getpw.h"
#include "iclose.h"
#include "case.h"
#include "variables.h"
#include "runcmmd.h"

#ifndef	lint
static char     sccsid[] = "$Id: vacation.c,v 1.8 2025-05-13 20:36:01+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vacation: fatal: "
#define WARN    "vacation: warning: "

#ifndef TMPDIR
#define TMPDIR "/tmp"
#endif

static char    *usage =
	"usage: vacation | vacation email_addr vacation_mesg_file\n"
	"where vacation_mesg_file can be either of the following\n"
	"-         for removing vacation\n"
	"+         for adding new vacation with text from stdin\n"
	"file_path for taking the content for vacation from file_path"
	;

static stralloc tmpbuf = {0};

int
get_options(int argc, char **argv, char **email, char **vacation_file)
{
	char           *ptr;

	*email = *vacation_file = 0;
	if (optind < argc) {
		for (ptr = argv[optind]; *ptr; ptr++) {
			if (isupper(*ptr))
				strerr_die4x(100, WARN, "email [", argv[optind], "] has an uppercase character");
		}
		*email = argv[optind++];
	}
	if (optind < argc)
		*vacation_file = argv[optind++];
	if (!*email || !*vacation_file)
		strerr_die2x(100, WARN, usage);
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vacation: out of memory", 0);
	_exit(111);
}

int
getuserinfo(char *username, stralloc *homedir, stralloc *user, stralloc *domain)
{
	const char     *real_domain;
	struct passwd  *mypw;

	parse_email(username, user, domain);
	if (!(real_domain = get_real_domain(domain->s)))
		real_domain = domain->s;
	if (!(mypw = sql_getpw(user->s, real_domain))) {
		if (!userNotFound)
			_exit(111);
		strerr_warn4("vacation: no such user ", user->s, "@", real_domain, 0);
		iclose();
		_exit(100);
	}
	iclose();
	if (!stralloc_copys(homedir, mypw->pw_dir) ||
			!stralloc_0(homedir))
		die_nomem();
	homedir->len--;
	return (0);
}

int
vacation_check(stralloc *email, stralloc *homedir)
{
	struct stat     statbuf;
	time_t          curtime;
	int             fd;

	curtime = time(0);
	if (!stralloc_copy(&tmpbuf, homedir) ||
			!stralloc_catb(&tmpbuf, "/.vacation.dir", 14) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (access(tmpbuf.s, F_OK)) {
		if (mkdir(tmpbuf.s, 0755) == -1)
			strerr_die3sys(111, "vacation: mkdir: ", tmpbuf.s, ": ");
		return (0);
	}
	if (!stralloc_copy(&tmpbuf, homedir) ||
			!stralloc_catb(&tmpbuf, "/.vacation.dir", 14) ||
			!stralloc_append(&tmpbuf, "/") ||
			!stralloc_cat(&tmpbuf, email) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (stat(tmpbuf.s, &statbuf)) {
		if (errno == 2) {
			if ((fd = open_trunc(tmpbuf.s)) == -1)
				strerr_die3sys(111, "vacation: open_trunc: ", tmpbuf.s, ": ");
			close(fd);
		}
		return (0);
	}
	if ((curtime - statbuf.st_mtime) > 86400) {
		if (utimes(tmpbuf.s, 0))
			strerr_die3sys(111, "vacation: utime: ", tmpbuf.s, ": ");
	} else
		return (1);
	return(0);
}

int
main(int argc, char **argv)
{
	char           *email, *ptr, *cptr, *sender, *vacation_file;
	static stralloc fromid = {0}, toid = {0}, subject = {0}, user = {0}, domain = {0},
					homedir = {0}, vac_msg_file = {0}, line = {0}, cmmd = {0};
	int             i, wfd, rfd, match;
	char            inbuf[1024], outbuf[512];
	char            filename[sizeof (TMPDIR) + 19] = TMPDIR "/vacation.XXXXXXXX";
	struct substdio ssin, ssout;

	if (argc == 3) {
		if (get_options(argc, argv, &email, &vacation_file))
			return (1);
		if (!str_diffn(argv[1], "-h", 3))
			strerr_die2x(100, WARN, usage);
		return(add_vacation(argv[1], ((argc == 3) ? argv[2] : (char *) 0)));
	} else
	if (argc == 2 || argc > 3)
		strerr_die2x(100, WARN, usage);
	if ((ptr = (char *) env_get("RECIPIENT")) != (char *) NULL) {
		i = str_chr(ptr, '-');
		if (ptr[i]) {
			if (!stralloc_copys(&fromid, ptr + i + 1) ||
					!stralloc_0(&fromid))
				die_nomem();
		} else {
			if (!stralloc_copys(&fromid, ptr) ||
					!stralloc_0(&fromid))
				die_nomem();
		}
		fromid.len--;
		getuserinfo(fromid.s, &homedir, &user, &domain);
		if (!stralloc_copy(&vac_msg_file, &homedir) ||
				!stralloc_catb(&vac_msg_file, "/.vacation.msg", 14) ||
				!stralloc_0(&vac_msg_file))
			die_nomem();
		vac_msg_file.len--;
	} else {
		strerr_warn1("vacation: No RECIPIENT in environment", 0);
		_exit(0);
	}
	if (!(sender = env_get("SENDER"))) {
		strerr_warn1("vacation: No SENDER in environment", 0);
		_exit(0);
	} else
	if (!str_diff(sender, "#@[]")) {
		strerr_warn1("vacation: SENDER is <#@[]> (double bounce message)", 0);
		_exit(0);
	}
	i = str_chr(sender, '@');
	if (!sender[i]) {
		strerr_warn1("vacation: SENDER did not contain a hostname", 0);
		_exit(0);
	} else
	if (!case_diffb(sender, 13, "mailer-daemon")) {
		strerr_warn1("vacation: SENDER was mailer-daemon", 0);
		_exit(0);
	}
	/*- RPLINE=Return-Path: <manvendra@indimail.org> -*/
	if ((ptr = (char *) env_get("RPLINE")) != (char *) NULL) {
		ptr += 12;
		for (; isspace((int) *ptr); ptr++);
		cptr = ptr;
		for (i = 0; *ptr; ptr++) {
			if (*ptr != '<' && *ptr != '>' && *ptr != '\n')
				i++;
		}
		if (!stralloc_ready(&toid, i + 1))
			die_nomem();
		ptr = cptr;
		cptr = toid.s;
		for (; *ptr; ptr++) {
			if (*ptr != '<' && *ptr != '>' && *ptr != '\n')
				*cptr++ = *ptr;
		}
		*cptr++ = 0;
		toid.len = i + 1;
	} else {
		strerr_warn1("vacation: No RPLINE in environment", 0);
		_exit(0);
	}
	for (;;) {
		if (getln(subfdinsmall, &line, &match, '\n') == -1) {
			strerr_warn1("vacation: read-email: ", &strerr_sys);
			return (-1);
		}
		if (!line.len)
			break;
		if (line.s[0] == '\n')
			break;
		line.len--;
		line.s[line.len] = 0;
		if (!case_diffb(line.s, 8, "Subject:")) {
			ptr = line.s + 8;
			for (; isspace((int) *ptr); ptr++);
			if (!stralloc_copyb(&subject, "Re: ", 4) ||
					!stralloc_cats(&subject, ptr) ||
					!stralloc_0(&subject))
				die_nomem();
		}
	}
	if (vacation_check(&toid, &homedir))
		return (0);
	if ((wfd = mkstemp(filename)) == -1)
		strerr_die3sys(111, "vacation: mkstemp: ", filename, ": ");
	else
	if (unlink(filename)) { /*- make file invisible */
		strerr_warn3("vacation: unlink: ", filename, ": ", &strerr_sys);
		close(wfd);
		_exit(111);
	}
	if ((rfd = open_read(vac_msg_file.s)) == -1) {
		if (errno != error_noent) {
			strerr_warn3("vacation: open: ", vac_msg_file.s, ": ", &strerr_sys);
			close(wfd);
			_exit(111);
		}
	}
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, wfd, outbuf, sizeof(outbuf));
	if (rfd != -1)
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, rfd, inbuf, sizeof(inbuf));
	if (substdio_put(&ssout, "To: ", 4) ||
			substdio_put(&ssout, toid.s, toid.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "From: ", 6) ||
			substdio_put(&ssout, fromid.s, fromid.len) ||
			substdio_put(&ssout, "\n", 1)) {
		strerr_warn3("vacation: write error: ", filename, ": ", &strerr_sys);
		close(wfd);
		if (rfd != -1)
			close(rfd);
		_exit(111);
	}
	if (subject.len) {
		if (substdio_put(&ssout, "Subject: ", 9) ||
				substdio_put(&ssout, subject.s, subject.len) ||
				substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("vacation: write error: ", filename, ": ", &strerr_sys);
			close(wfd);
			if (rfd != -1)
				close(rfd);
			_exit(111);
		}
	} else {
		if (substdio_put(&ssout, "Subject: Auto Response from ", 28) ||
				substdio_put(&ssout, fromid.s, fromid.len) ||
				substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("vacation: write error: ", filename, ": ", &strerr_sys);
			close(wfd);
			if (rfd != -1)
				close(rfd);
			_exit(111);
		}
	}
	if ((ptr = env_get("CHARSET"))) {
		if (substdio_put(&ssout, "Mime-Version: 1.0\n", 18) ||
				substdio_put(&ssout, "Content-Type: text/plain; charset=\"", 35) ||
				substdio_puts(&ssout, ptr) ||
				substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("vacation: write error: ", filename, ": ", &strerr_sys);
			close(wfd);
			if (rfd != -1)
				close(rfd);
			_exit(111);
		}
	}
	if (rfd != -1) {
		switch (substdio_copy(subfdinsmall, &ssout))
		{
		case -2: /*- read error */
			strerr_warn1("vacation: read error: ", &strerr_sys);
			close(wfd);
			close(rfd);
			_exit(111);
		case -3: /*- write error */
			strerr_warn1("vacation: write error: ", &strerr_sys);
			close(wfd);
			close(rfd);
			_exit(111);
		}
		close(rfd);
	} else {
		if (substdio_put(&ssout, "This is an Auto Reply from ", 27) ||
				substdio_put(&ssout, fromid.s, fromid.len) ||
				substdio_put(&ssout, "\n", 1)) {
			strerr_warn3("vacation: write error: ", filename, ": ", &strerr_sys);
			close(wfd);
			_exit(111);
		}
	}

	if (substdio_put(&ssout, "--------------------------------------------------------------------------\n", 75) ||
			substdio_put(&ssout, "Note: Further Auto Response will be deferred to ", 48) ||
			substdio_put(&ssout, toid.s, toid.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "      until 24 Hrs Elapses and a new message is received\n", 56) ||
			substdio_put(&ssout, "      from ", 11) ||
			substdio_put(&ssout, fromid.s, fromid.len) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "--------------------------------------------------------------------------\n", 75) ||
			substdio_flush(&ssout)) {
			strerr_warn3("vacation: write error: ", filename, ": ", &strerr_sys);
			close(wfd);
			_exit(111);
	}
	if (!stralloc_copys(&cmmd, PREFIX) ||
			!stralloc_catb(&cmmd, "/bin/qmail-inject -f", 20) ||
			!stralloc_cat(&cmmd, &fromid) ||
			!stralloc_append(&cmmd, " ") ||
			!stralloc_cat(&cmmd, &toid) ||
			!stralloc_0(&cmmd))
		die_nomem();
	if (lseek(wfd, 0, SEEK_SET) != 0) {
		strerr_warn1("vacation: lseek error: ", &strerr_sys);
		close(wfd);
		_exit(111);
	}
	if (dup2(wfd, 0) == -1) {
		strerr_warn1("vacation: dup2 error: ", &strerr_sys);
		close(wfd);
		_exit(111);
	}
	if (wfd)
		close(wfd);
	_exit(runcmmd(cmmd.s, 0));
}
/*
 * $Log: vacation.c,v $
 * Revision 1.8  2025-05-13 20:36:01+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.7  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2021-07-08 15:18:37+05:30  Cprogrammer
 * fixed argument handling
 *
 * Revision 1.5  2020-07-04 22:54:50+05:30  Cprogrammer
 * replaced utime() with utimes()
 *
 * Revision 1.4  2020-06-12 21:37:59+05:30  Cprogrammer
 * added stdlib.h for mkstemp() prototype
 *
 * Revision 1.3  2020-04-01 18:58:21+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 15:57:12+05:30  mbhangui
 * use _exit() instead of exit()
 *
 * Revision 1.1  2019-04-18 08:38:40+05:30  Cprogrammer
 * Initial revision
 *
 */
