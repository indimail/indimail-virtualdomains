/*
 * $Log: authpgsql.c,v $
 * Revision 1.8  2021-09-12 20:17:33+05:30  Cprogrammer
 * moved replacestr to libqmail
 *
 * Revision 1.7  2021-07-22 15:16:45+05:30  Cprogrammer
 * conditional define of _XOPEN_SOURCE
 *
 * Revision 1.6  2020-09-28 13:28:08+05:30  Cprogrammer
 * added pid in debug statements
 *
 * Revision 1.5  2020-09-28 12:48:55+05:30  Cprogrammer
 * print authmodule name in error logs/debug statements
 *
 * Revision 1.4  2020-04-01 18:53:05+05:30  Cprogrammer
 * moved getEnvConfig to libqmail
 *
 * Revision 1.3  2019-07-10 12:57:26+05:30  Cprogrammer
 * print more error information in print_error
 *
 * Revision 1.2  2019-04-22 22:24:30+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-14 20:55:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef ENABLE_DOMAIN_LIMITS
#include <time.h>
#include "vlimits.h"
#endif
#ifdef HAVE_UNISTD_H
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <error.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <pw_comp.h>
#include <getEnvConfig.h>
#include <replacestr.h>
#endif
#include "inquery.h"
#include "pipe_exec.h"
#include "variables.h"
#include "common.h"
#include "lowerit.h"
#include "parse_email.h"
#include "runcmmd.h"

#ifndef lint
static char     sccsid[] = "$Id: authpgsql.c,v 1.8 2021-09-12 20:17:33+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_PGSQL
#include <libpq-fe.h>			/* required pgsql front-end headers */

#ifdef AUTH_SIZE
#undef AUTH_SIZE
#define AUTH_SIZE 512
#else
#define AUTH_SIZE 512
#endif

static int      authlen = AUTH_SIZE;
static char     strnum[FMT_ULONG], module_pid[FMT_ULONG];
static stralloc tmp = {0};
PGconn         *pgc; /* pointer to pgsql connection */

static void
die_nomem()
{
	strerr_warn1("authpgsql: out of memory", 0);
	_exit(111);
}

