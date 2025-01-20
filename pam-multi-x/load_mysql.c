/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef mysql_query
#undef mysql_query
#endif
#include <mysql.h>
#include <mysqld_error.h>

#ifndef	lint
static char     sccsid[] = "$Id: load_mysql.c,v 1.13 2022-11-23 15:49:46+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef DLOPEN_LIBMYSQLCLIENT
#include <unistd.h>
#include <dlfcn.h>
#ifdef HAVE_QMAIL
#include <error.h>
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#endif
#ifndef HAVE_BOOL
typedef char bool;
#endif

MYSQL          *(*in_mysql_init) (MYSQL *);
MYSQL          *(*in_mysql_real_connect) (MYSQL *, const char *, const char *, const char *, const char *, unsigned int, const char *, unsigned long);
const char     *(*in_mysql_error) (MYSQL *);
unsigned int    (*in_mysql_errno) (MYSQL *);
int             (*in_mysql_next_result) (MYSQL *);
void            (*in_mysql_close) (MYSQL *);
int             (*in_mysql_options) (MYSQL *, enum mysql_option, const void *);
#if MYSQL_VERSION_ID >= 50703 && !defined(MARIADB_BASE_VERSION)
int             (*in_mysql_get_option) (MYSQL *, enum mysql_option, void *);
#endif
int             (*in_mysql_query) (MYSQL *, const char *);
MYSQL_RES      *(*in_mysql_store_result) (MYSQL *);
MYSQL_ROW       (*in_mysql_fetch_row) (MYSQL_RES *);
unsigned long  *(*in_mysql_fetch_lengths) (MYSQL_RES *);
my_ulonglong    (*in_mysql_num_rows) (MYSQL_RES *);
unsigned int    (*in_mysql_num_fields)(MYSQL_RES *);
my_ulonglong    (*in_mysql_affected_rows) (MYSQL *);
void            (*in_mysql_free_result) (MYSQL_RES *);
const char     *(*in_mysql_stat) (MYSQL *);
int             (*in_mysql_ping) (MYSQL *);
unsigned long   (*in_mysql_real_escape_string) (MYSQL *, char *, const char *, unsigned long);
unsigned int    (*in_mysql_get_proto_info) (MYSQL *);
const char     *(*in_mysql_get_host_info) (MYSQL *);
int             (*in_mysql_select_db) (MYSQL *, const char *);
bool            (*in_mysql_ssl_set) (MYSQL *, const char *, const char *, const char *, const char *, const char *);
const char     *(*in_mysql_get_ssl_cipher) (MYSQL *);
void            (*in_mysql_data_seek) (MYSQL_RES *, my_ulonglong);
const char     *(*in_mysql_get_server_info) (MYSQL *);
const char     *(*in_mysql_get_client_info) (void);

static char     memerr[] = "out of memory";
static char     ctlerr[] = "unable to read controls";
static char    *controldir;
static stralloc errbuf = { 0 };
static stralloc libfn = { 0 };
static char     inbuf[2048];

static void
striptrailingwhitespace(stralloc *sa)
{
	while (sa->len > 0) {
		switch (sa->s[sa->len - 1])
		{
		case '\n':
		case ' ':
		case '\t':
			--sa->len;
			break;
		default:
			return;
		}
	}
}

static int
control_readline(stralloc *sa, char *fn)
{
	substdio        ss;
	int             fd, match;
	static stralloc controlfile = {0};

	if (*fn != '/' && *fn != '.') {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = CONTROLDIR;
		}
		if (!stralloc_copys(&controlfile, controldir))
			return(-1);
		if (controlfile.s[controlfile.len - 1] != '/' && !stralloc_cats(&controlfile, "/"))
			return(-1);
		if (!stralloc_cats(&controlfile, fn))
			return(-1);
	} else
	if (!stralloc_copys(&controlfile, fn))
		return(-1);
	if (!stralloc_0(&controlfile))
		return(-1);
	if ((fd = open_read(controlfile.s)) == -1) {
		if (errno == error_noent)
			return 0;
		return -1;
	}
	substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	if (getln(&ss, sa, &match, '\n') == -1) {
		close(fd);
		return -1;
	}
	striptrailingwhitespace(sa);
	close(fd);
	sa->s[sa->len] = 0;
	return 1;
}

