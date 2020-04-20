/*
 * $Log: bulletin.c,v $
 * Revision 1.2  2019-06-07 15:58:03+05:30  mbhangui
 * added include file stdlib.h
 *
 * Revision 1.1  2019-04-15 09:45:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <open.h>
#include <stralloc.h>
#include <str.h>
#include <env.h>
#include <strerr.h>
#include <error.h>
#endif
#include "get_assign.h"
#include "iopen.h"
#include "fappend.h"
#include "create_table.h"
#include "indimail.h"
#include "disable_mysql_escape.h"

#ifdef CLUSTERED_SITE
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <getln.h>
#include <alloc.h>
#include <fmt.h>
#endif
#include "sql_init.h"
#include "findmdahost.h"
#include "mdaMySQLConnect.h"
#include "variables.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: bulletin.c,v 1.2 2019-06-07 15:58:03+05:30 mbhangui Exp mbhangui $";
#endif

static stralloc tmpbuf = {0};

static void
die_nomem()
{
	strerr_warn1("bulletin: out of memory", 0);
	_exit(111);
}

static long
insert_bulletin(char *domain, char *emailFile, char *list_file)
{
	static stralloc EmailFile = {0}, line = {0}, SqlBuf = {0};
	char           *p, *tmpdir;
	int             i, match, es_opt, rfd, wfd;
	uid_t           uid;
	gid_t           gid;
	long            row_count;
	char            inbuf[8192], outbuf[512];
	struct substdio ssin, ssout;

	i = str_chr(emailFile, '/');
	if (emailFile[i]) {
		strerr_warn2(emailFile, " contains '/'. Filename cannot have any path", 0);
		return (-1);
	}
	if (!stralloc_copys(&EmailFile, CONTROLDIR) ||
			!stralloc_append(&EmailFile, "/") ||
			!stralloc_cats(&EmailFile, domain) ||
			!stralloc_append(&EmailFile, "/") ||
			!stralloc_cats(&EmailFile, (p = env_get("BULK_MAILDIR")) ? p : BULK_MAILDIR) ||
			!stralloc_append(&EmailFile, "/") ||
			!stralloc_cats(&EmailFile, emailFile) ||
			!stralloc_0(&EmailFile))
		die_nomem();
	if (!get_assign(domain, 0, &uid, &gid)) {
		strerr_warn2(domain, ": No such domain", 0);
		return (-1);
	}
	if (access(emailFile, F_OK)) {
		strerr_warn3("bulletin: access: ", emailFile, ": ", &strerr_sys);
		return (-1);
	} else
	if (!access(EmailFile.s, F_OK)) {
		errno = EEXIST;
		strerr_warn3("bulletin: access: ", EmailFile.s, ": ", &strerr_sys);
		return (-1);
	}
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpbuf, tmpdir) ||
			!stralloc_catb(&tmpbuf, "/indiXXXXXX", 11) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	tmpbuf.len--;
	if ((wfd = mkstemp(tmpbuf.s)) == -1) {
		strerr_warn3("bulletin: mkstemp: ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	if (chown(tmpbuf.s, uid, gid) || chmod(tmpbuf.s, INDIMAIL_QMAIL_MODE)) {
		strerr_warn3("bulletin: chown/chmod: ", tmpbuf.s, ": ", &strerr_sys);
		close(wfd);
		unlink(tmpbuf.s);
		return (-1);
	}
	substdio_fdbuf(&ssout, write, wfd, outbuf, sizeof(outbuf));
	if ((rfd = open_read(list_file)) == -1) {
		strerr_warn3("bulletin: open: ", list_file, ": ", &strerr_sys);
		close(wfd);
		unlink(tmpbuf.s);
		return (-1);
	}
	substdio_fdbuf(&ssin, read, rfd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("bulletin: read: ", list_file, ": ", &strerr_sys);
			close(wfd);
			close(rfd);
			unlink(tmpbuf.s);
			return (-1);
		}
		if (line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (p = line.s; *p && isspace((int) *p); p++);
		if (!*p)
			continue;
		match = str_chr(p, '@');
		if (p[match]) {
			if (str_diffn(domain, p + match + 1, line.len - (match + 1))) {
				strerr_warn4("Skipping email [", p, "] - does not belong to ", domain, 0);
				continue;
			}
			if (substdio_puts(&ssout, p) ||
					substdio_put(&ssout, " ", 1) ||
					substdio_puts(&ssout, emailFile) ||
					substdio_put(&ssout, "\n", 1))
			{
				strerr_warn3("bulletin: write: ", tmpbuf.s, ": ", &strerr_sys);
				close(wfd);
				close(rfd);
				unlink(tmpbuf.s);
				return (-1);
			}
		} else {
			if (substdio_puts(&ssout, p) ||
					substdio_put(&ssout, "@", 1) ||
					substdio_puts(&ssout, domain) ||
					substdio_put(&ssout, " ", 1) ||
					substdio_puts(&ssout, emailFile) ||
					substdio_put(&ssout, "\n", 1))
			{
				strerr_warn3("bulletin: write: ", tmpbuf.s, ": ", &strerr_sys);
				close(wfd);
				close(rfd);
				unlink(tmpbuf.s);
				return (-1);
			}
		}
	}
	close(rfd);
	close(wfd);
	if (iopen((char *) 0)) {
		unlink(tmpbuf.s);
		return (-1);
	}
	if (fappend(emailFile, EmailFile.s, "w", INDIMAIL_QMAIL_MODE, uid, gid)) {
		strerr_warn5("bulletin: fappend: ", emailFile, " --> ", EmailFile.s, ": ", &strerr_sys);
		unlink(tmpbuf.s);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "load data infile \"", 18) ||
			!stralloc_cat(&SqlBuf, &tmpbuf) ||
			!stralloc_catb(&SqlBuf, "\" into table bulkmail fields terminated by ' ' lines terminated by '\\n'", 71) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	es_opt = disable_mysql_escape(1);
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "bulkmail", BULKMAIL_TABLE_LAYOUT)) {
				unlink(tmpbuf.s);
				unlink(EmailFile.s);
				disable_mysql_escape(es_opt);
				return (-1);
			}
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn4("bulletin: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
				unlink(tmpbuf.s);
				unlink(EmailFile.s);
				disable_mysql_escape(es_opt);
				return (-1);
			}
		} else {
			strerr_warn4("bulletin: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			unlink(tmpbuf.s);
			unlink(EmailFile.s);
			disable_mysql_escape(es_opt);
			return (-1);
		}
	}
	disable_mysql_escape(es_opt);
	unlink(tmpbuf.s);
	if ((row_count = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("bulletin: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		unlink(EmailFile.s);
		return (-1);
	}
	return (row_count);
}

#ifdef CLUSTERED_SITE
struct mdahosts
{
	char            mdahost[DBINFO_BUFF];
	long            emailcount;
	char          **emailptr;
};
struct mdahosts **mdaHOSTS = (struct mdahosts **) 0;

struct mdahosts **
store_email(char *host, char *domain, char *email)
{
	static struct mdahosts **mdaptr;
	static int      mdacount;
	char           *s;
	int             len, emailcount;

	if (!stralloc_copys(&tmpbuf, host) ||
			!stralloc_append(&tmpbuf, "@") ||
			!stralloc_cats(&tmpbuf, domain) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	for (mdaptr = mdaHOSTS; mdaptr && *mdaptr; mdaptr++) {
		if (!str_diffn((*mdaptr)->mdahost, tmpbuf.s, tmpbuf.len))
			break;
	}
	if (!mdaptr || !(*mdaptr)) { /*- entry for new host */
		if (!alloc_re((char *) &mdaHOSTS, mdacount * sizeof(struct mdahosts *), (mdacount + 1) * sizeof(struct mdahosts *)))
			die_nomem();
		if (!(mdaHOSTS[mdacount] = (struct mdahosts *) alloc(sizeof(struct mdahosts))))
			die_nomem();
		s = mdaHOSTS[mdacount]->mdahost;
		s += fmt_str(s, host);
		s += fmt_strn(s, "@", 1);
		s += fmt_str(s, domain);
		*s++ = 0;
		if (!(mdaHOSTS[mdacount]->emailptr = (char **) alloc(2 * sizeof(char *))))
			die_nomem();
		if (!(mdaHOSTS[mdacount]->emailptr[0] = (char *) alloc(len = (str_len(email) + 1))))
			die_nomem();
		s = mdaHOSTS[mdacount]->emailptr[0];
		s += fmt_strn(s, email, len);
		*s++ = 0;
		mdaHOSTS[mdacount]->emailcount = 1;
		mdaHOSTS[mdacount]->emailptr[1] = (char *) 0;
		mdaHOSTS[mdacount + 1] = (struct mdahosts *) 0;
		mdacount++;
	} else { /*- entry for existing host */
		emailcount = (*mdaptr)->emailcount + 1;
		if (!alloc_re((char *) &(*mdaptr)->emailptr, emailcount * sizeof(char *) , (emailcount + 1) * sizeof(char *)))
			die_nomem();
		else
		if (!((*mdaptr)->emailptr[emailcount - 1] = (char *) alloc(len = (str_len(email) + 1))))
			die_nomem();
		s = (*mdaptr)->emailptr[emailcount - 1];
		s += fmt_strn(s, email, len);
		*s++ = 0;
		(*mdaptr)->emailcount = emailcount;
		(*mdaptr)->emailptr[emailcount] = (char *) 0;
	}
	return (0);
}