struct passwd  *
pg_getpw(char *user, char *domain)
{
	static struct passwd   pwent;
	static stralloc IUser = {0}, IPass = {0}, IGecos = {0},
					IDir = {0}, IShell = {0}, SqlBuf = {0};
	char           *table_name, *select_stmt;
	int             i;
	PGresult       *pgres;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif

	lowerit(user);
	lowerit(domain);
	if (!(table_name = (char *) env_get("PG_TABLE_NAME")))
		table_name = "indimail";
	if (!(select_stmt = (char *) env_get("SELECT_STATEMENT"))) {
		if (!stralloc_copyb(&SqlBuf,
				"select high_priority pw_name, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell from ", 89) ||
				!stralloc_cats(&SqlBuf, table_name) ||
				!stralloc_catb(&SqlBuf, " where pw_name=\"", 16) ||
				!stralloc_cats(&SqlBuf, user) ||
				!stralloc_catb(&SqlBuf, "\" and pw_domain=\"", 17) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
	} else {
		tmp.len = 0;
		if ((i =  replacestr(select_stmt, "%u", user, &tmp)) == -1)
			die_nomem();
		else
		if (i)
			select_stmt = tmp.s;
		SqlBuf.len = 0;
		if ((i =  replacestr(select_stmt, "%d", domain, &SqlBuf)) == -1)
			die_nomem();
		else
		if (i)
			select_stmt = SqlBuf.s;
	}
	pgres = PQexec(pgc, select_stmt);
	if (!pgres || PQresultStatus(pgres) != PGRES_TUPLES_OK) {
		if (pgres)
			PQclear(pgres);
		return NULL;
	}
	if (PQntuples(pgres) <= 0) {	/* rows count */
		PQclear(pgres);
		return NULL;
	}
	if (!stralloc_copys(&IUser, PQgetvalue(pgres, 0, 0)) ||
			!stralloc_0(&IUser))
		die_nomem();
	pwent.pw_name = IUser.s;
	if (!stralloc_copys(&IPass, PQgetvalue(pgres, 0, 1)) ||
			!stralloc_0(&IPass))
		die_nomem();
	pwent.pw_passwd = IPass.s;
	scan_uint(PQgetvalue(pgres, 0, 2), (unsigned int *) &pwent.pw_uid);
	scan_uint(PQgetvalue(pgres, 0, 3), (unsigned int *) &pwent.pw_gid);
	if (!stralloc_copys(&IGecos, PQgetvalue(pgres, 0, 4)) ||
			!stralloc_0(&IGecos))
		die_nomem();
	pwent.pw_gecos = IGecos.s;
	if (!stralloc_copys(&IDir, PQgetvalue(pgres, 0, 5)) ||
			!stralloc_0(&IDir))
		die_nomem();
	pwent.pw_dir = IDir.s;
	if (!stralloc_copys(&IShell, PQgetvalue(pgres, 0, 6)) ||
			!stralloc_0(&IShell))
		die_nomem();
	pwent.pw_shell = IShell.s;
#ifdef ENABLE_DOMAIN_LIMITS
	if (env_get("DOMAIN_LIMITS") && !(pwent.pw_gid & V_OVERRIDE)) {
		if (!vget_limits(domain, &limits))
			pwent.pw_gid |= vlimits_get_flag_mask(&limits);
		else
			return ((struct passwd *) 0);
	}
#endif
	return (&pwent);
}

void
print_error(char *str)
{
	out("authpgsql", "454-");
	out("authpgsql", str);
	out("authpgsql", ": ");
	out("authpgsql", error_str(errno));
	out("authpgsql", " (#4.3.0)\r\n");
	flush("authpgsql");
}

int
main(int argc, char **argv)
{
	char           *authstr, *login, *ologin, *response, *challenge, *crypt_pass, *ptr, *cptr;
	static stralloc user = {0}, fquser = {0}, domain = {0}, buf = {0};
	int             i, count, offset, norelay = 0, status, auth_method;
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	time_t          curtime;
	struct vlimits  limits;
#endif

	if (argc < 2)
		_exit(2);
	if (!(authstr = calloc(1, (authlen + 1) * sizeof (char)))) {
		print_error("out of memory");
		strnum[fmt_uint(strnum, (unsigned int) authlen + 1)] = 0;
		strerr_warn3("alloc-", strnum, ": ", &strerr_sys);
		_exit(111);
	}
	for (offset = 0;;) {
		do {
			count = read(3, authstr + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while (count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while (count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			print_error("read error");
			strerr_warn1("authpgsql: read: ", &strerr_sys);
			_exit(111);
		} else if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
			_exit(2);
	}
	count = 0;
	login = authstr + count;	/*- username */
	for (; authstr[count] && count < offset; count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);

	count++;
	challenge = authstr + count;	/*- challenge (or plain text) */
	for (; authstr[count] && count < offset; count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);

	count++;
	response = authstr + count; /*- response (cram-md5, cram-sha1, etc) */
	for (; authstr[count] && count < offset; count++);
	if (count == offset || (count + 1) == offset)
		auth_method = 0;
	else
		auth_method = authstr[count + 1];

	ologin = login;
	for (ptr = login; *ptr && *ptr != '@'; ptr++);
	if (!*ptr) { /*- no @ in the login */
		if (auth_method == AUTH_DIGEST_MD5) { /*- for handling dumb programs like
												outlook written by dumb programmers */
			if ((ptr = str_str(login, "realm="))) {
				ptr += 6;
				for (i = 0, cptr = ptr; *ptr && *ptr != ','; ptr++) {
					if (isspace(*ptr) || *ptr == '\"')
						continue;
					i++;
				}
				if (!stralloc_ready(&domain, i + 1))
					die_nomem();
				for (i = 0, ptr = cptr; *ptr && *ptr != ','; ptr++) {
					if (isspace(*ptr) || *ptr == '\"')
						continue;
					domain.s[i++] = *ptr;
				}
				domain.len = i;
				if (!stralloc_0(&domain))
					die_nomem();
				domain.len--;
				for (i = 0, ptr = login; *ptr && *ptr != '@'; i++, ptr++);
				if (!stralloc_copyb(&fquser, login, i) ||
						!stralloc_append(&fquser, "@") ||
						!stralloc_cat(&fquser, &domain) ||
						!stralloc_0(&fquser))
					die_nomem();
				login = fquser.s;
			}
		}
	}
	parse_email(login, &user, &domain);
	pgc = PQconnectdb("user=postgress,database=indimail");
	if (PQstatus(pgc) == CONNECTION_BAD) {
		out("pgsql", "454-failed to connect to database (");
		out("pgsql", PQerrorMessage(pgc));
		out("pgsql", ") (#4.3.0)\r\n");
		flush("pgsql");
		_exit(111);
	}
	pw = pg_getpw(user.s, domain.s);
    PQfinish(pgc);
	if (!pw) {
		if (userNotFound)
			pipe_exec(argv, authstr, offset);
		else
			strerr_warn1("authpgsql: pg_getpw: ", &strerr_sys);
		print_error("pg_gepw");
		_exit(111);
	} else
	if (pw->pw_gid & NO_SMTP) {
		out("authpgsql", "553-Sorry, this account cannot use SMTP (#5.7.1)\r\n");
		flush("authpgsql");
		_exit(1);
	} else
	if (is_inactive && !env_get("ALLOW_INACTIVE")) {
		out("authpgsql", "553-Sorry, this account is inactive (#5.7.1)\r\n");
		flush("authpgsql");
		_exit(1);
	} else
	if (pw->pw_gid & NO_RELAY)
		norelay = 1;
	crypt_pass = pw->pw_passwd;
	strnum[fmt_uint(strnum, (unsigned int) auth_method)] = 0;
	module_pid[fmt_ulong(module_pid, getpid())] = 0;
	if (env_get("DEBUG_LOGIN"))
		strerr_warn13("authpgsql: pid [", module_pid, "] login [", login, "] challenge [", challenge,
			"] response [", response, "] pw_passwd [", crypt_pass, "] method [", strnum, "]", 0);
	else
	if (env_get("DEBUG"))
		strerr_warn7("authpgsql: pid [", module_pid, "] login [", login, "] method [", strnum, "]", 0);
	if (pw_comp((unsigned char *) ologin, (unsigned char *) crypt_pass, (unsigned char *) (*response ? challenge : 0),
		 (unsigned char *) (*response ? response : challenge), auth_method)) {
		pipe_exec(argv, authstr, offset);
		print_error("exec");
		_exit(111);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (env_get("DOMAIN_LIMITS")) {
		struct vlimits *lmt;
#ifdef QUERY_CACHE
		if (!env_get("QUERY_CACHE")) {
			if (vget_limits(domain.s, &limits)) {
				strerr_warn2("authpgsql: unable to get domain limits for for ", domain.s, 0);
				out("authpgsql", "454-unable to get domain limits for ");
				out("authpgsql", domain.s);
				out("authpgsql", "\r\n");
				flush("authpgsql");
				_exit(111);
			}
			lmt = &limits;
		} else
			lmt = inquery(LIMIT_QUERY, login, 0);
#else
		if (vget_limits(domain.s, &limits)) {
			strerr_warn2("authpgsql: unable to get domain limits for for ", domain.s, 0);
			out("authpgsql", "454-unable to get domain limits for ");
			out("authpgsql", domain.s);
			out("authpgsql", "\r\n");
			flush("authpgsql");
			_exit(111);
		}
		lmt = &limits;
#endif
		curtime = time(0);
		if (lmt->domain_expiry > -1 && curtime > lmt->domain_expiry) {
			out("authpgsql", "553-Sorry, your domain has expired (#5.7.1)\r\n");
			flush("authpgsql");
			_exit(1);
		} else if (lmt->passwd_expiry > -1 && curtime > lmt->passwd_expiry) {
			out("authpgsql", "553-Sorry, your password has expired (#5.7.1)\r\n");
			flush("authpgsql");
			_exit(1);
		}
	}
#endif
	status = 0;
	if ((ptr = (char *) env_get("POSTAUTH")) && !access(ptr, X_OK)) {
		if (!stralloc_copys(&buf, ptr) ||
				!stralloc_append(&buf, " ") ||
				!stralloc_cats(&buf, login) ||
				!stralloc_0(&buf))
			die_nomem();
		status = runcmmd(buf.s, 0);
	}
	_exit(norelay ? 3 : status);
	/*- Not reached */
	return (0);
}

#else
#include <unistd.h>
#include <strerr.h>
#include "common.h"
#warning "not compiled with -DHAVE_PGSQL"

void
print_error(char *str)
{
	out("authpgsql", "454-");
	out("authpgsql", str);
	out("authpgsql", ": ");
	out("authpgsql", error_str(errno));
	out("authpgsql", " (#4.3.0)\r\n");
	flush("authpgsql");
}

int
main(int argc, char **argv)
{
	execvp(argv[1], argv + 1);
	print_error("exec");
	strerr_warn3("authpgsql: execvp: ", argv[1], ": ", &strerr_sys);
}
#endif