static void *
loadLibrary(void **handle, char *libenv, int *errflag, char **errstr)
{
	char           *ptr;
	int             i;

	if (*libenv == '/') { /*- filename */
		if ((i = control_readline(&libfn, libenv)) == -1 || !i) {
			if (errflag)
				*errflag = errno;
			if (errstr)
				*errstr = (char *) 0;
			if (!stralloc_copys(&errbuf, ctlerr) ||
					!stralloc_catb(&errbuf, ": ", 2) ||
					!stralloc_copys(&errbuf, error_str(errno)) ||
					!stralloc_0(&errbuf)) {
				if (errstr)
					*errstr = memerr;
			} else
			if (errstr)
				*errstr = errbuf.s;
			return ((void *) 0);
		}
		if (!stralloc_0(&libfn)) {
			if (errstr)
				*errstr = memerr;
			return ((void *) 0);
		}
		ptr = libfn.s;
	} else
	if (!(ptr = env_get(libenv))) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		return ((void *) 0);
	} else
	if (handle && *handle) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		return (handle);
	}
	if (errflag)
		*errflag = -1;
	if (errstr)
		*errstr = (char *) 0;
	if (*libenv != '/' && access(ptr, R_OK)) {
		if (errflag)
			*errflag = errno;
		if (!stralloc_copys(&errbuf, error_str(errno))) {
			if (errstr)
				*errstr = memerr;
		} else
		if (!stralloc_0(&errbuf)) {
			if (errstr)
				*errstr = memerr;
		} else
		if (errstr)
			*errstr = errbuf.s;
		return ((void *) 0);
	}
