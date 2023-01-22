/*
 * $Log: LoadDbInfo.c,v $
 * Revision 1.12  2021-07-27 18:05:36+05:30  Cprogrammer
 * set default domain using vset_default_domain
 *
 * Revision 1.11  2020-10-18 07:51:37+05:30  Cprogrammer
 * initialize last_error_len field of dbinfo
 *
 * Revision 1.10  2020-07-04 22:53:56+05:30  Cprogrammer
 * replaced utime() with utimes()
 *
 * Revision 1.9  2020-05-12 19:23:58+05:30  Cprogrammer
 * BUG - uninitialized relayhosts variable
 *
 * Revision 1.8  2020-04-30 19:25:15+05:30  Cprogrammer
 * changed scope of ssin, ssout variables to local
 *
 * Revision 1.7  2020-04-01 18:56:49+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.6  2019-07-02 09:51:38+05:30  Cprogrammer
 * Null terminate RelayHosts before calling writemcdinfo
 *
 * Revision 1.5  2019-06-27 10:46:31+05:30  Cprogrammer
 * use newline as a separator between records
 *
 * Revision 1.4  2019-06-07 10:51:50+05:30  Cprogrammer
 * fix SIGSEGV on hosts with mcdfile missing (non-distributed domains)
 *
 * Revision 1.3  2019-05-27 20:34:37+05:30  Cprogrammer
 * initialize socket & port after allocating dbinfo structure
 *
 * Revision 1.2  2019-04-22 23:13:33+05:30  Cprogrammer
 * replaced atoi(), atol() with scan_int(), scan_ulong() functions
 *
 * Revision 1.1  2019-04-17 12:49:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <str.h>
#include <open.h>
#include <getln.h>
#include <stralloc.h>
#include <substdio.h>
#include <subfd.h>
#include <fmt.h>
#include <error.h>
#include <strerr.h>
#include <scan.h>
#include <env.h>
#include <getEnvConfig.h>
#include <subfd.h>
#endif
#include "create_table.h"
#include "get_local_ip.h"
#include "variables.h"
#include "open_master.h"
#include "get_indimailuidgid.h"
#include "is_alias_domain.h"
#include "isvirtualdomain.h"
#include "common.h"
#include "check_group.h"
#include "vset_default_domain.h"

#ifndef	lint
static char     sccsid[] = "$Id: LoadDbInfo.c,v 1.12 2021-07-27 18:05:36+05:30 Cprogrammer Exp mbhangui $";
#endif

static DBINFO **loadMCDInfo(int *);
static DBINFO **localDbInfo(int *, DBINFO ***);

static int      _total;

int
loadDbinfoTotal()
{
	return (_total);
}

#ifdef CLUSTERED_SITE
static int      delete_dbinfo_rows(char *);

static stralloc SqlBuf = {0}, mcdFile = {0}, line = { 0 }, filename = {0};
static char     inbuf[4096], outbuf[256];
static char     strnum1[FMT_ULONG], strnum2[FMT_ULONG];

static void
die_nomem()
{
	strerr_warn1("LoadDbInfo: out of memory", 0);
	_exit(111);
}

/*
 * write dbinfo records to mcdinfo
 */
