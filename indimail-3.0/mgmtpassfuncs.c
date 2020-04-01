/*
 * $Log: mgmtpassfuncs.c,v $
 * Revision 1.3  2020-04-01 18:57:07+05:30  Cprogrammer
 * added encrypt flag to mkpasswd()
 *
 * Revision 1.2  2019-04-22 23:14:00+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.1  2019-04-15 11:34:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef lint
static char     sccsid[] = "$Id: mgmtpassfuncs.c,v 1.3 2020-04-01 18:57:07+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <scan.h>
#include <alloc.h>
#include <str.h>
#include <mkpasswd.h>
#include <makesalt.h>
#include <in_crypt.h>
#include <pw_comp.h>
#endif
#include "mgmtpassfuncs.h"
#include "passwd_policy.h"
#include "findhost.h"
#include "common.h"
#include "create_table.h"
#include "open_master.h"
#include "variables.h"
#include "indimail.h"

#define SETPAS_MAX_ATTEMPTS 6
#define LOGIN_MAX_ATTEMPTS 3
#define DAILY_MAX_ATTEMPTS 6
#define PASSDICT SYSCONFDIR"/pass.dict"

static stralloc tmpbuf = {0}, SqlBuf = {0};

static void
die_nomem()
{
	strerr_warn1("mgmtpass: out of memory", 0);
	_exit(111);
}

int
getpassword(char *user)
{
	char           *pwdptr, *passwd;
	int             count;

	if (isDisabled(user))
		pwdptr = (char *) 0;
	else
		pwdptr = mgmtgetpass(user, 0);
	for (count = 0; count < LOGIN_MAX_ATTEMPTS; count++) {
		if (!stralloc_copyb(&tmpbuf, "(current ", 9) ||
				!stralloc_cats(&tmpbuf, user) ||
				!stralloc_catb(&tmpbuf, ") Password: ", 12) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		passwd = (char *) getpass(tmpbuf.s);
		if (!pwdptr) {
			strerr_warn1("Login incorrect or you are disabled", 0);
			continue;
		}
		if (passwd && *passwd && *pwdptr) {
			if (!pw_comp(0, (unsigned char *) pwdptr, 0, (unsigned char *) passwd, 0))
				return (0);
		}
		updateLoginFailed(user);
		if (isDisabled(user))
			pwdptr = (char *) 0;
		strerr_warn1("Login incorrect or you are disabled", 0);
	}
	if (pwdptr && *pwdptr)
		(void) ChangeLoginStatus(user, 1);
	strerr_warn2(user, " you are disabled", 0);
	return (1);
}

int
updateLoginFailed(char *user)
{
	char            strnum[FMT_ULONG];
	int             err;
	time_t          tmval;
	struct tm      *tmptr;

	if (open_central_db(0)) {
		strerr_warn1("mgmtpass: Unable to open central db", 0);
		return (1);
	}
	tmval = time(0);
	tmptr = localtime(&tmval);
	if (!stralloc_copyb(&SqlBuf, "update low_priority mgmtaccess set day=", 39) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) tmptr->tm_mday)) ||
			!stralloc_catb(&SqlBuf, ", attempts=attempts + 1 where user=\"", 36) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
		return (1);
	}
	if (!(err = in_mysql_affected_rows(&mysql[0])) || err == -1)
		return (1);
	return (0);
}

int
ChangeLoginStatus(char *user, int status)
{
	char            strnum[FMT_ULONG];
	int             err;

	if (open_central_db(0)) {
		strerr_warn1("mgmtpass: Unable to open central db", 0);
		return (1);
	}
	if (!stralloc_copyb(&SqlBuf, "update low_priority mgmtaccess set status=", 42) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_int(strnum, status)) ||
			!stralloc_catb(&SqlBuf, " where user=\"", 13) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
		return (1);
	}
	if (!(err = in_mysql_affected_rows(&mysql[0])) || err == -1)
		return (1);
	return (0);
}

int
mgmtlist()
{
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0)) {
		strerr_warn1("mgmtpass: Unable to open central db", 0);
		return (1);
	}
	if (!stralloc_copyb(&SqlBuf, "select high_priority user from mgmtaccess", 41) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
			return (0);
		} else
			strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	} 
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("mgmtpass: MySQL Store Result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	} else
	if (!in_mysql_num_rows(res)) {
		in_mysql_free_result(res);
		return (0);
	} 
	for(;;) {
		if (!(row = in_mysql_fetch_row(res)))
			break;
		out("mgmtlist", row[0]);
	}
	flush("mgmtlist");
	in_mysql_free_result(res);
	return (0);
}

int
isDisabled(char *user)
{
	int             status, t1, t2;
	time_t          tmval;
	struct tm      *tmptr;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0)) {
		strerr_warn1("mgmtpass: Unable to open central db", 0);
		return (1);
	}
	tmval = time(0);
	tmptr = localtime(&tmval);
	if (!stralloc_copyb(&SqlBuf, "select high_priority day,attempts,status from mgmtaccess where user=\"", 69) ||
			!stralloc_cats(&SqlBuf, user) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
		else
			strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	} else
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("mgmtpass: MySQL Store Result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (1);
	} else
	if (!in_mysql_num_rows(res)) {
		in_mysql_free_result(res);
		return (1);
	} else
	if ((row = in_mysql_fetch_row(res))) {
		scan_int(row[0], &t1);
		scan_int(row[1], &t2);
		if ((tmptr->tm_mday == t1) && (t2 > DAILY_MAX_ATTEMPTS))
			status = 1;
		else
			scan_int(row[2], &status);
	} else
		status = 1;
	in_mysql_free_result(res);
	return (status);
}

int
mgmtpassinfo(char *username, int print_flag)
{
	time_t          tmval;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (!username || !*username)
		return (1);
	if (open_central_db(0)) {
		strerr_warn1("mgmtpass: Unable to open central db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "select high_priority pass, pw_uid, pw_gid, lastaccess, lastupdate, ", 67) ||
			!stralloc_catb(&SqlBuf, "attempts, status from mgmtaccess where user=\"", 45) ||
			!stralloc_cats(&SqlBuf, username) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
			return (1);
		} else
			strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	} else
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("mgmtpass: MySQL Store Result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return (-1);
	} else
	if (!(in_mysql_num_rows(res))) {
		if (print_flag)
			strerr_warn2(username, ": No such user", 0);
		userNotFound = 1;
		in_mysql_free_result(res);
		return (1);
	}
	if (!print_flag) {
		in_mysql_free_result(res);
		return (0);
	}
	if ((row = in_mysql_fetch_row(res))) {
		out("mgmtpass", "User        : ");
		out("mgmtpass", username);
		out("mgmtpass", "\n");
		out("mgmtpass", "Pass        : ");
		out("mgmtpass", row[0]);
		out("mgmtpass", "\n");
		out("mgmtpass", "Uid         : ");
		out("mgmtpass", row[1]);
		out("mgmtpass", "\n");
		out("mgmtpass", "Gid         : ");
		out("mgmtpass", row[2]);
		out("mgmtpass", "\n");
		scan_ulong(row[3], (unsigned long *) &tmval);
		out("mgmtpass", "Last Access : ");
		out("mgmtpass", ctime(&tmval));
		scan_ulong(row[4], (unsigned long *) &tmval);
		out("mgmtpass", "Last Update : ");
		out("mgmtpass", ctime(&tmval));
		out("mgmtpass", "Attempts    : ");
		out("mgmtpass", row[5]);
		out("mgmtpass", "\n");
		out("mgmtpass", "Status      : ");
		out("mgmtpass", row[6]);
		out("mgmtpass", " (");
		out("mgmtpass", isDisabled(username) ? "Disabled" : "Enabled");
		out("mgmtpass", ")\n");
		flush("mgmtpass");
		in_mysql_free_result(res);
		return (0);
	}
	in_mysql_free_result(res);
	return (1);
}

int
setpassword(char *user)
{
	char            salt[SALTSIZE + 1];
	char           *newpass1, *newpass2, *pwdptr, *passwd, *crypt_pass;
	int             i1, i2, plen;
	time_t          lastupdate;

	pwdptr = mgmtgetpass(user, 0);
	for (i1 = 0; i1 < LOGIN_MAX_ATTEMPTS; i1++) {
		passwd = (char *) getpass("Old password: ");
		if (!pwdptr) {
			strerr_warn1("Login incorrect or you are disabled", 0);
			continue;
		}
		if (passwd && *passwd && *pwdptr) {
			if (!pw_comp(0, (unsigned char *) pwdptr, 0, (unsigned char *) passwd, 0))
				break;
		}
		(void) updateLoginFailed(user);
		if (isDisabled(user))
			pwdptr = (char *) 0;
		strerr_warn1("Login incorrect or you are disabled", 0);
	}
	if (!pwdptr) /* user does not exist */
		return (1);
	if (i1 == LOGIN_MAX_ATTEMPTS) {
		(void) ChangeLoginStatus(user, 1);
		return (1);
	}
	newpass1 = newpass2 = (char *) 0;
	for (i1 = 0; i1 < SETPAS_MAX_ATTEMPTS; i1++) {
		makesalt(salt, SALTSIZE);
		for (i2 = 1;;i2++) {
			if (i2 > SETPAS_MAX_ATTEMPTS)
			{
				strerr_warn1("passwd: Too many tries; try again later.", 0);
				if (newpass1)
					alloc_free(newpass1);
				if (newpass2)
					alloc_free(newpass2);
				return (1);
			}
			passwd = (char *) getpass("New password: ");
			if (passwd_policy(passwd))
				continue;
			if (!pw_comp(0, (unsigned char *) pwdptr, 0, (unsigned char *) passwd, 0)) {
				strerr_warn1("Your passwd cannot be same as the previous one", 0);
				continue;
			}
			break;
		}
		if (!(crypt_pass = (char *) in_crypt(passwd, salt))) {
			strerr_warn1("Error with in_crypt() module: ", &strerr_sys);
			continue;
		}
		plen = str_len(crypt_pass);
		if (!newpass1 && !(newpass1 = (char *) alloc(sizeof(char) * (plen + 1))))
			die_nomem();
		str_copy(newpass1, crypt_pass);

		passwd = (char *) getpass("Re-enter new password: ");
		if (!(crypt_pass = (char *) in_crypt(passwd, salt))) {
			strerr_warn1("Error with in_crypt() module: ", &strerr_sys);
			continue;
		}
		plen = str_len(crypt_pass);
		if (!newpass2 && !(newpass2 = (char *) alloc(sizeof(char) * (plen + 1))))
			die_nomem();
		str_copy(newpass2, crypt_pass);
		if (!str_diffn(newpass1, newpass2, plen + 1)) {
			if (newpass2)
				alloc_free(newpass2);
			break;
		}
		if (i1 < (SETPAS_MAX_ATTEMPTS - 1))
			strerr_warn1("They don't match; try again.", 0);
		else {
			strerr_warn1("passwd: Too many tries; try again later.", 0);
			if (newpass1)
				alloc_free(newpass1);
			if (newpass2)
				alloc_free(newpass2);
			return (1);
		}
	}
	lastupdate = (time_t) time(0);
	encrypt_flag = 1;
	mgmtsetpass(user, newpass1, getuid(), getgid(), lastupdate, lastupdate);
	if (newpass1)
		alloc_free(newpass1);
	return (0);
}

