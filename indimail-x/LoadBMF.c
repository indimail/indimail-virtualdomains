/*
 * $Id: LoadBMF.c,v 1.7 2025-05-13 20:01:07+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: LoadBMF.c,v 1.7 2025-05-13 20:01:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <str.h>
#include <scan.h>
#include <subfd.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#include <getEnvConfig.h>
#endif
#include "common.h"
#include "variables.h"
#include "open_master.h"
#include "create_table.h"
#include "disable_mysql_escape.h"

static char   **LoadBMF_internal(int *, char *);
static time_t   BMFTimestamp(int, char *);

static void
die_nomem()
{
	strerr_warn1("LoadBMF: out of memory", 0);
	_exit(111);
}

/*
 * loads entry from file defined by environment variable BADMAILFROM. If not defined, the qmail
 * control file badmailfrom is used.
 * After successful updation, utime of the file is updated to the current time
 */
int
UpdateSpamTable(char *bmf)
{
	static stralloc SqlBuf = {0}, badmailfrom = {0};
	char           *sysconfdir, *controldir;
	int             badmail_flag, err, es_opt = 0;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&badmailfrom, controldir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, bmf) ||
				!stralloc_0(&badmailfrom))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&badmailfrom, sysconfdir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, controldir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, bmf) ||
				!stralloc_0(&badmailfrom))
			die_nomem();
	}
	badmailfrom.len--;
	if (access(badmailfrom.s, F_OK)) {
		strerr_warn3("UpdateSpamTable: access: ", badmailfrom.s, ": ", &strerr_sys);
		return (-1);
	}
	if (verbose) {
		subprintfe(subfdout, "LoadBMF", "Updating Table %s\n", bmf);
		flush("LoadBMF");
	}
	if (!str_diffn(bmf, "badmailfrom", 12) || !str_diffn(bmf, "badrcptto", 10) || !str_diffn(bmf, "spamdb", 7))
		badmail_flag = 1;
	else
		badmail_flag = 0;
	if (open_master()) {
		strerr_warn1("LoadBMF: failed to open master db", 0);
		return (-1);
	}
	if (badmail_flag) {
		if (!stralloc_copyb(&SqlBuf, "LOAD DATA LOW_PRIORITY LOCAL INFILE \"", 37) ||
				!stralloc_cat(&SqlBuf, &badmailfrom) ||
				!stralloc_catb(&SqlBuf, "\" IGNORE INTO TABLE ", 20) ||
				!stralloc_cats(&SqlBuf, bmf) ||
				!stralloc_catb(&SqlBuf, " (email)", 8) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		es_opt = disable_mysql_escape(1);
		if (!stralloc_copyb(&SqlBuf, "LOAD DATA LOW_PRIORITY LOCAL INFILE \"", 37) ||
				!stralloc_cat(&SqlBuf, &badmailfrom) ||
				!stralloc_catb(&SqlBuf, "\" REPLACE INTO TABLE spam fields terminated by ' ' (email, spam_count)", 70) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, badmail_flag == 1 ? bmf : "spam",
				badmail_flag == 1 ? BADMAILFROM_TABLE_LAYOUT : SPAM_TABLE_LAYOUT))
			if (mysql_query(&mysql[0], SqlBuf.s)) {
				strerr_warn4("LoadBMF: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
				if (!badmail_flag)
					disable_mysql_escape(es_opt);
				return (-1);
			}
		} else {
			if (!badmail_flag)
				disable_mysql_escape(es_opt);
			strerr_warn4("LoadBMF: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
			return (-1);
		}
	}
	if (!badmail_flag)
		disable_mysql_escape(es_opt);
	if ((err = in_mysql_affected_rows(&mysql[0])) == -1) {
		strerr_warn2("LoadBMF: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	/*
	 * If err is 0, possibility is file has been  updated without any content
	 * being changed.
	 */
	if (err && utimes(badmailfrom.s, 0))
		strerr_warn3("LoadBMF: utime: ", badmailfrom.s, ": ", &strerr_sys);
	if (verbose) {
		subprintfe(subfdout, "LoadBMF", "%d rows affected\n", err);
		flush("LoadBMF");
	}
	return (err);
}

/*
 * synchronizes table badmailfrom/badrcptto/spamdb with control file
 * "badmailfrom", "badrcptto", "spamdb" or any other spam format file
 */
char **
LoadBMF(int *total, char *bmf)
{
	static stralloc SqlBuf = {0}, badmailfrom = {0};
	struct stat     statbuf;
	int             num_rows, fd, err = 0, sync_file = 0, sync_mcd = 0, badmail_flag;
	MYSQL_RES      *res = 0;
	MYSQL_ROW       row;
	time_t          mtime = 0l, mcd_time = 0l, file_time = 0l;
	char           *sysconfdir, *controldir;
	struct timeval  ubuf[2] = {0};
	char            outbuf[512];
	struct substdio ssout;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (total)
		*total = 0;
	if (*controldir == '/') {
		if (!stralloc_copys(&badmailfrom, controldir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, bmf) ||
				!stralloc_0(&badmailfrom))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&badmailfrom, sysconfdir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, controldir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, bmf) ||
				!stralloc_0(&badmailfrom))
			die_nomem();
	}
	badmailfrom.len--;
	if (stat(badmailfrom.s, &statbuf)) {
		sync_file = 1;
		file_time = 0l;
	} else {
		file_time = statbuf.st_mtime;
		if (verbose) {
			subprintfe(subfdout, "LoadBMF", "File UNIX %-40s Modification Time %s",
						badmailfrom.s, ctime(&file_time));
			flush("LoadBMF");
		}
	}
	if (open_master()) {
		if (sync_file) {
			strerr_warn1("LoadBMF: failed to open master db", 0);
			return ((char **) 0);
		} else
			return (LoadBMF_internal(total, bmf));
	}
	if (!str_diffn(bmf, "badmailfrom", 12) || !str_diffn(bmf, "badrcptto", 10) || !str_diffn(bmf, "spamdb", 7))
		badmail_flag = 1;
	else
		badmail_flag = 0;
	if (badmail_flag) {
		if (!stralloc_copyb(&SqlBuf, "select email, UNIX_TIMESTAMP(timestamp) from ", 45) ||
				!stralloc_cats(&SqlBuf, bmf) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		if (!stralloc_copyb(&SqlBuf, "select email, spam_count, UNIX_TIMESTAMP(timestamp) from spam", 61) ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	}
	if (mysql_query(&mysql[0], SqlBuf.s) && ((err = in_mysql_errno(&mysql[0])) != ER_NO_SUCH_TABLE)) {
		strerr_warn4("LoadBMF: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		if (sync_file)
			return ((char **) 0);
		else
			return (LoadBMF_internal(total, bmf));
	}
	if (err == ER_NO_SUCH_TABLE) {
		if (create_table(ON_MASTER, badmail_flag == 1 ? bmf : "spam",
			badmail_flag == 1 ? BADMAILFROM_TABLE_LAYOUT : SPAM_TABLE_LAYOUT)) {
			if (sync_file)
				return ((char **) 0);
			else
				return (LoadBMF_internal(total, bmf));
		}
		sync_mcd = 1;
	} else {
		if (!(res = in_mysql_store_result(&mysql[0]))) {
			strerr_warn2("LoadBMF: in_mysql_store_results: ", (char *) in_mysql_error(&mysql[0]), 0);
			if (sync_file)
				return ((char **) 0);
			else
				return (LoadBMF_internal(total, bmf));
		}
		if (!(num_rows = in_mysql_num_rows(res))) {
			if (sync_file) {
				in_mysql_free_result(res);
				return ((char **) 0);
			}
			sync_mcd = 1;
			in_mysql_free_result(res);
			res = 0;
		} else {
			for (mcd_time = 0l; (row = in_mysql_fetch_row(res));) {
				if (badmail_flag)
					scan_ulong(row[1], (unsigned long *) &mtime);
				else
					scan_ulong(row[2], (unsigned long *) &mtime);
				if (mtime > mcd_time)
					mcd_time = mtime;
			}
			if (verbose) {
				subprintfe(subfdout, "LoadBMF", "Table MySQL %-40s Modification Time %s",
							badmail_flag ? bmf : "spam", ctime(&mcd_time));
				if (mcd_time == file_time)
					subprintfe(subfdout, "LoadBMF", "Nothing to update\n");
				flush("LoadBMF");
			}
			if (mcd_time == file_time) {
				in_mysql_free_result(res);
				return (LoadBMF_internal(total, bmf));
			} else
			if (mcd_time > file_time) {
				sync_file = 1;
				sync_mcd = 0;
				if ((err = UpdateSpamTable(bmf)) > 0) { /*- Reload entries from mysql if this happens */
					if (verbose) {
						subprintfe(subfdout, "LoadBMF", "Reloading Table %s\n", badmail_flag ? bmf : "spam");
						flush("LoadBMF");
					}
					in_mysql_free_result(res);
					if (mysql_query(&mysql[0], SqlBuf.s)) {
						strerr_warn4("LoadBMF: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
						return ((char **) 0);
					}
					if (!(res = in_mysql_store_result(&mysql[0]))) {
						strerr_warn2("LoadBMF: in_mysql_store_results: ", (char *) in_mysql_error(&mysql[0]), 0);
						return ((char **) 0);
					}
					if (!(num_rows = in_mysql_num_rows(res))) { /*- should never happen */
						in_mysql_free_result(res);
						return ((char **) 0);
					}
				}
			} else if (mcd_time < file_time)
			{
				sync_file = 0;
				sync_mcd = 1;
			}
		}
	}
	if (sync_mcd) {
		if (res)
			in_mysql_free_result(res);
		if ((err = UpdateSpamTable(bmf)) == -1)
			return ((char **) 0);
		else {
			if (verbose) {
				subprintfe(subfdout, "LoadBMF", "Syncing time of %s with Table %s\n", badmailfrom.s, bmf);
				flush("LoadBMF");
			}
			if (!err && (file_time = BMFTimestamp(badmail_flag, bmf)) == -1) {
				strerr_warn1("LoadBMF: Invalid TIMESTAMP: Internal BUG", 0);
				return ((char **) 0);
			}
			ubuf[0].tv_sec = ubuf[1].tv_sec = (err ? time(0) : file_time);
			if (ubuf[0].tv_sec && utimes(badmailfrom.s, ubuf))
				strerr_warn3("LoadBMF: utime: ", badmailfrom.s, ": ", &strerr_sys);
			return (LoadBMF_internal(total, bmf));
		}
	} else
	if (sync_file && res) {
		if (verbose) {
			subprintfe(subfdout, "LoadBMF", "Updating File %s\n", badmailfrom.s);
			flush("LoadBMF");
		}
		if ((fd = open_trunc(badmailfrom.s)) == -1) {
			strerr_warn3("LoadBMF: open: ", badmailfrom.s, ": ", &strerr_sys);
			in_mysql_free_result(res);
			return ((char **) 0);
		}
		substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fd, outbuf, sizeof(outbuf));
		in_mysql_data_seek(res, 0);
		for (;(row = in_mysql_fetch_row(res));) {
			if (badmail_flag) {
				if (substdio_puts(&ssout, row[0]) ||
						substdio_put(&ssout, "\n", 1)) {
					strerr_warn3("LoadBMF: write: ", badmailfrom.s, ": ", &strerr_sys);
				}
			} else {
				if (substdio_puts(&ssout, row[0]) ||
						substdio_put(&ssout, " ", 1) ||
						substdio_puts(&ssout, row[1]) ||
						substdio_put(&ssout, "\n", 1)) {
					strerr_warn3("LoadBMF: write: ", badmailfrom.s, ": ", &strerr_sys);
					close(fd);
					return ((char **) 0);
				}
			}
		}
		in_mysql_free_result(res);
		if (substdio_flush(&ssout)) {
			close(fd);
			return ((char **) 0);
		}
		close(fd);
		ubuf[0].tv_sec = time(0);
		ubuf[1].tv_sec = mcd_time;
		if (utimes(badmailfrom.s, ubuf))
			strerr_warn3("LoadBMF: utime: ", badmailfrom.s, ": ", &strerr_sys);
	} else
	if (verbose) {
		subprintfe(subfdout, "LoadBMF", "Nothing to update\n");
		flush("LoadBMF");
	}
	return (LoadBMF_internal(total, bmf));
}

static char **
LoadBMF_internal(int *total, char *bmf)
{
	char           *ptr, *sysconfdir, *controldir;
	static stralloc line = {0}, badmailfrom = {0};
	int             count, fd, match;
	static int      _count;
	static char   **bmfptr;
	static time_t   file_time;
	struct stat     statbuf;
	char            inbuf[4096];
	struct substdio ssin;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&badmailfrom, controldir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, bmf) ||
				!stralloc_0(&badmailfrom))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&badmailfrom, sysconfdir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, controldir) ||
				!stralloc_append(&badmailfrom, "/") ||
				!stralloc_cats(&badmailfrom, bmf) ||
				!stralloc_0(&badmailfrom))
			die_nomem();
	}
	badmailfrom.len--;
	if (total)
		*total = 0;
	if (stat(badmailfrom.s, &statbuf))
		return ((char **) 0);
	if (bmfptr && (statbuf.st_mtime == file_time)) {
		if (total)
			*total = _count;
		return (bmfptr);
	}
	file_time = statbuf.st_mtime;
	if ((fd = open_read(badmailfrom.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "LoadBMF: open: ", badmailfrom.s, ": ");
		return ((char **) 0);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (count = 0;;count++) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("LoadBMF: read: ", badmailfrom.s, ": ", &strerr_sys);
			close(fd);
			return ((char **) 0);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		count++;
	}
	if (total)
		*total = -1;
	_count = -1;
	if (!count) {
		close(fd);
		return ((char **) 0);
	}
	if (!(bmfptr = (char **) alloc(sizeof(char *) * (count + 1))))
		die_nomem();
	if (lseek(fd, 0, SEEK_SET) != 0) {
		strerr_warn1("LoadBMF: lseek error: ", &strerr_sys);
		close(fd);
		return ((char **) 0);
	}
	ssin.p = 0;
	ssin.n = sizeof(inbuf);
	for (count = 1;;count++) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("LoadBMF: read: ", badmailfrom.s, ": ", &strerr_sys);
			close(fd);
			return ((char **) 0);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!(bmfptr[count - 1] = (char *) alloc(sizeof(char) * (line.len +1 - (ptr - line.s)))))
			die_nomem();
		str_copyb(bmfptr[count - 1], ptr, line.len + 1 - (ptr - line.s));
	}
	close(fd);
	if (total)
		*total = (count - 1);
	_count = count - 1;
	bmfptr[count - 1] = 0;
	return (bmfptr);
}

