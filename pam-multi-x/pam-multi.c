/*
 * $Id: pam-multi.c,v 1.22 2025-01-22 16:04:09+05:30 Cprogrammer Exp mbhangui $
 *
 * pam-multi.c - Generic PAM Authentication module
 * Copyright (C) <2008>  Manvendra Bhangui <mbhangui@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * The GNU General Public License does not permit incorporating your program
 * into proprietary programs.  If your program is a subroutine library, you
 * may consider it more useful to permit linking proprietary applications with
 * the library.  If this is what you want to do, use the GNU Lesser General
 * Public License instead of this License.  But first, please read
 * <http://www.gnu.org/philosophy/why-not-lgpl.html>.
 *
 * Testing
 * pamtester imap postmaster@indimail.org authenticate
 *
 */
#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <getopt.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif
#ifdef USE_MYSQL
#include "load_mysql.h"
#endif
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

/*
 * here, we make definitions for the externally accessible functions
 * in this file (these definitions are required for static modules
 * but strongly encouraged generally) they are used to instruct the
 * modules include file to define their prototypes.
 */

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#define PAM_SM_PASSWORD

#ifdef HAVE_PAM_PAM_MODULES_H
#include <pam/pam_modules.h>
#endif
#ifdef HAVE_SECURITY_PAM_MODULES_H
#include <security/pam_modules.h>
#endif

#ifdef HAVE_PAM_PAM_APPL_H
#include <pam/pam_appl.h>
#endif
#ifdef HAVE_SECURITY_PAM_APPL_H
#include <security/pam_appl.h>
#endif

#ifndef PAM_EXTERN
#define PAM_EXTERN
#endif

#ifdef LINUX_PAM_CONST_BUG
#define PAM_AUTHTOK_RECOVERY_ERR PAM_AUTHTOK_RECOVER_ERR
#endif

#define wait_crashed(w)  ((w) & 127)
#define wait_exitcode(w) ((w) >> 8)

#define PAM_MODULE_NAME             "pam-multi"
#define PAM_MYSQL_LOG_PREFIX        PAM_MODULE_NAME " - "
#define PLEASE_ENTER_PASSWORD       "Password:"
#define PLEASE_ENTER_OLD_PASSWORD   "(Current) Password:"
#define PLEASE_ENTER_NEW_PASSWORD   "(New) Password:"
#define PLEASE_REENTER_NEW_PASSWORD "Retype (New) Password:"
#define MAX_QUERY_LENGTH 255

#define MAX_RETRIES     3
#define DEFAULT_WARN    (2L * 7L * 86400L)	/* two weeks */
#define SALTSIZE        32

#define MYSQL_MODE      0
#define COMMAND_MODE    1
#define LIB_MODE        2

#define DES_HASH        0
#define MD5_HASH        1
#define SHA256_HASH     2
#define SHA512_HASH     3

PAM_EXTERN int  pam_sm_authenticate(pam_handle_t * pamh, int flags, int argc, const char **argv);
PAM_EXTERN int  pam_sm_chauthtok(pam_handle_t * pamh, int flags, int argc, const char **argv);
PAM_EXTERN int  pam_sm_acct_mgmt(pam_handle_t * pamh, int flags, int argc, const char **argv);
PAM_EXTERN int  pam_sm_setcred(pam_handle_t * pamh, int flags, int argc, const char **argv);
PAM_EXTERN int  pam_sm_open_session(pam_handle_t * pamh, int flags, int argc, const char **argv);
PAM_EXTERN int  pam_sm_close_session(pam_handle_t * pamh, int flags, int argc, const char **argv);
char           *md5_crypt(const char *, const char *);
char           *sha256_crypt(const char *, const char *);
char           *sha512_crypt(const char *, const char *);
#ifdef HAVE_SHADOW_H
static int      update_shadow(pam_handle_t *, const char *, const char *);
#endif
#ifdef WHY_IS_THIS_NEEDED
static int      update_passwd(pam_handle_t *, const char *, const char *);
#endif

#ifndef	lint
static char     sccsid[] = "$Id: pam-multi.c,v 1.22 2025-01-22 16:04:09+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * logging function ripped from pam_listfile.c 
 */
static void
_pam_log(int err, const char *fmt, ...)
{
	va_list         ap;

	va_start(ap, fmt);
	(void) vfprintf(stderr, fmt, ap);
	(void) fprintf(stderr, "\n");
	va_end(ap);
	openlog(PAM_MODULE_NAME, LOG_PID, LOG_AUTHPRIV);
	va_start(ap, fmt);
	vsyslog(err, fmt, ap);
	va_end(ap);
	closelog();
}

int
wait_pid(int *wstat, int pid)
{
	int             r;

	do {
		r = waitpid(pid, wstat, 0);
	} while ((r == -1) && (errno == EINTR));
	return r;
}

static int
run_command(char *command_args, int mode, const char *user, char **qresult,
	int *nitems, int debug)
{
	char            buffer[1024];
	char           *ptr = (char *) 0;
	pid_t           filt_pid;
	int             pipefd[2];
	int             wstat, filt_exitcode, len, n;

	*qresult = 0;
	if (nitems)
		*nitems = 0;
	if (debug)
		_pam_log(LOG_INFO, "run_command user %s command [%s]", user, command_args);
	if (pipe(pipefd) == -1) {
		_pam_log(LOG_ERR, "pipe: %s", strerror(errno));
		return (-1);
	}
	switch ((filt_pid = fork())) {
	case -1:
		_pam_log(LOG_ERR, "fork: %s", strerror(errno));
		return (-1);
	case 0:
		   /*- Command */
		if (mode == 1) {
			if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1) {
				_pam_log(LOG_ERR, "dup2: %s", strerror(errno));
				_exit(60);
			}
			if (pipefd[1] != 1)
				close(pipefd[1]);
		}
		execl("/bin/sh", "pam-multi", "-c", command_args, (char *) 0);
		_pam_log(LOG_ERR, "execl: %s: %s", command_args, strerror(errno));
		_exit(75);
	default:
		if (mode == 1) {
			if (close(pipefd[1]) == -1) {
				close(pipefd[0]);
				wait_pid(&wstat, filt_pid);
				return (-1);
			}
			for (len = 0;;) {
				if ((n = read(pipefd[0], buffer, sizeof (buffer))) == -1) {
					if (errno == EINTR)
						continue;
				} else
				if (!n)
					break;
				if (!ptr) {
					if (!(ptr = malloc(len + n + 1))) {
						_pam_log(LOG_ERR, "malloc: %s", strerror(errno));
						return (-1);
					}
				} else {
					if (!(ptr = realloc(ptr, len + n + 1))) {
						_pam_log(LOG_ERR, "malloc: %s", strerror(errno));
						return (-1);
					}
				}
				memcpy(ptr + len, buffer, n);
				len += n;
			}
			if (ptr)
				ptr[len] = 0;
			*qresult = ptr;
		}
		if (wait_pid(&wstat, filt_pid) != filt_pid)
			return (-1);
		if (wait_crashed(wstat))
			return (-1);
		break;
	}
	if (nitems)
		*nitems = 1;
	return (filt_exitcode = wait_exitcode(wstat));
}