long
bulletin(char *emailFile, char *subscriber_list)
{
	char           *domain, *p, *q, *r, *tmpdir;
	char          **emailptr;
	static stralloc line = {0}, SplitFile = {0};
	struct mdahosts **mdaptr;
	MYSQL         **mysqlptr;
	uid_t           uid;
	gid_t           gid;
	int             err, fd, fdtmp, match, count, ret;
	char            inbuf[8192], outbuf[512];
	struct substdio ssin, ssout;

	if ((fd = open_read(subscriber_list)) == -1)
		strerr_die3sys(111, "bulletin: open: ", subscriber_list, ": ");
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (err = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("bulletin: read: ", subscriber_list, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (p = line.s; *p && isspace((int) *p); p++);
		if (!*p)
			continue;
		match = str_chr(p, '@');
		if (!p[match]) {
			strerr_warn3("bulletin: invalid email address [", p, "]", 0);
			err++;
			continue;
		} else
			domain = p + match + 1;
		q = p;
		/*- domain:host:port */
		for (p = findmdahost(q, 0); p && *p && *p != ':'; p++);
		if (p) {
			p++; /*- host:port */
			for (r = p; r && *r && *r != ':'; r++); /*- :port */
			*r = 0;
			if (store_email(p, domain, q)) {
				err++;
				continue;
			}
		} else
			strerr_warn2(q, ": No such user", 0);
	}
	close(fd);
	for (mdaptr = mdaHOSTS; mdaptr && *mdaptr; mdaptr++) {
		emailptr = (*mdaptr)->emailptr;
		match = str_chr(*emailptr, '@');
		if (*emailptr[match])
			domain = *emailptr + match + 1;
		else {
			strerr_warn3("bulletin: invalid MDA [", *emailptr, "]", 0);
			err++;
			continue;
		}
		if (!get_assign(domain, 0, &uid, &gid)) {
			strerr_warn2(domain, ": No such domain", 0);
			continue;
		}
		if (!(tmpdir = env_get("TMPDIR")))
			tmpdir = "/tmp";
		if (!stralloc_copys(&SplitFile, tmpdir) ||
				!stralloc_catb(&SplitFile, "/indi.", 6) ||
				!stralloc_cats(&SplitFile, (*mdaptr)->mdahost) ||
				!stralloc_catb(&SplitFile, ".XXXXXX", 7) ||
				!stralloc_0(&SplitFile))
			die_nomem();
		if ((fdtmp = mkstemp(SplitFile.s)) == -1)
			strerr_die3sys(111, "bulletin: mkstemp: ", SplitFile.s, ": ");
		if (chown(SplitFile.s, uid, gid) || chmod(SplitFile.s, INDIMAIL_QMAIL_MODE))
			strerr_die3sys(111, "bulletin: chown/chmod: ", SplitFile.s, ": ");
		substdio_fdbuf(&ssout, write, fdtmp, outbuf, sizeof(outbuf));
		for (; *emailptr; emailptr++) {
			if (substdio_puts(&ssout, *emailptr) ||
					substdio_put(&ssout, "\n", 1))
				strerr_die3sys(111, "bulletin: write: ", SplitFile.s, ": ");
		}
		if (substdio_flush(&ssout))
			strerr_die3sys(111, "bulletin: write: ", SplitFile.s, ": ");
		close(fdtmp);
		if (!stralloc_copys(&tmpbuf, (*mdaptr)->mdahost) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		match = str_chr(tmpbuf.s, '@');
		if (tmpbuf.s[match])
			tmpbuf.s[match] = 0;
		if (!(mysqlptr = mdaMySQLConnect(tmpbuf.s, domain))) {
			strerr_warn3("bulletin: unable to locate MySQL Host for [", tmpbuf.s, "]", 0);
			err++;
			unlink(SplitFile.s);
			continue;
		}
		sql_init(1, *mysqlptr);
		if ((ret = insert_bulletin(domain, emailFile, SplitFile.s)) == -1)
			err++;
		else
			count += ret;
		unlink(SplitFile.s);
		is_open = 0; /*- do not close main connection set by mysqlptr */
	} /*- for (mdaptr = mdaHOSTS;*mdaptr;mdaptr++) */
	return (err ? -1 : count);
}
#else
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <stralloc.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <str.h>
#endif

long
bulletin(char *emailFile, char *subscriber_list)
{
	int             fd, match;
	char           *p, *domain;
	char            inbuf[8192];
	static stralloc line = {0};
	struct substdio ssin;

	if ((fd = open_read(emailFile)) == -1) {
		strerr_warn3("bulletin: open: ", emailFile, ": ", &strerr_sys);
		return (-1);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("bulletin: read: ", emailFile, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (p = line.s; *p && isspace((int) *p); p++);
		if (!*p)
			continue;
		match = str_chr(p, '@');
		if (!p[match]) {
			strerr_warn3("bulletin: invalid email address [", p, "]", 0);
			err++;
			continue;
		} else
			domain = p + match + 1;
	}
	close(fd);
	if (!domain) {
		strerr_warn1("bulletin: No domain specified", 0);
		return (-1);
	}
	return (insert_bulletin(domain, emailFile, subscriber_list));
}
#endif /*- #ifdef CLUSTERED_SITE */