char           *
mgmtgetpass(char *username, int *status)
{
	static stralloc _user = {0}, mysql_pass = {0};
	char            strnum[FMT_ULONG];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (!username || !*username) {
		strerr_warn1("Password incorrect", 0);
		return ((char *) 0);
	}
	if (_user.len && username && *username && !str_diffn(username, _user.s, _user.len))
		return (mysql_pass.s);
	if (open_central_db(0)) {
		strerr_warn1("mgmtpass: Unable to open central db", 0);
		return ((char *) 0);
	}
	if (!stralloc_copyb(&SqlBuf, "select high_priority pass,status from mgmtaccess where user=\"", 61) ||
			!stralloc_cats(&SqlBuf, username) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
			strerr_warn1("Password incorrect", 0); /*- password is incorrect of no user/table exists */
			return ((char *) 0);
		} else
			strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	} else
	if (!(res = in_mysql_store_result(&mysql[0]))) {
		strerr_warn2("mgmtpass: MySQL Store Result: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	} else
	if (!(in_mysql_num_rows(res))) {
		in_mysql_free_result(res);
		strerr_warn1("Password incorrect", 0);
		return ((char *) 0);
	}
	if ((row = in_mysql_fetch_row(res))) {
		if (!stralloc_copys(&mysql_pass, row[0]) || !stralloc_0(&mysql_pass))
			die_nomem();
		mysql_pass.len--;
		if (status)
			scan_int(row[1], status);
	}
	in_mysql_free_result(res);
	if (!stralloc_copyb(&SqlBuf, "update low_priority mgmtaccess set lastaccess=", 46) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_ulong(strnum, (unsigned long) time(0))) ||
			!stralloc_catb(&SqlBuf, " where user=\"", 13) ||
			!stralloc_cats(&SqlBuf, username) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
		return ((char *) 0);
	}
	if (!stralloc_copys(&_user, username) || !stralloc_0(&_user))
		die_nomem();
	_user.len--;
	return (mysql_pass.s);
}