int
writemcdinfo(DBINFO **rhostsptr, time_t mtime)
{
	char           *sysconfdir, *mcdfile, *controldir;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             fd, idx;
	uid_t           uid, uidtmp;
	gid_t           gid, gidtmp;
	struct timeval  ubuf[2] = {0};
	DBINFO        **ptr;
	struct substdio ssout;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.') {
		if (!stralloc_copys(&mcdFile, mcdfile) ||
			!stralloc_0(&mcdFile))
			die_nomem();
	} else 
	if (*controldir == '/') {
		if (!stralloc_copys(&mcdFile, controldir) ||
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, mcdfile) ||
			!stralloc_0(&mcdFile))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&mcdFile, sysconfdir) ||
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, controldir) ||
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, mcdfile) ||
			!stralloc_0(&mcdFile))
			die_nomem();
	}
	mcdFile.len--;
	if (!rhostsptr)
		return (1);
	if ((fd = open(mcdFile.s, O_CREAT|O_WRONLY, INDIMAIL_QMAIL_MODE)) == -1)
		strerr_die3sys(111, "LoadDbInfo: open-write: ", mcdFile.s, ": ");
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = indimailuid;
	gid = indimailgid;
	uidtmp = getuid();
	gidtmp = getgid();
	if (uidtmp != 0 && uidtmp != uid && gidtmp != gid && check_group(gid, 0) != 1) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("LoadDbInfo: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	if (fchown(fd, uid, gid))
		strerr_die3sys(111, "writemcdinfo: fchown: ", mcdFile.s, ": ");
	for (ptr = rhostsptr, idx = 0;(*ptr);idx++, ptr++) {
		if ((*ptr)->isLocal)
			continue;
		if (substdio_put(&ssout, "domain   ", 9) ||
			substdio_puts(&ssout, (*ptr)->domain) ||
			substdio_put(&ssout, (*ptr)->distributed ? " 1\n" : " 0\n", 3) ||
			substdio_put(&ssout, "server   ", 9) ||
			substdio_puts(&ssout, (*ptr)->server) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "mdahost  ", 9) ||
			substdio_puts(&ssout, (*ptr)->mdahost) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "port     ", 9) ||
			substdio_put(&ssout, strnum1, fmt_uint(strnum1, (*ptr)->port)) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "use_ssl  ", 9) ||
			substdio_put(&ssout, (*ptr)->use_ssl ? "1\n" : "0\n", 2) ||
			substdio_put(&ssout, "database ", 9) ||
			substdio_puts(&ssout, (*ptr)->database) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "user     ", 9) ||
			substdio_puts(&ssout, (*ptr)->user) ||
			substdio_put(&ssout, "\n", 1) ||
			substdio_put(&ssout, "pass     ", 9) ||
			substdio_puts(&ssout, (*ptr)->password) ||
			substdio_put(&ssout, "\n\n", 2) ||
			substdio_flush(&ssout))
			strerr_die3sys(111, "LoadDbInfo: write error: ", mcdFile.s, ": ");
	}
	close(fd);
	ubuf[0].tv_sec = time(0);
	ubuf[1].tv_sec = mtime;
	if (utimes(mcdFile.s, ubuf))
		strerr_die3sys(111, "writemcdinfo: utime: ", mcdFile.s, ": ");
	return (0);
}