static time_t
BMFTimestamp(int badmail_flag, char *bmf)
{
	static stralloc SqlBuf = {0};
	int             num_rows, err;
	MYSQL_RES      *res = 0;
	MYSQL_ROW       row;
	time_t          mtime = 0l, mcd_time = 0l;

	if (!stralloc_copyb(&SqlBuf, "select UNIX_TIMESTAMP(timestamp) from ", 38) ||
			!stralloc_cats(&SqlBuf, badmail_flag ? bmf : "spam") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if ((err = in_mysql_errno(&mysql[0])) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, badmail_flag == 1 ? bmf : "spam",
				badmail_flag == 1 ? BADMAILFROM_TABLE_LAYOUT : SPAM_TABLE_LAYOUT);
		else
			strerr_warn4("LoadBMF: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("LoadBMF: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	if (!(num_rows = in_mysql_num_rows(res))) {
		in_mysql_free_result(res);
		return (0);
	}
	for (mcd_time = 0l;(row = in_mysql_fetch_row(res));) {
		scan_ulong(row[0], (unsigned long *) &mtime);
		if (mtime > mcd_time)
			mcd_time = mtime;
	}
	in_mysql_free_result(res);
	if (!mcd_time)
		return (-1);
	return (mcd_time);
}
#endif /*- CLUSTERED_SITE */
/*
 * $Log: LoadBMF.c,v $
 * Revision 1.7  2025-05-13 20:01:07+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.6  2023-03-23 23:10:01+05:30  Cprogrammer
 * fix wrong counts when badmailfrom has comments
 *
 * Revision 1.5  2023-03-20 10:11:13+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.4  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.3  2020-07-04 22:53:05+05:30  Cprogrammer
 * replaced utime() with utimes()
 *
 * Revision 1.2  2020-04-01 18:56:45+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-15 13:05:29+05:30  Cprogrammer
 * Initial revision
 *
 */