int
mgmtsetpass(char *username, char *pass, uid_t uid, gid_t gid, time_t lastaccess, time_t lastupdate)
{
	static stralloc crypted = {0};
	char            strnum[FMT_ULONG];
	int             i;
	time_t          cur_time;
	struct tm      *tmptr;
	int             err;

	if (open_master()) {
		strerr_warn1("mgmtsetpass: failed to open master db", 0);
		return (-1);
	}
	if (encrypt_flag) {
		if (!stralloc_copys(&crypted, pass) || !stralloc_0(&crypted))
			die_nomem();
		crypted.len--;
	} else
		mkpasswd(pass, &crypted, encrypt_flag);
	cur_time = time(0);
	tmptr = localtime(&cur_time);
	if (!stralloc_copyb(&SqlBuf, "update low_priority mgmtaccess set pass=\"", 41) ||
			!stralloc_cat(&SqlBuf, &crypted) ||
			!stralloc_catb(&SqlBuf, "\", pw_uid=", 10) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) uid)) ||
			!stralloc_catb(&SqlBuf, ", pw_gid=", 9) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) gid)) ||
			!stralloc_catb(&SqlBuf, ", lastaccess=", 13) ||
			!stralloc_catb(&SqlBuf, strnum, (i = fmt_uint(strnum, (unsigned int) cur_time))) ||
			!stralloc_catb(&SqlBuf, ", lastupdate=", 13) ||
			!stralloc_catb(&SqlBuf, strnum, i) ||
			!stralloc_catb(&SqlBuf, ", day=", 6) ||
			!stralloc_catb(&SqlBuf, strnum, fmt_uint(strnum, (unsigned int) tmptr->tm_mday)) ||
			!stralloc_catb(&SqlBuf, ", attempts=0, status=0 where user=\"", 35) ||
			!stralloc_cats(&SqlBuf, username) ||
			!stralloc_append(&SqlBuf, "\"") ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[0], SqlBuf.s)) {
				strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
		}  else {
			strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
			return (-1);
		}
	}
	if (!(err = in_mysql_affected_rows(&mysql[0])) || err == -1)
		return (1);
	return (0);
}

int
mgmtadduser(char *username, char *pass, uid_t uid, gid_t gid, time_t lastaccess, time_t lastupdate)
{
	if (open_master()) {
		strerr_warn1("mgmtsetpass: failed to open master db", 0);
		return (-1);
	}
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into mgmtaccess (user, pass) values (\"", 58) ||
			!stralloc_cats(&SqlBuf, username) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, pass) ||
			!stralloc_catb(&SqlBuf, "\")", 2) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[0], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[0], SqlBuf.s)) {
				strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
				return (-1);
			}
		}  else {
			strerr_warn4("mgmtpass: mysql_query[", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[0]), 0);
			return (-1);
		}
	}
	return (mgmtsetpass(username, pass, uid, gid, lastaccess, lastupdate));
}
#endif