#ifndef INDIMAIL
void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = getenv(envname)))
		*source = defaultValue;
	return;
}
#endif

static int
run_mysql(char *mysql_user, char *mysql_pass, char *mysql_host, char *mysql_database, int mysql_port, char *query_str,
		  const char *user, char **qresult, int *nitems, int debug)
{
	char            thischar, lastchar;
	char           *sql, *p, *q;
	char           *escapeUser;	/* User provided stuff MUST be escaped */
	unsigned int    user_length, domain_length, len, i, j, row_count;
	MYSQL           mysql;
	MYSQL_RES      *result;
	MYSQL_ROW       row;

	if (debug)
		_pam_log(LOG_INFO, "run_mysql user[%s],pass[%s],host[%s], database[%s], port[%d], query[%s], User[%s]", mysql_user, mysql_pass,
			 mysql_host, mysql_database, mysql_port, query_str, user);
	*qresult = (char *) 0;
	if (nitems)
		*nitems = 0;
	if (!mysql_Init(&mysql)) {
		_pam_log(LOG_ERR, "mysql_init: %s", in_mysql_error(&mysql));
		return (PAM_SERVICE_ERR);
	}
	if (!(in_mysql_real_connect(&mysql, mysql_host, mysql_user, mysql_pass, mysql_database, mysql_port, NULL, 0))) {
		_pam_log(LOG_ERR, "mysql_real_connect: %s", in_mysql_error(&mysql));
		return (PAM_SERVICE_ERR);
	}
	if (!(escapeUser = malloc(sizeof (char) * (strlen(user) * 2) + 1))) {
		_pam_log(LOG_ERR, "malloc: %s", strerror(errno));
		return PAM_BUF_ERR;
	}
	in_mysql_real_escape_string(&mysql, escapeUser, user, strlen(user));
	for (q = (char *) user; *q && *q != '@'; q++);
	if (*q) {
		*q = 0;
		q++;
	} else
		getEnvConfigStr(&q, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (p = escapeUser; *p && *p != '@'; p++);
	if (*p) {
		*p = 0;
		p++;
	} else
		getEnvConfigStr(&p, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	domain_length = strlen(p);
	user_length = strlen(escapeUser);
	if (!(sql = (char *) malloc(sizeof (char) * (MAX_QUERY_LENGTH + 1)))) {
		_pam_log(LOG_ERR, "malloc: %s", strerror(errno));
		in_mysql_close(&mysql);
		return PAM_BUF_ERR;
	}
	len = strlen(query_str);
	for (i = j = lastchar = 0; i < len; i++) {
		thischar = query_str[i];
		if (lastchar == '%') {
			switch (thischar)
			{
			case 'U':
				if ((j + user_length) >= MAX_QUERY_LENGTH) {
					_pam_log(LOG_ERR, "pam_db: query too long");
					in_mysql_close(&mysql);
					return PAM_BUF_ERR;
				}
				strcpy(sql + j, user);
				j += strlen(user);
				lastchar = 0;
				break;
			case 'u':
				if ((j + user_length) >= MAX_QUERY_LENGTH) {
					_pam_log(LOG_ERR, "pam_db: query too long");
					in_mysql_close(&mysql);
					return PAM_BUF_ERR;
				}
				strcpy(sql + j, escapeUser);
				j += user_length;
				lastchar = 0;
				break;
			case 'D':
				if ((j + domain_length) >= MAX_QUERY_LENGTH) {
					_pam_log(LOG_ERR, "pam_db: query too long");
					in_mysql_close(&mysql);
					return PAM_BUF_ERR;
				}
				strcpy(sql + j, q);
				j += strlen(q);
				lastchar = 0;
				break;
			case 'd':
				if ((j + domain_length) >= MAX_QUERY_LENGTH) {
					_pam_log(LOG_ERR, "pam_db: query too long");
					in_mysql_close(&mysql);
					return PAM_BUF_ERR;
				}
				strcpy(sql + j, p);
				j += domain_length;
				lastchar = 0;
				break;
			case '%':
			default:
				lastchar = (lastchar == '%' ? 0 : thischar);
				sql[j++] = thischar;
				if (j > MAX_QUERY_LENGTH) {
					_pam_log(LOG_ERR, "pam_db: query too long");
					in_mysql_close(&mysql);
					return PAM_BUF_ERR;
				}
				break;
			}
		} else {
			if (thischar != '%')
				sql[j++] = thischar;
			if (j > MAX_QUERY_LENGTH) {
				_pam_log(LOG_ERR, "pam_db: query too long");
				in_mysql_close(&mysql);
				return PAM_BUF_ERR;
			}
			lastchar = thischar;
		}
	}
	sql[j] = 0;
	if (debug)
		_pam_log(LOG_INFO, "run_mysql user[%s],pass[%s],host[%s], database[%s], port[%d], query[%s], User[%s]", mysql_user, mysql_pass,
			 mysql_host, mysql_database, mysql_port, sql, user);
	if (in_mysql_query(&mysql, sql)) {
		_pam_log(LOG_ERR, "mysql_query: %s: %s", sql, in_mysql_error(&mysql));
		in_mysql_close(&mysql);
		return (PAM_SERVICE_ERR);
	}
	if (!(result = in_mysql_store_result(&mysql))) {
		_pam_log(LOG_ERR, "mysql_store_result: %s", in_mysql_error(&mysql));
		in_mysql_close(&mysql);
		return (PAM_SERVICE_ERR);
	}
	if (!(row_count = in_mysql_num_rows(result))) {
		in_mysql_free_result(result);
		in_mysql_close(&mysql);
		return (PAM_AUTH_ERR);
	}
	in_mysql_close(&mysql);
	if ((row = in_mysql_fetch_row(result))) {
		if (!(*qresult = (char *) malloc((len = strlen(row[0]) + 1)))) {
			_pam_log(LOG_ERR, "malloc: %s", strerror(errno));
			in_mysql_free_result(result);
			return (PAM_BUF_ERR);
		} else {
			if (nitems)
				*nitems = 1;
			strncpy(*qresult, row[0], len);
			in_mysql_free_result(result);
			return (PAM_SUCCESS);
		}
	} else
		in_mysql_free_result(result);
	return (PAM_AUTH_ERR);
}

#ifdef HAVE_DLFCN_H
static int
loadLIB(char *shared_lib, const char *user, const char *service, char **qresult,
	int auth_or_acct_mgmt, int *nitems, int debug)
{
	void           *handle;
	int             size;
	char           *error, *ptr;
	void           *(*func) (const char *, const char *, int, int *, int *, int);

	*qresult = 0;
	if (debug)
		_pam_log(LOG_INFO, "loadLIB %s %s", user, service ? service : "no service");
	if (!(handle = dlopen(shared_lib, RTLD_LAZY|RTLD_NODELETE))) {
		_pam_log(LOG_ERR, "dlopen: %s", dlerror());
		return (PAM_SERVICE_ERR);
	}
	dlerror(); /*- man page told me to do this */
	func = dlsym(handle, "iauth");
	if ((error = dlerror())) {
		_pam_log(LOG_ERR, "dlsym: %s", error);
		dlclose(handle);
		return (PAM_SERVICE_ERR);
	}
	if (!(ptr = (char *) (*func) (user, service, auth_or_acct_mgmt, &size, nitems, debug))) {
		dlclose(handle);
		return (PAM_AUTH_ERR);
	}
	if (debug)
		_pam_log(LOG_INFO, "loadLIB nitems=%d, size=%d", nitems ? *nitems : 0, size);
	if (!size) {
		dlclose(handle);
		return (PAM_AUTH_ERR);
	}
	if (!(*qresult = (char *) malloc(size))) {
		_pam_log(LOG_ERR, "malloc: %s", strerror(errno));
		dlclose(handle);
		return (PAM_BUF_ERR);
	} else {
		if (debug)
			_pam_log(LOG_INFO, "loadLIB nitems=%d, size=%d", nitems ? *nitems : 0, size);
		memcpy(*qresult, ptr, size);
	}
	if (dlclose(handle)) {
		_pam_log(LOG_ERR, "dlsym: %s", error);
		return (PAM_SERVICE_ERR);
	}
	return (PAM_SUCCESS);
}
#endif

char           *
cryptMethod(int method)
{
	switch (method)
	{
	case DES_HASH:
		return ("DES");
	case MD5_HASH:
		return ("MD5");
	case SHA256_HASH:
		return ("SHA256");
	case SHA512_HASH:
		return ("SHA521");
	}
	return ("unknown");
}

int
pw_comp(const char *plain_text, char *crypted, int method, int debug)
{
	char           *crypt_pass;

	if (debug)
		_pam_log(LOG_INFO, "plain[%s], crypted[%s], method %s", plain_text, crypted, cryptMethod(method));
	switch (method)
	{
	case DES_HASH:
		if (!(crypt_pass = crypt((char *) plain_text, (char *) crypted)))
			return (-1);
		break;
	case MD5_HASH:
		if (!(crypt_pass = md5_crypt((char *) plain_text, (char *) crypted)))
			return (-1);
		break;
	case SHA256_HASH:
		if (!(crypt_pass = sha256_crypt((char *) plain_text, (char *) crypted)))
			return (-1);
		break;
	case SHA512_HASH:
		if (!(crypt_pass = sha512_crypt((char *) plain_text, (char *) crypted)))
			return (-1);
		break;
	default:
		return (1);
	}
	return (strncmp((const char *) crypt_pass, (const char *) crypted, strlen(crypt_pass) + 1));
}

int
converse(pam_handle_t *pamh, int flags, const char *prompt, const char **password)
{
	struct pam_conv *conv;
	struct pam_message msg;
	struct pam_message *msgp;
	struct pam_response *resp;
	int             retry, pam_err;

	if ((pam_err = pam_get_item(pamh, PAM_CONV, (const void **) &conv)) != PAM_SUCCESS) {
		_pam_log(LOG_AUTHPRIV | LOG_ERR, PAM_MYSQL_LOG_PREFIX "could not obtain coversation interface (reason: %s)",
			   pam_strerror(pamh, pam_err));
		return (pam_err == PAM_PERM_DENIED ? PAM_AUTH_ERR : pam_err);
	}
	msg.msg_style = PAM_PROMPT_ECHO_OFF;
	msg.msg = (char *) prompt;
	msgp = &msg;
	*password = NULL;
	for (retry = 0; retry < MAX_RETRIES; ++retry) {
		resp = NULL;
		pam_err = (*conv->conv) (1, (const struct pam_message **) &msgp, (struct pam_response **) &resp, conv->appdata_ptr);
		if (resp) {
			if ((flags & PAM_DISALLOW_NULL_AUTHTOK) && resp[0].resp == NULL) {
				free(resp);
				return (PAM_AUTH_ERR);
			}
			if (pam_err == PAM_SUCCESS) {
				*password = resp->resp;
				return (PAM_SUCCESS);
			} else
				free(resp->resp);
			free(resp);
		} else
			return (PAM_CONV_ERR); /*- have to check on this */
		if (pam_err == PAM_SUCCESS)
			break;
	}
	return (PAM_AUTH_ERR);
}

/*
 * Mostly stolen from passwd(1)'s local_passwd.c - markm 
 */
int
Arc4random(int start_num, int end_num)
{
	int             j;
	float           fnum;

	fnum = (float) end_num;
	j = start_num + (int) (fnum * rand() / (RAND_MAX + 1.0));
	return (j);
}

/*
 * Salt suitable for traditional DES and MD5 
 */
void
makesalt(char salt[SALTSIZE + 1])
{
	int             i, len;
	static int      seeded;
	static char     itoa64[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz./";
		/* 0 ... 63 => ascii - 64 */

	/*
	 * These are not really random numbers, they are just
	 * numbers that change to thwart construction of a
	 * dictionary. This is exposed to the public.
	 */
	if (!seeded) {
		seeded = 1;
		srand(time(0)^(getpid()<<15));
	}
	len = strlen(itoa64);
	for (i = 0; i < SALTSIZE; i++)
		/* generate random no from 0 to len */
		salt[i] = itoa64[Arc4random(0, len - 1)];
	salt[SALTSIZE] = '\0';
}

static char    *_global_user;

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	int             mode, pam_err, errflag, c, debug = 0, status, crypt_method = DES_HASH,
					mysql_port = 3306;
	const char     *user, *password, *rhost;
	char           *mysql_query_str, *mysql_user, *mysql_pass, *mysql_host, *mysql_database,
				   *command_str, *result;
#ifdef HAVE_DLFCN_H
	char           *shared_lib, *service;
#endif

	mysql_user = mysql_pass = mysql_database = mysql_host = command_str = (char *) 0;
	mysql_query_str = (char *) 0;
#ifdef HAVE_DLFCN_H
	shared_lib = service = 0;
#endif
	_global_user = (char *) 0;
	mode = -1;
	/*-
	 * Substitutions:
	 * %U - Username
	 * %D - Domain
	 * %S - Secret
	 */
	if (argc < 3)
		_pam_log(LOG_ERR, "Invalid PAM configuration (less than 3 arguments). Check config file");
	optind = 1;
#ifdef DARWIN
	optreset = 1;
#endif
	opterr = errflag = 0;
#ifdef HAVE_DLFCN_H
	while ((c = getopt(argc, (char **) argv, "dm:u:p:D:H:P:c:s:i:")) != -1) {
#else
	while ((c = getopt(argc, (char **) argv, "dm:u:p:D:H:P:c:")) != -1) {
#endif
		switch (c)
		{
		case 'd':	/* debug */
			debug = 1;
			break;
		case 'm':	/* MySQL query */
			mode = MYSQL_MODE;
			mysql_query_str = optarg;
			break;
		case 'u':	/* username */
			mysql_user = optarg;
			break;
		case 'p':	/* password */
			mysql_pass = optarg;
			break;
		case 'D':	/* database */
			mysql_database = optarg;
			break;
		case 'H':	/* host */
			mysql_host = optarg;
			break;
		case 'P':	/* Port */
			mysql_port = atoi(optarg);
			break;
		case 'c':	/* Command or pipe */
			mode = COMMAND_MODE;
			command_str = optarg;
			break;
#ifdef HAVE_DLFCN_H
		case 'i':	/* identifier */
			service = optarg;
			break;
		case 's':	/* shared library */
			mode = LIB_MODE;
			shared_lib = optarg;
			break;
#endif
		default:
			errflag = 1;
			break;
		}
		if (debug)
			_pam_log(LOG_INFO, "optind=%d, c=[%c]", optind, c);
	}
	if (debug)
		for (c = 0; c < argc; c++)
			_pam_log(LOG_INFO, "arg[%d]=[%s]", c, argv[c]);
	if (!service)
		service = getenv("AUTHSERVICE");
	if (errflag > 0) {
		_pam_log(LOG_ERR, "Invalid PAM configuration. Check config file");
		return (PAM_SERVICE_ERR);
	}
	if (mode < 0) {
#ifdef HAVE_DLFCN_H
		_pam_log(LOG_ERR, "Invalid PAM configuration (without -m, -c, -s). Mode must be mysql, command or lib");
#else
		_pam_log(LOG_ERR, "Invalid PAM configuration (without -m, -c). Mode must be mysql or command");
#endif
		return (PAM_SERVICE_ERR);
	}
	if ((pam_err = pam_get_user(pamh, (const char **) &user, NULL)) != PAM_SUCCESS) {
		_pam_log(LOG_ERR, "pam_get_user (reason: %s)", pam_strerror(pamh, pam_err));
		return (pam_err);
	}
	if (!user)
		return (PAM_USER_UNKNOWN);
	if (debug)
		_pam_log(LOG_INFO, "sm_auth %s", user);
	switch ((pam_err = pam_get_item(pamh, PAM_RHOST, (const void **) &rhost)))
	{
	case PAM_SUCCESS:
		break;
	default:
		rhost = NULL;
	}
	/*
	 *  PAM_BAD_ITEM
	 *  The application attempted to set an undefined or inaccessible item.
	 *
	 *  PAM_BUF_ERR
	 *  Memory buffer error.
	 *
	 *  PAM_PERM_DENIED
	 *  The value of item was NULL.
	 *
	 *  PAM_SUCCESS
	 *  Data was successful updated.
	 *
	 *  PAM_SYSTEM_ERR
	 *  The pam_handle_t passed as first argument was invalid.
	 *
	 */
	/*- get password */
	if ((pam_err = pam_get_item(pamh, PAM_AUTHTOK, (const void **) &password)) != PAM_SUCCESS) {
		_pam_log(LOG_AUTHPRIV | LOG_ERR, PAM_MYSQL_LOG_PREFIX "PAM_AUTHTOK-password (reason: %s)", pam_strerror(pamh, pam_err));
		return (pam_err == PAM_PERM_DENIED ? PAM_AUTH_ERR : pam_err);
	}
	if (!password && (pam_err = converse(pamh, flags, PLEASE_ENTER_PASSWORD, &password)) != PAM_SUCCESS)
		return (pam_err == PAM_CONV_ERR ? pam_err : PAM_AUTH_ERR);
	switch (mode) 
	{
	case COMMAND_MODE:
		status = run_command(command_str, 1, user, &result, 0, debug);
		break;
#ifdef HAVE_DLFCN_H
	case LIB_MODE:
		status = loadLIB(shared_lib, user, service, &result, 0, 0, debug);
		break;
#endif
	case MYSQL_MODE:
		status = run_mysql(mysql_user, mysql_pass, mysql_host,
			mysql_database, mysql_port, mysql_query_str, user, &result, 0, debug);
		break;
	default:
		status = 1;
		break;
	}
	if (status) {
		if (debug)
			_pam_log(LOG_INFO, "status=%d", status);
		free((void *) password);
		return (PAM_SERVICE_ERR);
	}
	/*- Override command line */
	if (!memcmp(result, "$1$", 3))
		crypt_method = MD5_HASH;
	else if (!memcmp(result, "$5$", 3))
		crypt_method = SHA256_HASH;
	else if (!memcmp(result, "$6$", 3))
		crypt_method = SHA512_HASH;
	else
		crypt_method = DES_HASH;
	if (!pw_comp(password, result, crypt_method, debug)) {
		_global_user = (char *) user;
		if (result)
			free(result);
		free((void *) password);
		return (PAM_SUCCESS);
	}
	sleep(5);
	if (result)
		free(result);
	free((void *) password);
	return (PAM_AUTH_ERR);
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t * pamh, int flags, int argc, const char *argv[])
{
	/*
	 * This functions takes care of renewing/initializing
	 * user credentials as well as gid/uids. Someday, it
	 * will be completed. For now, it's not very urgent. 
	 */
	return (PAM_SUCCESS);
}

/*
 * PAM_ACCT_EXPIRED
 * User account has expired.
 *
 * PAM_AUTH_ERR
 * Authentication failure.
 *
 * PAM_NEW_AUTHTOK_REQD
 * The user´s authentication token has expired. Before calling this function
 * again the application will arrange for a new one to be given. This will
 * likely result in a call to pam_sm_chauthtok().
 *
 * PAM_PERM_DENIED
 * Permission denied.
 *
 * PAM_SUCCESS
 * The authentication token was successfully updated.
 *
 * PAM_USER_UNKNOWN
 * User unknown to password service.
 */
PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char *argv[])
{
	const char     *user, *rhost;
	int             c, status, mode, pam_err, errflag, debug = 0,
					nitems = 0, mysql_port = 3306;
	char           *mysql_query_str, *mysql_user, *mysql_pass, *mysql_host, *mysql_database,
				   *command_str, *result = 0, *ptr;
	long            exp_times[2];
	long           *expiry_ptr;
	time_t          curtime;
#ifdef HAVE_DLFCN_H
	char           *shared_lib, *service;
#endif

	if (!_global_user)
		return (PAM_AUTH_ERR);
	/*- Calculate current time */
	curtime = time(0);
	if ((pam_err = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS)
		return (pam_err);
	if (!user || !*user)
		return (PAM_USER_UNKNOWN);
	if (strcmp(user, _global_user))
		return (PAM_AUTH_ERR);
	mysql_user = mysql_pass = mysql_database = mysql_host = command_str = (char *) 0;
#ifdef HAVE_DLFCN_H
	shared_lib = service = 0;
#endif
	mysql_query_str = (char *) 0;
	mode = -1;
	/*-
	 * Substitutions:
	 * %U - Username
	 * %D - Domain
	 * %S - Secret
	 */
	opterr = errflag = 0;
	optind = 1;
#ifdef DARWIN
	optreset = 1;
#endif
#ifdef HAVE_DLFCN_H
	while (!errflag && (c = getopt(argc, (char **) argv, "dm:u:p:D:H:P:c:s:i:")) != -1) {
#else
	while (!errflag && (c = getopt(argc, (char **) argv, "dm:u:p:D:H:P:c:")) != -1) {
#endif
		switch (c)
		{
		case 'd':	/* debug */
			debug = 1;
			break;
		case 'm':	/* MySQL query */
			mode = MYSQL_MODE;
			mysql_query_str = optarg;
			break;
		case 'u':	/* username */
			mysql_user = optarg;
			break;
		case 'p':	/* password */
			mysql_pass = optarg;
			break;
		case 'D':	/* database */
			mysql_database = optarg;
			break;
		case 'H':	/* host */
			mysql_host = optarg;
			break;
		case 'P':	/* Port */
			mysql_port = atoi(optarg);
			break;
		case 'c':	/* Command or pipe */
			mode = COMMAND_MODE;
			command_str = optarg;
			break;
#ifdef HAVE_DLFCN_H
		case 'i':	/* identifier */
			service = optarg;
			break;
		case 's':	/* shared library */
			mode = LIB_MODE;
			shared_lib = optarg;
			break;
#endif
		default:
			errflag = 1;
			break;
		}
		if (debug)
			_pam_log(LOG_INFO, "optind=%d, c=[%c]", optind, c);
	}
	if (!service)
		service = getenv("AUTHSERVICE");
	if (debug)
		_pam_log(LOG_INFO, "sm_acct_mgmt %s", user);
	switch ((pam_err = pam_get_item(pamh, PAM_RHOST, (const void **) &rhost)))
	{
	case PAM_SUCCESS:
		break;
	default:
		rhost = NULL;
	}
	switch (mode) 
	{
	case COMMAND_MODE:
		if (!(status = run_command(command_str, 1, user, &result, &nitems, debug))) {
			if (debug)
				_pam_log(LOG_INFO, "sm_acct_mgmt result[%s]", result ? result : "null");
			for (ptr = result;*ptr;ptr++) {
				if (*ptr == ',' && *(ptr + 1)) {
					exp_times[1] = atol(ptr + 1);
					*ptr = 0;
					break;
				}
			}
		}
		exp_times[0] = atol(result);
		result = (char *) &exp_times[0];
		break;
#ifdef HAVE_DLFCN_H
	case LIB_MODE:
		status = loadLIB(shared_lib, user, service, &result, 1, &nitems, debug);
		break;
#endif
	case MYSQL_MODE:
		status = run_mysql(mysql_user, mysql_pass, mysql_host,
			mysql_database, mysql_port, mysql_query_str, user, &result, &nitems, debug);
		break;
	default:
		_pam_log(LOG_INFO, "invalid/insufficient arguments");
		status = 1;
		break;
	}
	if (status) {
		if (debug)
			_pam_log(LOG_INFO, "status=%d", status);
		return (PAM_SERVICE_ERR);
	}
	expiry_ptr = (long *) result;
	for (c = 0;c < nitems;c++) {
		switch (c)
		{
		case 0:
			ptr = "Account";
			break;
		case 1:
			ptr = "Password";
			break;
		default:
			ptr = "Account";
			break;
		}
		if (debug)
			_pam_log(LOG_INFO, "expiry[%d]=%ld", c, *expiry_ptr);
		if (*expiry_ptr > 0) {
			if (curtime > *expiry_ptr) {
				_pam_log(LOG_WARNING, "%s has expired!", ptr);
				return (PAM_ACCT_EXPIRED);
			} else
			if (*expiry_ptr - curtime < DEFAULT_WARN)
				_pam_log(LOG_WARNING, "Warning: your %s expires on %s", ptr, ctime(expiry_ptr));
		}
		expiry_ptr++;
	}
	return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_open_session(pam_handle_t * pamh, int flags, int argc, const char *argv[])
{
	int             pam_err;
	char           *user, *service;

	if ((pam_err = pam_get_user(pamh, (const char **) &user, NULL)) != PAM_SUCCESS)
		return (pam_err);
	if (!user || !*user) {
		_pam_log(LOG_AUTHPRIV | LOG_ERR, " no user specified.");
		return (PAM_USER_UNKNOWN);
	}
	pam_err = pam_get_item(pamh, PAM_SERVICE, (void *) &service);
	if (pam_err != PAM_SUCCESS || service == NULL || *service == '\0') {
		fprintf(stderr, "Open session - Error recovering service");
		return (PAM_SESSION_ERR);
	}
	_pam_log(LOG_ERR, "Opened session for user [%s] by %s(uid=%lu)", user, getlogin(), (unsigned long) getuid());
	_pam_log(LOG_INFO, "open_session called but not implemented.");
	return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t * pamh, int flags, int argc, const char *argv[])
{
	int             pam_err;
	char           *user, *service;

	pam_err = pam_get_item(pamh, PAM_USER, (void *) &user);
	if (pam_err != PAM_SUCCESS || user == NULL || *user == '\0') {
		_pam_log(LOG_ERR, "Close session - Error recovering username");
		return (PAM_SESSION_ERR);
	}
	pam_err = pam_get_item(pamh, PAM_SERVICE, (void *) &service);
	if (pam_err != PAM_SUCCESS || service == NULL || *service == '\0') {
		_pam_log(LOG_ERR, "Close session - Error recovering service");
		return (PAM_SESSION_ERR);
	}
	_pam_log(LOG_INFO, "close_session called but not implemented.");
	return PAM_SUCCESS;
}

PAM_EXTERN int
pam_sm_chauthtok(pam_handle_t * pamh, int flags, int argc, const char *argv[])
{
	struct passwd  *pwd;
	const char     *old_pass, *new_pass;
	char           *hashedpwd, salt[SALTSIZE + 1];
	int             retries;
#ifdef HAVE_SHADOW_H
	struct spwd    *spw;
#endif
	const char     *user;
	int             pam_err;

	if ((pam_err = pam_get_user(pamh, &user, NULL)) != PAM_SUCCESS)
		return (pam_err);
	if (!(pwd = getpwnam(user))) {
		_pam_log(LOG_WARNING, "User [%s] either has a corrupted passwd entry or \
				is not in the selected database", user);
		return (PAM_AUTHTOK_RECOVERY_ERR);
	}
#ifdef HAVE_SHADOW_H
	if (!(spw = getspnam(user))) {
		_pam_log(LOG_WARNING, "User [%s] either has a corrupted passwd entry or \
				is not in the selected database", user);
		return PAM_USER_UNKNOWN;
	}
#endif
	/*
	 * When looking through the LinuxPAM code, I came across this : 
	 * 
	 * ` Various libraries at various times have had bugs related to
	 * '+' or '-' as the first character of a user name. Don't
	 * allow them. `
	 * 
	 * I don't know if the problem is still around but just in case...  
	 */
	if (user == NULL || user[0] == '-' || user[0] == '+') {
		_pam_log(LOG_WARNING, "Bad username [%s]", user);
		return (PAM_USER_UNKNOWN);
	}
	if (flags & PAM_PRELIM_CHECK) {
		/*- root doesn't need old passwd */
		if (!getuid())
			return (pam_set_item(pamh, PAM_OLDAUTHTOK, ""));
		if ((pam_err = pam_get_item(pamh, PAM_OLDAUTHTOK, (const void **) &old_pass)) != PAM_SUCCESS) {
			_pam_log(LOG_AUTHPRIV | LOG_ERR, PAM_MYSQL_LOG_PREFIX "PAM_OLDAUTHTOK-password (reason: %s)",
				   pam_strerror(pamh, pam_err));
			return (pam_err == PAM_PERM_DENIED ? PAM_AUTH_ERR : pam_err);
		}
		if (!old_pass && (pam_err = converse(pamh, flags, PLEASE_ENTER_OLD_PASSWORD, &old_pass)) != PAM_SUCCESS)
			return (pam_err == PAM_CONV_ERR ? pam_err : PAM_AUTH_ERR);
#ifdef HAVE_SHADOW_H
		hashedpwd = crypt(old_pass, spw->sp_pwdp);
#else
		hashedpwd = crypt(old_pass, pwd->pw_passwd);
#endif
		if (strcmp(hashedpwd, pwd->pw_passwd))
			return (PAM_PERM_DENIED);
	} else
	if (flags & PAM_UPDATE_AUTHTOK) {
		if ((pam_err = pam_get_item(pamh, PAM_OLDAUTHTOK, (const void **) &old_pass)) != PAM_SUCCESS) {
			_pam_log(LOG_AUTHPRIV | LOG_ERR, PAM_MYSQL_LOG_PREFIX "PAM_OLDAUTHTOK-password (reason: %s)",
				   pam_strerror(pamh, pam_err));
			return (pam_err == PAM_PERM_DENIED ? PAM_AUTH_ERR : pam_err);
		}
		if (!old_pass && (pam_err = converse(pamh, flags, PLEASE_ENTER_OLD_PASSWORD, &old_pass)) != PAM_SUCCESS)
			return (pam_err == PAM_CONV_ERR ? pam_err : PAM_AUTH_ERR);
		retries = 0;
		pam_err = PAM_AUTHTOK_ERR;
		while ((pam_err != PAM_SUCCESS) && (retries++ <= MAX_RETRIES)) {
			if ((pam_err = pam_get_item(pamh, PAM_AUTHTOK, (const void **) &new_pass)) != PAM_SUCCESS) {
				_pam_log(LOG_AUTHPRIV | LOG_ERR, PAM_MYSQL_LOG_PREFIX "PAM_AUTHTOK-password (reason: %s)",
					   pam_strerror(pamh, pam_err));
				return (pam_err == PAM_PERM_DENIED ? PAM_AUTH_ERR : pam_err);
			}
			if (!new_pass && (pam_err = converse(pamh, flags, PLEASE_ENTER_NEW_PASSWORD, &new_pass)) != PAM_SUCCESS)
				return (pam_err == PAM_CONV_ERR ? pam_err : PAM_AUTH_ERR);
			fprintf(stderr, "Unable to get new passwd. Please try again");
		}
		if (pam_err != PAM_SUCCESS) {
			_pam_log(LOG_ERR, "Unable to get new password!");
			return (pam_err);
		}
		/*
		 * checking has to be done (?) for the new passwd to 
		 * verify it's not weak. 
		 */
		makesalt(salt);
#ifdef HAVE_SHADOW_H
		/*- Update shadow/passwd entries for Linux */
		if ((pam_err = update_shadow(pamh, user, (const char *) sha512_crypt(new_pass, salt))) != PAM_SUCCESS) {
#ifdef DEBUG
			_pam_log(LOG_INFO, "failed sha512_crypt\n");
#endif
			pam_err = update_shadow(pamh, user, md5_crypt(new_pass, salt));
		}
		if (pam_err != PAM_SUCCESS)
			return (pam_err);
#endif
#ifdef WHY_IS_THIS_NEEDED
		if ((pam_err = update_passwd(pamh, user, "x")) != PAM_SUCCESS)
			return (pam_err);
#endif
	} else {
		pam_err = PAM_ABORT;
		_pam_log(LOG_ERR, "Unrecognized flags.");
		return (pam_err);
	}
	/*
	 * This code is yet to be completed
	 */
	return (PAM_SERVICE_ERR);
}

#ifdef HAVE_SHADOW_H
#define NEW_SHADOW "/etc/.shadow"
/*
 * Update shadow with new user password
 */
static int
update_shadow(pam_handle_t * pamh, const char *user, const char *newhashedpwd)
{
	FILE           *oldshadow, *newshadow;
	struct spwd    *pwd, *cur_pwd;
	struct stat     filestat;

	if ((pwd = getspnam(user)) == NULL)
		return PAM_USER_UNKNOWN;
	if ((oldshadow = fopen("/etc/shadow", "r")) == NULL) {
		fprintf(stderr, "Could not open /etc/shadow. Updating shadow database cancelled.");
		return (PAM_AUTHTOK_ERR);
	}

	if ((newshadow = fopen(NEW_SHADOW, "w")) == NULL) {
		fprintf(stderr, "Could not open temp file. Updating shadow database cancelled.");
		fclose(oldshadow);
		return (PAM_AUTHTOK_ERR);
	}

	if (fstat(fileno(oldshadow), &filestat) == -1) {
		fprintf(stderr, "Could not get stat for /etc/shadow. Updating shadow database cancelled.");
		fclose(oldshadow);
		fclose(newshadow);
		unlink(NEW_SHADOW);
		return (PAM_AUTHTOK_ERR);
	}

	if (fchown(fileno(newshadow), filestat.st_uid, filestat.st_gid) == -1) {
		fprintf(stderr, "Could not set uid/gid for new shadwow file. Updating shadow database cancelled.");
		fclose(oldshadow);
		fclose(newshadow);
		unlink(NEW_SHADOW);
		return (PAM_AUTHTOK_ERR);
	}

	if (fchmod(fileno(newshadow), filestat.st_mode) == -1) {
		fprintf(stderr, "Could not chmod for new shadow file. Updating shadow database cancelled.");
		fclose(oldshadow);
		fclose(newshadow);
		unlink(NEW_SHADOW);
		return (PAM_AUTHTOK_ERR);
	}
	while ((cur_pwd = fgetspent(oldshadow))) {
		if (strlen(user) == strlen(cur_pwd->sp_namp) && !strncmp(cur_pwd->sp_namp, user, strlen(user))) {
			cur_pwd->sp_pwdp = (char *) newhashedpwd;
			cur_pwd->sp_lstchg = time(NULL) / (60 * 60 * 24);
			_pam_log(LOG_ERR, "Updated password for user [%s]", user);
		}
		if (putspent(cur_pwd, newshadow)) {
			fprintf(stderr, "Error writing entry to new shadow file. Updating shadow database cancelled.");
			fclose(oldshadow);
			fclose(newshadow);
			unlink(NEW_SHADOW);
			return (PAM_AUTHTOK_ERR);
		}
	}
	fclose(oldshadow);
	if (fclose(newshadow)) {
		fprintf(stderr, "Error updating new shadow file.");
		unlink(NEW_SHADOW);
		return (PAM_AUTHTOK_ERR);
	}
	/*
	 * If program flow has come up to here, all is good
	 * and it's safe to update the shadow file.
	 */
	if (rename(NEW_SHADOW, "/etc/shadow") == 0)
		_pam_log(LOG_ERR, "Password updated successfully for user [%s]", user);
	else {
		fprintf(stderr, "Error updating shadow file.");
		unlink(NEW_SHADOW);
		return (PAM_AUTHTOK_ERR);
	}
	return (PAM_SUCCESS);
}
#endif

#ifdef WHY_IS_THIS_NEEDED
/*- Update /etc/passwd with new user information */
#define NEW_PASSWD "/etc/.passwd"

static int
update_passwd(pam_handle_t * pamh, const char *user, const char *newhashedpwd)
{
	FILE           *oldpasswd, *newpasswd;
	struct passwd  *pwd, *cur_pwd;
	struct stat     filestat;


	if ((pwd = getpwnam(user)) == NULL)
		return PAM_USER_UNKNOWN;
	if ((oldpasswd = fopen("/etc/passwd", "r")) == NULL) {
		fprintf(stderr, "Could not open /etc/passwd. Updating passwd database cancelled.");
		return (PAM_AUTHTOK_ERR);
	}
	if ((newpasswd = fopen(NEW_PASSWD, "w")) == NULL) {
		fprintf(stderr, "Could not open temp file. Updating passwd database cancelled.");
		fclose(oldpasswd);
		return (PAM_AUTHTOK_ERR);
	}
	if (fstat(fileno(oldpasswd), &filestat) == -1) {
		fprintf(stderr, "Could not get stat for /etc/passwd. Updating passwd database cancelled.");
		fclose(oldpasswd);
		fclose(newpasswd);
		unlink(NEW_PASSWD);
		return (PAM_AUTHTOK_ERR);
	}
	if (fchown(fileno(newpasswd), filestat.st_uid, filestat.st_gid) == -1) {
		fprintf(stderr, "Could not set uid/gid for new shadwow file. Updating passwd database cancelled.");
		fclose(oldpasswd);
		fclose(newpasswd);
		unlink(NEW_PASSWD);
		return (PAM_AUTHTOK_ERR);
	}
	if (fchmod(fileno(newpasswd), filestat.st_mode) == -1) {
		fprintf(stderr, "Could not chmod for new passwd file. Updating passwd database cancelled.");
		fclose(oldpasswd);
		fclose(newpasswd);
		unlink(NEW_PASSWD);
		return (PAM_AUTHTOK_ERR);
	}
	while ((cur_pwd = fgetpwent(oldpasswd))) {
		if (strlen(user) == strlen(cur_pwd->pw_name) && !strncmp(cur_pwd->pw_name, user, strlen(user))) {
			cur_pwd->pw_passwd = newhashedpwd;
			_pam_log(LOG_ERR, "Updated password for user [%s]", user);
		}
		if (putpwent(cur_pwd, newpasswd)) {
			fprintf(stderr, "Error writing entry to new passwd file. Updating passwd database cancelled.");
			fclose(oldpasswd);
			fclose(newpasswd);
			unlink(NEW_PASSWD);
			return (PAM_AUTHTOK_ERR);
		}
	}
	fclose(oldpasswd);
	if (fclose(newpasswd)) {
		fprintf(stderr, "Error updating new passwd file.");
		unlink(NEW_PASSWD);
		return (PAM_AUTHTOK_ERR);
	}
	/*
	 * If program flow has come up to here, all is good
	 * and it's safe to update the passwd file.
	 */
	if (rename(NEW_PASSWD, "/etc/passwd") == 0)
		_pam_log(LOG_ERR, "Password updated successfully for user [%s]", user);
	else {
		fprintf(stderr, "Error updating passwd file.");
		unlink(NEW_PASSWD);
		return (PAM_AUTHTOK_ERR);
	}
	return (PAM_SUCCESS);
}
#endif

void
getversion_pam_multi()
{
	printf("%s\n", sccsid);
}

#ifdef PAM_STATIC
/*- static module data */
struct pam_module _pam_multi = {
	PAM_MODULE_NAME,
	pam_sm_authenticate,
	pam_sm_setcred,
	pam_sm_acct_mgmt,
	pam_sm_open_session,
	pam_sm_close_session,
	pam_sm_chauthtok
};
#endif

/*
 * $Log: pam-multi.c,v $
 * Revision 1.22  2025-01-22 16:04:09+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.21  2023-11-13 10:26:36+05:30  Cprogrammer
 * include crypt.h
 *
 * Revision 1.20  2021-07-18 08:27:16+05:30  Cprogrammer
 * fixed salt size
 *
 * Revision 1.19  2020-10-04 09:21:32+05:30  Cprogrammer
 * set optreset=1 for Darwin
 *
 * Revision 1.18  2020-10-03 12:29:44+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.17  2020-09-29 11:07:05+05:30  Cprogrammer
 * replaced LOG_EMERG with LOG_INFO
 * changed/added debug statements
 *
 * Revision 1.16  2020-09-23 11:01:37+05:30  Cprogrammer
 * fold braces for readability
 *
 * Revision 1.15  2020-09-22 00:17:52+05:30  Cprogrammer
 * FreeBSD port
 *
 * Revision 1.14  2019-07-03 19:35:49+05:30  Cprogrammer
 * load libmysqlclient/libmariadb dynamically using load_mysql.c
 *
 * Revision 1.13  2018-09-12 12:55:04+05:30  Cprogrammer
 * fixed SIGSEGV in _pam_log()
 *
 * Revision 1.12  2010-05-05 20:13:41+05:30  Cprogrammer
 * use environment variable AUTHSERVICE for service identifier
 *
 * Revision 1.11  2010-04-10 14:50:16+05:30  Cprogrammer
 * changed all units to seconds to avoid mistakes during comparision
 *
 * Revision 1.10  2009-10-17 20:14:09+05:30  Cprogrammer
 * duplicate definition of pam_err removed
 *
 * Revision 1.9  2009-10-17 16:53:45+05:30  Cprogrammer
 * fix for DARWIN
 *
 * Revision 1.8  2009-10-12 08:33:41+05:30  Cprogrammer
 * rearranged functions
 * completed run_command() function
 *
 * Revision 1.7  2009-10-11 11:34:57+05:30  Cprogrammer
 * prevent doS
 *
 * Revision 1.6  2009-10-11 09:38:51+05:30  Cprogrammer
 * completed acct_mgmt
 *
 * Revision 1.5  2009-10-08 16:53:31+05:30  Cprogrammer
 * added code for account management
 *
 * Revision 1.4  2009-10-07 11:51:38+05:30  Cprogrammer
 * added option 'D' to use non-escaped mysql string
 *
 * Revision 1.3  2009-10-07 09:59:18+05:30  Cprogrammer
 * initialize optind
 *
 * Revision 1.2  2009-10-06 22:23:46+05:30  Cprogrammer
 * use config.h to include correct pam header files
 *
 * Revision 1.1  2009-10-06 12:01:31+05:30  Cprogrammer
 * Initial revision
 */