#ifdef RTLD_DEEPBIND
	if (!(*handle = dlopen(ptr, RTLD_NOW|RTLD_GLOBAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
	if (!(*handle = dlopen(ptr, RTLD_NOW|RTLD_GLOBAL))) {
#endif
		if (!stralloc_copys(&errbuf, dlerror())) {
			if (errstr)
				*errstr = memerr;
		} else
		if (!stralloc_0(&errbuf)) {
			if (errstr)
				*errstr = memerr;
		} else
		if (errstr)
			*errstr = errbuf.s;
		return ((void *) 0);
	}
	dlerror();
	if (errflag)
		*errflag = 0;
	return (*handle);
}

void
closeLibrary(void **handle)
{
	if (*handle) {
		dlclose(*handle);
		*handle = (void *) 0;
	}
	return;
}

static void *
getlibObject(char *libenv, void **handle, char *plugin_symb, char **errstr)
{
	void           *i;
	char           *ptr;

	if (!*handle)
		*handle = loadLibrary(handle, libenv, 0, errstr);
	if (!*handle)
		return ((void *) 0);
	i = dlsym(*handle, plugin_symb);
	if (!i && (!stralloc_copyb(&errbuf, "getlibObject: ", 14) ||
			!stralloc_cats(&errbuf, plugin_symb) ||
			!stralloc_catb(&errbuf, ": ", 2))) {
		if (errstr)
			*errstr = memerr;
	}
	if (!i && (ptr = dlerror()) && !stralloc_cats(&errbuf, ptr)) {
		if (errstr)
			*errstr = memerr;
	} else
	if (!i)
		errbuf.len--; /*- remove trailing colon */
	if (!i && !stralloc_0(&errbuf)) {
		if (errstr)
			*errstr = memerr;
	}
	if (!i && errstr)
		*errstr = errbuf.s;
	return (i);
}

int
initMySQLlibrary(char **errstr)
{
	static void    *phandle = (void *) 0;
	char           *ptr;
	int             i = -1;

	if (phandle)
		return (0);
	if (!(ptr = env_get("MYSQL_LIB")))
		ptr = CONTROLDIR"/libmysql";
	else
		ptr = "MYSQL_LIB";
	if (!(phandle = loadLibrary(&phandle, ptr, &i, errstr))) {
		if (!i)
			return (0);
		return (1);
	}
	if (!(in_mysql_init = getlibObject(ptr, &phandle, "mysql_init", errstr)))
		return (1);
	else
	if (!(in_mysql_real_connect = getlibObject(ptr, &phandle, "mysql_real_connect", errstr)))
		return (1);
	else
	if (!(in_mysql_error = getlibObject(ptr, &phandle, "mysql_error", errstr)))
		return (1);
	else
	if (!(in_mysql_errno = getlibObject(ptr, &phandle, "mysql_errno", errstr)))
		return (1);
	else
	if (!(in_mysql_next_result = getlibObject(ptr, &phandle, "mysql_next_result", errstr)))
		return (1);
	else
	if (!(in_mysql_close = getlibObject(ptr, &phandle, "mysql_close", errstr)))
		return (1);
	else
	if (!(in_mysql_options = getlibObject(ptr, &phandle, "mysql_options", errstr)))
		return (1);
#if MYSQL_VERSION_ID >= 50703 && !defined(MARIADB_BASE_VERSION)
	else
	if (!(in_mysql_get_option = getlibObject(ptr, &phandle, "mysql_get_option", errstr)))
		return (1);
#endif
	else
	if (!(in_mysql_query = getlibObject(ptr, &phandle, "mysql_query", errstr)))
		return (1);
	else
	if (!(in_mysql_store_result = getlibObject(ptr, &phandle, "mysql_store_result", errstr)))
		return (1);
	else
	if (!(in_mysql_fetch_row = getlibObject(ptr, &phandle, "mysql_fetch_row", errstr)))
		return (1);
	else
	if (!(in_mysql_fetch_lengths = getlibObject(ptr, &phandle, "mysql_fetch_lengths", errstr)))
		return (1);
	else
	if (!(in_mysql_num_rows = getlibObject(ptr, &phandle, "mysql_num_rows", errstr)))
		return (1);
	else
	if (!(in_mysql_num_fields = getlibObject(ptr, &phandle, "mysql_num_fields", errstr)))
		return (1);
	else
	if (!(in_mysql_affected_rows = getlibObject(ptr, &phandle, "mysql_affected_rows", errstr)))
		return (1);
	else
	if (!(in_mysql_free_result = getlibObject(ptr, &phandle, "mysql_free_result", errstr)))
		return (1);
	else
	if (!(in_mysql_stat = getlibObject(ptr, &phandle, "mysql_stat", errstr)))
		return (1);
	else
	if (!(in_mysql_ping = getlibObject(ptr, &phandle, "mysql_ping", errstr)))
		return (1);
	else
	if (!(in_mysql_real_escape_string = getlibObject(ptr, &phandle, "mysql_real_escape_string", errstr)))
		return (1);
	else
	if (!(in_mysql_get_proto_info = getlibObject(ptr, &phandle, "mysql_get_proto_info", errstr)))
		return (1);
	else
	if (!(in_mysql_get_host_info = getlibObject(ptr, &phandle, "mysql_get_host_info", errstr)))
		return (1);
	else
	if (!(in_mysql_select_db = getlibObject(ptr, &phandle, "mysql_select_db", errstr)))
		return (1);
	else
	if (!(in_mysql_ssl_set = getlibObject(ptr, &phandle, "mysql_ssl_set", errstr)))
		return (1);
	else
	if (!(in_mysql_get_ssl_cipher = getlibObject(ptr, &phandle, "mysql_get_ssl_cipher", errstr)))
		return (1);
	else
	if (!(in_mysql_data_seek = getlibObject(ptr, &phandle, "mysql_data_seek", errstr)))
		return (1);
	else
	if (!(in_mysql_get_server_info = getlibObject(ptr, &phandle, "mysql_get_server_info", errstr)))
		return (1);
	else
	if (!(in_mysql_get_client_info = getlibObject(ptr, &phandle, "mysql_get_client_info", errstr)))
		return (1);
	return (0);
}

MYSQL *
mysql_Init(MYSQL *mysql)
{
	char           *x;

	if (initMySQLlibrary(&x))
		strerr_die2x(111, "mysql_init: couldn't load libmysqlclient: ", x);
	return (in_mysql_init(mysql));
}
#else /*- DLOPEN_LIBMYSQLCLIENT */
#ifdef LIBMARIADB
typedef char bool;
#endif

MYSQL *
mysql_Init(MYSQL *mysql)
{
	return (mysql_init(mysql));
}

MYSQL          *
in_mysql_real_connect(MYSQL *mysql, const char *a1, const char *a2,
		const char *a3, const char *a4, unsigned int a5, const char *a6, unsigned long a7)
{
	return (mysql_real_connect(mysql, a1, a2, a3, a4, a5, a6, a7));
}

const char     *
in_mysql_error(MYSQL *mysql)
{
	return (mysql_error(mysql));
}

unsigned int
in_mysql_errno(MYSQL *mysql)
{
	return (mysql_errno(mysql));
}

int
in_mysql_next_result(MYSQL *mysql)
{
	return (mysql_next_result(mysql));
}

void
in_mysql_close(MYSQL *mysql)
{
	mysql_close(mysql);
	return;
}

int
in_mysql_options(MYSQL *mysql, enum mysql_option option, const void *arg)
{
	return (mysql_options(mysql, option, arg));
}

#if MYSQL_VERSION_ID >= 50703 && !defined(MARIADB_BASE_VERSION)
int
in_mysql_get_option(MYSQL *mysql, enum mysql_option option, void *arg)
{
	return (mysql_get_option(mysql, option, arg));
}
#endif

int
in_mysql_query(MYSQL *mysql, const char *q)
{
	return (mysql_query(mysql, q));
}

MYSQL_RES      *
in_mysql_store_result(MYSQL *mysql)
{
	return (mysql_store_result(mysql));
}

MYSQL_ROW
in_mysql_fetch_row(MYSQL_RES *result)
{
	return (mysql_fetch_row(result));
}

unsigned long  *
in_mysql_fetch_lengths(MYSQL_RES *result)
{
	return (mysql_fetch_lengths(result));
}

my_ulonglong
in_mysql_num_rows(MYSQL_RES *result)
{
	return (mysql_num_rows(result));
}

unsigned int
in_mysql_num_fields(MYSQL_RES *result)
{
	return(mysql_num_fields(result));
}

my_ulonglong
in_mysql_affected_rows(MYSQL *mysql)
{
	return (mysql_affected_rows(mysql));
}

void
in_mysql_free_result(MYSQL_RES *result)
{
	mysql_free_result(result);
	return;
}

const char     *
in_mysql_stat(MYSQL *mysql)
{
	return (mysql_stat(mysql));
}

int
in_mysql_ping(MYSQL *mysql)
{
	return (mysql_ping(mysql));
}

unsigned long
in_mysql_real_escape_string(MYSQL *mysql, char *to, const char *from, unsigned long length)
{
	return (mysql_real_escape_string(mysql, to, from, length));
}

unsigned int
in_mysql_get_proto_info(MYSQL *mysql)
{
	return (mysql_get_proto_info(mysql));
}

const char     *
in_mysql_get_host_info(MYSQL *mysql)
{
	return (mysql_get_host_info(mysql));
}

int
in_mysql_select_db(MYSQL *mysql, const char *db)
{
	return (mysql_select_db(mysql, db));
}

bool
in_mysql_ssl_set(MYSQL *mysql, const char *key, const char *cert, const char *ca,
		const char *capath, const char *cipher)
{
	return (mysql_ssl_set(mysql, key, cert, ca, capath, cipher));
}

const char     *
in_mysql_get_ssl_cipher(MYSQL *mysql)
{
	return (mysql_get_ssl_cipher(mysql));
}

void
in_mysql_data_seek(MYSQL_RES *mysql, my_ulonglong offset)
{
	return (mysql_data_seek(mysql, offset));
}

const char     *
in_mysql_get_server_info(MYSQL *mysql)
{
	return (mysql_get_server_info(mysql));
}

const char     *in_mysql_get_client_info(void)
{
	return (mysql_get_client_info());
}
#endif /*- #ifdef DLOPEN_LIBMYSQLCLIENT */

void
getversion_load_mysql_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}

/*
 * $Log: load_mysql.c,v $
 * Revision 1.13  2022-11-23 15:49:46+05:30  Cprogrammer
 * renamed mysql_lib to libmysql
 *
 * Revision 1.12  2020-10-05 18:42:36+05:30  Cprogrammer
 * fixed compilation warning on Darwin
 *
 * Revision 1.11  2020-09-23 11:00:40+05:30  Cprogrammer
 * fold braces for code readability
 *
 * Revision 1.10  2020-06-08 23:43:18+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.9  2019-07-03 19:37:17+05:30  Cprogrammer
 * added getversion_load_mysql_c()
 *
 * Revision 1.8  2019-06-13 19:14:35+05:30  Cprogrammer
 * added wrappers for mysql_next_result(), mysql_fetch_lengths(), mysql_num_fields()
 *
 * Revision 1.7  2019-06-09 17:39:16+05:30  Cprogrammer
 * conditional compilation of bool typedef
 *
 * Revision 1.6  2019-06-08 18:10:50+05:30  Cprogrammer
 * define bool unconditionally as older mariadb devel package don't have #ifdef LIBMARIADB
 *
 * Revision 1.5  2019-06-07 19:22:07+05:30  Cprogrammer
 * treat missing libmysqlclient as error
 *
 * Revision 1.4  2019-06-07 17:31:29+05:30  Cprogrammer
 * changed scope of closeLibrary() to global
 *
 * Revision 1.3  2019-06-07 16:07:56+05:30  Cprogrammer
 * fix for missing mysql_get_option() in new versions of libmariadb
 *
 * Revision 1.2  2019-06-03 06:50:56+05:30  Cprogrammer
 * use RTLD_NODELETE
 *
 * Revision 1.1  2019-05-28 16:17:27+05:30  Cprogrammer
 * Initial revision
 *
 */