DBINFO **
LoadDbInfo_TXT(int *total)
{
	struct stat     statbuf;
	int             num_rows, idx, tmp, err = 0, sync_file = 0, sync_mcd = 0, relative;
	DBINFO        **ptr, **relayhosts;
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	time_t          mtime = 0l, mcd_time = 0l, file_time = 0l;
	char           *sysconfdir, *mcdfile, *controldir;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
	if (relative) {
		if (!stralloc_copys(&filename, sysconfdir) ||
			!stralloc_append(&filename, "/") ||
			!stralloc_cats(&filename, controldir))
			die_nomem();
	} else 
	if (!stralloc_copys(&filename, controldir) ||
		!stralloc_catb(&filename, "/host.master", 12) ||
		!stralloc_0(&filename))
		die_nomem();
	if (total)
		_total = *total = 0;
	if (access(filename.s, F_OK)) /*- return dbinfo structure loaded from local mcdinfo if host.master is absent */
		return (loadMCDInfo(total));
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.') {
		if (!stralloc_copys(&mcdFile, mcdfile) || !stralloc_0(&mcdFile))
			die_nomem();
	} else
	if (relative) {
		if (!stralloc_copys(&mcdFile, sysconfdir) ||
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, controldir) ||
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, mcdfile) ||
			!stralloc_0(&mcdFile))
			die_nomem();
	} else {
		if (!stralloc_copys(&mcdFile, controldir) ||
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, mcdfile) ||
			!stralloc_0(&mcdFile))
			die_nomem();
	}
	mcdFile.len--;
	if (stat(mcdFile.s, &statbuf)) {
		if (verbose)
			strerr_warn3("LoadDbInfo: stat: ", mcdFile.s, ": ", &strerr_sys);
		sync_file = 1;
		file_time = 0l;
	} else {
		file_time = statbuf.st_mtime;
		if (verbose) {
			subprintfe(subfdout, "LoadDbInfo", "File UNIX  %-40s Modification Time %s", mcdFile.s, ctime(&file_time));
			flush("LoadDbInfo");
		}
	}
	if (open_master()) {
		if (sync_file) { /*- in absense of mcdinfo, we can't proceed further */
			strerr_warn1("LoadDbInfo: Failed to open master db", 0);
			return ((DBINFO **) 0);
		} else /*- get records from mcdinfo file */
			return (loadMCDInfo(total));
	}
	if (!stralloc_copys(&SqlBuf, "select UNIX_TIMESTAMP(timestamp) from dbinfo where filename=\"") ||
		!stralloc_cat(&SqlBuf, &mcdFile) ||
		!stralloc_append(&SqlBuf, "\"") ||
		!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s) && ((err = in_mysql_errno(&mysql[0])) != ER_NO_SUCH_TABLE)) {
		strerr_warn4("LoadDbInfo: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		if (sync_file) /*- in absense of mcdinfo, we can't proceed further */
			return ((DBINFO **) 0);
		else /*- get records from mcdinfo file */
			return (loadMCDInfo(total));
	}
	if (err == ER_NO_SUCH_TABLE) {
		if (create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT)) {
			if (sync_file) /*- in absense of mcdinfo, we can't proceed further */
				return ((DBINFO **) 0);
			else /*- get records from mcdinfo file */
				return (loadMCDInfo(total));
		}
		sync_mcd = 1;
	} else { /*- figure out if mcdfile or dbinfo needs to be updated */
		if (!(res = in_mysql_store_result(&mysql[0]))) {
			strerr_warn2("LoadDbInfo: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
			if (sync_file)
				return ((DBINFO **) 0);
			else /*- get records from mcdinfo file */
				return (loadMCDInfo(total));
		}
		if (!(num_rows = in_mysql_num_rows(res))) { /*- dbinfo table is empty */
			if (sync_file) /*- there are no records so no point in updating mcdfile */
				return (loadMCDInfo(total));
			sync_mcd = 1;
			row = in_mysql_fetch_row(res);
			in_mysql_free_result(res);
		} else { /*- figure out which is newer - dbinfo or mcdinfo */
			for (mcd_time = 0l;(row = in_mysql_fetch_row(res));) {
				scan_long(row[0], &mtime);
				if (mtime > mcd_time)
					mcd_time = mtime; /*- get the time of the newest dbinfo record */
			}
			in_mysql_free_result(res);
			if (verbose) {
				subprintfe(subfdout, "LoadDbInfo", "Table MySQL %-40s Modification Time %s", mcdFile.s, ctime(&mcd_time));
				flush("LoadDbInfo");
				if (mcd_time == file_time) {
					out("LoadDbInfo", "Nothing to update\n");
					flush("LoadDbInfo");
				}
			}
			if (mcd_time == file_time) /*- nothing to update */
				return (loadMCDInfo(total));
			else
			if (mcd_time > file_time) {
				sync_file = 1;
				sync_mcd = 0;
			} else
			if (mcd_time < file_time) {
				sync_file = 0;
				sync_mcd = 1;
			}
		}
	}
	if (sync_mcd) {  /*- sync dbinfo table */
		/* 
		 * update dbinfo table with latest modification in mcdfile
		 * and time = file modification time of mcdinfo
		 */
		if (verbose) {
			out("LoadDbInfo", "Updating Table dbinfo\n");
			flush("LoadDbInfo");
		}
		if (!(relayhosts = loadMCDInfo(total))) {
			strerr_warn1("LoadDbInnfo_TXT: loadMCDInfo", 0);
			return ((DBINFO **) 0);
		}
		if (delete_dbinfo_rows(mcdFile.s)) /*- delete from dbinfo records for mcdFile */
			return (relayhosts);
		for (err = 0, ptr = relayhosts;(*ptr);ptr++) {
			if ((*ptr)->isLocal) /*- don't insert dbinfo obtained from localDbInfo() */
				continue;
			strnum1[fmt_uint(strnum1, (*ptr)->port)] = 0;
			strnum2[fmt_ulong(strnum2, file_time)] = 0;
			if (!stralloc_copys(&SqlBuf, "replace low_priority into dbinfo ") ||
				!stralloc_cats(&SqlBuf, "(filename, domain, distributed, server, mdahost, ") ||
				!stralloc_cats(&SqlBuf, "port, use_ssl, dbname, user, passwd, timestamp) ") ||
				!stralloc_cats(&SqlBuf, "values (") ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_cat(&SqlBuf, &mcdFile) ||
				!stralloc_catb(&SqlBuf, "\",\"", 3) ||
				!stralloc_cats(&SqlBuf, (*ptr)->domain) ||
				!stralloc_catb(&SqlBuf, "\",", 2) ||
				!stralloc_catb(&SqlBuf, (*ptr)->distributed ? "1,\"" : "0,\"", 3) ||
				!stralloc_cats(&SqlBuf, (*ptr)->server) ||
				!stralloc_catb(&SqlBuf, "\",\"", 3) ||
				!stralloc_cats(&SqlBuf, (*ptr)->mdahost) ||
				!stralloc_catb(&SqlBuf, "\",", 2) ||
				!stralloc_cats(&SqlBuf, strnum1) ||
				!stralloc_append(&SqlBuf, ",") ||
				!stralloc_catb(&SqlBuf, (*ptr)->use_ssl ? "1,\"" : "0,\"", 3) ||
				!stralloc_cats(&SqlBuf, (*ptr)->database) ||
				!stralloc_catb(&SqlBuf, "\",\"", 3) ||
				!stralloc_cats(&SqlBuf, (*ptr)->user) ||
				!stralloc_catb(&SqlBuf, "\",\"", 3) ||
				!stralloc_cats(&SqlBuf, (*ptr)->password) ||
				!stralloc_catb(&SqlBuf, "\",", 2) ||
				!stralloc_catb(&SqlBuf, "FROM_UNIXTIME(", 14) ||
				!stralloc_cats(&SqlBuf, strnum2) ||
				!stralloc_catb(&SqlBuf, ") - 0)", 6) ||
				!stralloc_0(&SqlBuf))
				die_nomem();
			if (mysql_query(&mysql[0], SqlBuf.s)) {
				strerr_warn4("LoadDbInfo: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
				err = 1;
				continue;
			}
		}
		return (relayhosts);
	} else
	if (sync_file) {
		if (verbose) {
			subprintfe(subfdout, "LoadDbInfo", "Updating File %s\n", mcdFile.s);
			flush("LoadDbInfo");
		}
		if (!stralloc_copyb(&SqlBuf, "select high_priority domain, distributed, server, ", 50) ||
			!stralloc_catb(&SqlBuf, "mdahost, port, use_ssl, dbname, user, passwd, timestamp ", 56) ||
			!stralloc_catb(&SqlBuf, "from dbinfo where filename=\"", 28) ||
			!stralloc_cat(&SqlBuf,  &mcdFile) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[0], SqlBuf.s)) {
			strerr_warn4("LoadDbInfo: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
			if (access(mcdFile.s, F_OK)) /*- no records in dbinfo as well as mcdinfo */
				return ((DBINFO **) 0);
			else
				return (loadMCDInfo(total));
		}
		if (!(res = in_mysql_store_result(&mysql[0]))) {
			strerr_warn2("LoadDbInfo: in_mysql_store_result: ", (char *) in_mysql_error(&mysql[0]), 0);
			if (access(mcdFile.s, F_OK)) /*- no records in dbinfo as well as mcdinfo */
				return ((DBINFO **) 0);
			else
				return (loadMCDInfo(total));
		}
		if (!(num_rows = in_mysql_num_rows(res))) {
			in_mysql_free_result(res);
			strerr_warn1("LoadDbInfo: No rows selected", 0);
			if (access(mcdFile.s, F_OK)) /*- no records in dbinfo as well as mcdinfo */
				return ((DBINFO **) 0);
			else
				return (loadMCDInfo(total));
		}
		if (total)
			_total = (*total += num_rows);
		if (!(relayhosts = (DBINFO **) alloc(sizeof(DBINFO *) * (num_rows + 1))))
			die_nomem();
		for (ptr = relayhosts, idx = 0;(row = in_mysql_fetch_row(res));idx++, ptr++) {
			if (!((*ptr) = (DBINFO *) alloc(sizeof(DBINFO))))
				die_nomem();
			str_copyb((*ptr)->domain, row[0], DBINFO_BUFF);
			scan_int(row[1], &(*ptr)->distributed);
			str_copyb((*ptr)->server, row[2], DBINFO_BUFF);
			str_copyb((*ptr)->mdahost, row[3], DBINFO_BUFF);
			scan_int(row[4], &(*ptr)->port);
			(*ptr)->socket = (char *) 0;
			scan_int(row[5], &tmp);
			(*ptr)->use_ssl = tmp ? 1 : 0;
			str_copyb((*ptr)->database, row[6], DBINFO_BUFF);
			str_copyb((*ptr)->user, row[7], DBINFO_BUFF);
			str_copyb((*ptr)->password, row[8], DBINFO_BUFF);
			(*ptr)->fd = -1;
			(*ptr)->last_error = 0;
			(*ptr)->last_error_len = 0;
			(*ptr)->failed_attempts = 0;
			(*ptr)->isLocal = 0;
		}
		(*ptr) = (DBINFO *) 0;
		in_mysql_free_result(res);
 		/* write dbinfo records to mcdinfo and set file modification time to mcd_time */
		if (writemcdinfo(relayhosts, mcd_time)) {
			strerr_warn1("LoadDbInfo: writemcdinfo failed", 0);
		}
		return (relayhosts);
	} else
	if (verbose) {
		out("LoadDbInfo", "Nothing to update\n");
		flush("LoadDbInfo");
	}
	return (loadMCDInfo(total));
}

static int
delete_dbinfo_rows(char *filename)
{
	if (open_master()) {
		strerr_warn1("LoadDbInfo: delete_dbinfo_rows: Failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copys(&SqlBuf, "delete low_priority from dbinfo where filename=\"") ||
		!stralloc_cats(&SqlBuf,  filename) ||
		!stralloc_append(&SqlBuf, "\"") ||
		!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		strerr_warn4("LoadDbInfo: mysql_query: [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	}
	return (0);
}
#else
DBINFO **
LoadDbInfo_TXT(int *total)
{
	return (loadMCDInfo(total));
}
#endif

/*
 * Load records fromm the file mcdinfo
 */
static DBINFO **
loadMCDInfo(int *total)
{
	char            dombuf[DBINFO_BUFF];
	char           *sysconfdir, *mcdfile, *controldir, *ptr; 
	int             t, count, items, distributed, fd, match;
	DBINFO        **relayhosts, **rhostsptr;
	static stralloc dummy1 = {0}, dummy2 = {0};
	struct substdio ssin;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.') {
		if (!stralloc_copys(&mcdFile, mcdfile) || !stralloc_0(&mcdFile))
			die_nomem();
	} else 
	if (*controldir == '/')
	{
		if (!stralloc_copys(&mcdFile, controldir) || 
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, mcdfile) ||
			!stralloc_0(&mcdFile))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&mcdFile, sysconfdir) || 
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, controldir) ||
			!stralloc_append(&mcdFile, "/") ||
			!stralloc_cats(&mcdFile, mcdfile) ||
			!stralloc_0(&mcdFile))
			die_nomem();
	}
	mcdFile.len--;
	count = 0;
	relayhosts = (DBINFO **) 0;
	if ((fd = open_read(mcdFile.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "LoadDbInfo: open-read: ", mcdFile.s, ": ");
		else 
			return (localDbInfo(total, &relayhosts));
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	/*- 
	 * get count of dbinfo records each
	 * dbinfo record has a 'server line
	 */
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "LoadDbInfo: read: ", mcdFile.s, ": ");
		}
		if (!match && line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!(str_diffn(ptr, "server", 6)))
			count++;
	}
	if (!count) {
		close(fd);
		return (localDbInfo(total, &relayhosts));
	} else
	if (total)
		_total = (*total += count);
	if (!(relayhosts = (DBINFO **) alloc(sizeof(DBINFO *) * (count + 1))))
		die_nomem();
	if (lseek(fd, 0, SEEK_SET) == -1)
		strerr_die3sys(111, "LoadDbInfo: lseek: ", mcdFile.s, ": ");
	ssin.p = 0; /*- reset position to beginning of file */
	ssin.n = sizeof(inbuf);
	for (*dombuf = 0, items = 0, count = 1, rhostsptr = relayhosts;; count++) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "LoadDbInfo: read: ", mcdFile.s, ": ");
		}
		if (!match && line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		match = str_chr(ptr, ' ');
		if (!ptr[match]) {
			strnum1[fmt_uint(strnum1, count)] = 0;
			strerr_warn7("LoadDbInfo: Line No ", strnum1, " in ", mcdFile.s, " has no value[", line.s, "]", 0);
			close(fd);
			alloc_free((char *) relayhosts);
			return ((DBINFO **) 0);
		}
		if (!stralloc_copyb(&dummy1, ptr, match) || !stralloc_0(&dummy1))
			die_nomem();
		for (ptr += match + 1; *ptr && isspace(*ptr); ptr++);
		if (!*ptr) {
			strnum1[fmt_uint(strnum1, count)] = 0;
			strerr_warn7("LoadDbInfo: Line No ", strnum1, " in ", mcdFile.s, " has no value[", line.s, "]", 0);
			close(fd);
			alloc_free((char *) relayhosts);
			return ((DBINFO **) 0);
		}
		if (!stralloc_copys(&dummy2, ptr) || !stralloc_0(&dummy2))
			die_nomem();
		if (!str_diffn(dummy1.s, "domain", 6)) {
			match = str_chr(dummy2.s, ' ');
			str_copyb(dombuf, dummy2.s, match);
			dombuf[match] = 0;
			if (!dummy2.s[match])
				distributed = 0;
			else {
				for (ptr = dummy2.s + match; *ptr && isspace(*ptr); ptr++);
				scan_int(ptr, &distributed);
			}
			continue;
		} else
		if (!str_diffn(dummy1.s, "count", 5))
			continue;
		else
		if (!str_diffn(dummy1.s, "table", 5))
			continue;
		else
		if (!str_diffn(dummy1.s, "time", 4))
			continue;
		if (!str_diffn(dummy1.s, "server", 6)) {
			if (items) {
				strnum1[fmt_uint(strnum1, count)] = 0;
				strerr_warn5("LoadDbInfo: Line Preceding ", strnum1, " in ", mcdFile.s, " is incomplete", 0);
				close(fd);
				alloc_free((char *) relayhosts);
				errno = EINVAL;
				return ((DBINFO **) 0);
			}
			if (!((*rhostsptr) = (DBINFO *) alloc(sizeof(DBINFO))))
				die_nomem();
			items++;
			(*rhostsptr)->socket = (char *) 0;
			(*rhostsptr)->isLocal = 0;
			(*rhostsptr)->fd = -1;
			(*rhostsptr)->port = -1;
			(*rhostsptr)->last_error = 0;
			(*rhostsptr)->last_error_len = 0;
			(*rhostsptr)->failed_attempts = 0;
			str_copyb((*rhostsptr)->server, dummy2.s, dummy2.len);
		} else
		if ((*rhostsptr)) {
			if (!str_diffn(dummy1.s, "mdahost", 7)) {
				items++;
				str_copyb((*rhostsptr)->mdahost, dummy2.s, dummy2.len);
			} else
			if (!str_diffn(dummy1.s, "port", 4)) {
				items++;
				scan_int(dummy2.s, &(*rhostsptr)->port);
			} else
			if (!str_diffn(dummy1.s, "use_ssl", 7)) {
				items++;
				scan_int(dummy2.s, &t);
				(*rhostsptr)->use_ssl = (t ? 1 : 0);
			} else
			if (!str_diffn(dummy1.s, "database", 8)) {
				items++;
				str_copyb((*rhostsptr)->database, dummy2.s, dummy2.len);
			} else
			if (!str_diffn(dummy1.s, "user", 4)) {
				items++;
				str_copyb((*rhostsptr)->user, dummy2.s, dummy2.len);
			} else
			if (!str_diffn(dummy1.s, "pass", 4)) {
				items++;
				str_copyb((*rhostsptr)->password, dummy2.s, dummy2.len);
			} else {
				strnum1[fmt_uint(strnum1, count)] = 0;
				strerr_warn7("LoadDbInfo: Invalid syntax at line ", strnum1, " file ", mcdFile.s, " - [", line.s, "]", 0);
				close(fd);
				alloc_free((char *) relayhosts);
				return ((DBINFO **) 0);
			}
			if (items == 7) {
				if (*dombuf) {
					str_copyb((*rhostsptr)->domain, dombuf, DBINFO_BUFF);
					(*rhostsptr)->distributed = distributed;
				} else {
					str_copyb((*rhostsptr)->domain, "unknown domain", DBINFO_BUFF);
					(*rhostsptr)->distributed = -1;
				}
				rhostsptr++;
				items = 0;
				*dombuf = 0;
			}
		}
	}
	close(fd);
	if (items) {
		strerr_warn2("LoadDbInfo: Incomplete structure in file ", mcdFile.s, 0);
		alloc_free((char *) relayhosts);
		errno = EINVAL;
		return ((DBINFO **) 0);
	}
	(*rhostsptr) = (DBINFO *) 0; /*- Null structure to end relayhosts */
	if (!(rhostsptr = localDbInfo(total, &relayhosts))) {
		strerr_warn1("LoadDbInfo: localDbInfo: No local dbinfo", 0);
	} else
		relayhosts = rhostsptr;
	_total = *total;
	return (relayhosts);
}

static DBINFO **
localDbInfo(int *total, DBINFO ***rhosts)
{
	char           *mysqlhost, *mysql_user = 0, *mysql_passwd = 0; 
	char           *mysql_database = 0, *sysconfdir, *assigndir, *controldir, *ptr, *domain;
	char           *localhost, *mysql_socket = 0, *mysql_port = 0;
	int             t, count, field_count, found, use_ssl = 0, fd, mfd, match;
	static stralloc host_path = {0}, mysqlhost_buf = {0};
	DBINFO        **relayhosts, **rhostsptr, **tmpPtr;
	struct substdio ssin;

	relayhosts = *rhosts;
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	if (!stralloc_copys(&filename, assigndir) || 
		!stralloc_catb(&filename, "/assign", 7) || !stralloc_0(&filename))
		die_nomem();
	if ((fd = open_read(filename.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "LoadDbInfo: open-read: ", filename.s, ": ");
		else
			return ((DBINFO **) 0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	/*- +indimail.org-:indimail.org:508:508:/var/indimail/domains/indimail.org:-:: -*/
	for (count = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "LoadDbInfo: read: ", line.s, ": ");
		}
		if (!match && line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, ':');
		if (!line.s[match])
			continue;
		ptr = line.s + match;
		if (relayhosts) { /*- check for entries for domain in relayhosts */
			ptr++;
			domain = ptr;
			for (; *ptr && *ptr != ':'; ptr++);
			if (*ptr)
				*ptr = 0;
			if (!isvirtualdomain(domain))
				continue;
			if (is_alias_domain(domain))
				continue;
			for (found = 0,tmpPtr = relayhosts;*tmpPtr;tmpPtr++) {
				if (!str_diffn((*tmpPtr)->domain, domain, DBINFO_BUFF)) {
					/*- if relayhosts already has an entry then skip */
					found = 1;
					break;
				}
			}
			if (found)
				continue;
		}
		count++; /*- new domains - domains without entry in relayhosts */
	}
	mysqlhost_buf.len = 0;
	if ((mysqlhost = (char *) env_get("MYSQL_HOST")) != (char *) 0) {
		if (!stralloc_copys(&mysqlhost_buf, mysqlhost) || !stralloc_0(&mysqlhost_buf))
			die_nomem();
	}
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&host_path, controldir) ||
			!stralloc_catb(&host_path, "/host.mysql", 11) ||
			!stralloc_0(&host_path))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&host_path, sysconfdir) ||
			!stralloc_append(&host_path, "/") ||
			!stralloc_cats(&host_path, controldir) ||
			!stralloc_catb(&host_path, "/host.mysql", 11) ||
			!stralloc_0(&host_path))
			die_nomem();
	}
	if (!mysqlhost_buf.len && !access(host_path.s, F_OK)) {
		if ((mfd = open_read(host_path.s)) == -1)
			strerr_die2sys(111, host_path.s, ": ");
		else {
			substdio_fdbuf(&ssin, read, mfd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				t = errno;
				close(mfd);
				errno = t;
				strerr_die3sys(111, "read: ", host_path.s, ": ");
			}
			if (!match && line.len == 0) {
				close(mfd);
				strerr_warn3("LoadDbInfo: ", host_path.s, ": incomplete line", 0);
				_exit (100);
			}
			close(mfd);
			if (match) {
				line.s[line.len - 1] = 0; /*- remove newline */
			}
			if (!stralloc_copy(&mysqlhost_buf, &line)) /*- copy & null terminate */
				die_nomem();
			mysqlhost_buf.len--; /*- exclude null in string length */
		}
	} else
	if (!mysqlhost_buf.len) {
		if (!stralloc_copys(&mysqlhost_buf, MYSQL_HOST) || !stralloc_0(&mysqlhost_buf))
			die_nomem();
	}
	mysqlhost = mysqlhost_buf.s;
	for (field_count = 0,ptr = mysqlhost; *ptr; ptr++) {
		if (*ptr == ':') {
			*ptr = 0;
			switch (field_count++)
			{
			case 0: /*- mysql user */
				if (*(ptr + 1))
					mysql_user = ptr + 1;
				break;
			case 1: /*- mysql passwd */
				if (*(ptr + 1))
					mysql_passwd = ptr + 1;
				break;
			case 2: /*- mysql socket/port */
				if (*(ptr + 1) == '/' || *(ptr + 1) == '.')
					mysql_socket = ptr + 1;
				else
				if (*(ptr + 1))
					mysql_port = ptr + 1;
				break;
			case 3: /*- ssl/nossl */
				use_ssl = (str_diffn(ptr + 1, "ssl", 4) ? 0 : 1);
				break;
			}
		}
	}
	if (!mysql_user)
		getEnvConfigStr(&mysql_user, "MYSQL_USER", MYSQL_USER);
	if (!mysql_passwd)
		getEnvConfigStr(&mysql_passwd, "MYSQL_PASSWD", MYSQL_PASSWD);
	if (!mysql_socket)
		mysql_socket = (char *) env_get("MYSQL_SOCKET");
	if (!mysql_port && !(mysql_port = (char *) env_get("MYSQL_VPORT")))
		mysql_port = "0";
	getEnvConfigStr(&mysql_database, "MYSQL_DATABASE", MYSQL_DATABASE);
	if (!count) { /*- no extra domains found in assign file */
		close(fd);
		if (total && *total) {
			/*- 
			 * remember that total is one less than the actual number of records allocated
			 * in loadMCDInfo(). So for one more record we have to allocate total + 1 + 1
			 * The new allocated becomes total + 1 plus 1 for the last NULL dbinfo structure
			 * The old total was total + 1 and the new total becomes total + 2
			 */
			alloc_re((char *) &relayhosts, sizeof(DBINFO *) * (*total + 1), sizeof(DBINFO *) * (*total + 2));
			rhostsptr = relayhosts + *total;
			for (tmpPtr = rhostsptr;tmpPtr < relayhosts + *total + 2;tmpPtr++)
				*tmpPtr = (DBINFO *) 0;
			(*total) += 1;
		} else {
			relayhosts = (DBINFO **) alloc(sizeof(DBINFO *) * 2);
			rhostsptr = relayhosts;
		}
		if (!((*rhostsptr) = (DBINFO *) alloc(sizeof(DBINFO))))
			die_nomem();
		/*- Should check virtual domains and smtproutes */
		(*rhostsptr)->isLocal = 1; /*- this indicates that this record was created automatically */
		(*rhostsptr)->fd = -1;
		(*rhostsptr)->last_error = 0;
		(*rhostsptr)->last_error_len = 0;
		(*rhostsptr)->failed_attempts = 0;
		if (!(localhost = get_local_ip(AF_INET))) /*- entry in control/localiphost */
			localhost = "localhost";
		str_copyb((*rhostsptr)->mdahost, localhost, DBINFO_BUFF);
		str_copyb((*rhostsptr)->server, mysqlhost, DBINFO_BUFF);
		ptr = vset_default_domain();
		str_copyb((*rhostsptr)->domain, ptr, DBINFO_BUFF);
		scan_int(mysql_port, &(*rhostsptr)->port);
		(*rhostsptr)->socket = mysql_socket;
		(*rhostsptr)->use_ssl = use_ssl;
		str_copyb((*rhostsptr)->database, mysql_database, DBINFO_BUFF);
		str_copyb((*rhostsptr)->user, mysql_user, DBINFO_BUFF);
		str_copyb((*rhostsptr)->password, mysql_passwd, DBINFO_BUFF);
		(*rhostsptr)->distributed = 0;
		rhostsptr++;
		(*rhostsptr) = (DBINFO *) 0;
		return (relayhosts);
	}
	if (*total) { /*- non-empty mcdinfo file */
		/*- +ve count indicates that we found domains in the assign file */
		alloc_re((char *) &relayhosts, sizeof(DBINFO *) * *total, sizeof(DBINFO *) * (*total + count + 1));
		rhostsptr = relayhosts + *total;
		for (tmpPtr = rhostsptr;tmpPtr < relayhosts + *total + count + 1;tmpPtr++)
			*tmpPtr = (DBINFO *) 0;
		_total = (*total) += count;
	} else {
		/*- empty relayhosts */
		relayhosts = (DBINFO **) alloc(sizeof(DBINFO *) * (count + 1));
		for (found = 0, rhostsptr = relayhosts; found < count + 1;found++, rhostsptr++)
			(*rhostsptr) = (DBINFO *) 0;
		rhostsptr = relayhosts;
	}
	if (!relayhosts)
		die_nomem();
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (lseek(fd, 0, SEEK_SET) == -1)
		strerr_die3sys(111, "LoadDbInfo: lseek: ", filename.s, ": ");
	ssin.p = 0; /*- reset position to beginning of file */
	ssin.n = sizeof(inbuf);
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			t = errno;
			close(fd);
			errno = t;
			strerr_die3sys(111, "LoadDbInfo: read: ", filename.s, ": ");
		}
		if (!match && line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- null terminate */
		}
		match = str_chr(line.s, ':');
		if (!line.s[match])
			continue;
		ptr = line.s + match + 1;
		domain = ptr;
		for (;*ptr && *ptr != ':';ptr++);
		if (*ptr)
			*ptr = 0;
		if (!isvirtualdomain(domain))
			continue;
		if (is_alias_domain(domain))
			continue;
		for (found = 0,tmpPtr = relayhosts;*tmpPtr;tmpPtr++) {
			if (!str_diffn((*tmpPtr)->domain, domain, DBINFO_BUFF)) {
				/*- if relayhosts already has an entry then skip */
				found = 1;
				break;
			}
		}
		if (found)
			continue;
		if (!((*rhostsptr) = (DBINFO *) alloc(sizeof(DBINFO))))
			die_nomem();
		if (total)
			(*total)++;
		/*- Should check virtual domains and smtproutes */
		(*rhostsptr)->isLocal = 1; /*- indicate that we were created automatically */
		(*rhostsptr)->fd = -1;
		(*rhostsptr)->last_error = 0;
		(*rhostsptr)->last_error_len = 0;
		(*rhostsptr)->failed_attempts = 0;
		if (!(localhost = get_local_ip(AF_INET))) /*- entry in control/localiphost */
			localhost = "localhost";
		str_copyb((*rhostsptr)->mdahost, localhost, DBINFO_BUFF);
		str_copyb((*rhostsptr)->server, mysqlhost, DBINFO_BUFF);
		str_copyb((*rhostsptr)->domain, domain, DBINFO_BUFF);
		scan_int(mysql_port, &(*rhostsptr)->port);
		(*rhostsptr)->socket = mysql_socket;
		(*rhostsptr)->use_ssl = use_ssl;
		str_copyb((*rhostsptr)->database, mysql_database, DBINFO_BUFF);
		str_copyb((*rhostsptr)->user, mysql_user, DBINFO_BUFF);
		str_copyb((*rhostsptr)->password, mysql_passwd, DBINFO_BUFF);
		(*rhostsptr)->distributed = 0;
		rhostsptr++;
		(*rhostsptr) = (DBINFO *) 0;
	}
	close(fd);
	(*rhostsptr) = (DBINFO *) 0; /*- Null structure to end relayhosts */
	return (relayhosts);
}
