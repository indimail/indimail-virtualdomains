/*
 * $Log: sq_vacation.c,v $
 * Revision 1.2  2019-04-22 23:18:46+05:30  Cprogrammer
 * replaced exit with _exit
 *
 * Revision 1.1  2019-04-15 11:58:16+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <env.h>
#include <substdio.h>
#include <getln.h>
#include <subfd.h>
#include <str.h>
#endif
#include "indimail.h"
#include "variables.h"
#include "iopen.h"
#include "iclose.h"
#include "sqlOpen_user.h"
#include "inquery.h"
#include "pw_comp.h"
#include "fappend.h"
#include "get_assign.h"
#include "getEnvConfig.h"
#include "get_real_domain.h"
#include "sql_getpw.h"

#define SERVER 1
#define USER   2
#define ACTION 3
#define SRC    4
#define DEST   5

#define BUFSIZE 512

/*- Exit status */
#define ERR_OK          0       /*- no error */
#define ERR_NOTFOUND    1       /*- file not found */
#define ERR_BADPASS     32      /*- bad password */
#define ERR_USAGE       33      /*- usage error */
#define ERR_RESTRICTED  34      /*- not allowed to use this program */
#define ERR_REMOTEFILE  35      /*- illegal remote filename */
#define ERR_LOCALFILE   36      /*- illegal local filename */
#define ERR_CONFIG      37      /*- global configuration problem */
#define ERR_USER        38      /*- problem with this user */
#define ERR_HOME        39      /*- problem accessing home directory */
#define ERR_SOURCEFILE  40      /*- problem opening/stat()ing source file */
#define ERR_DESTFILE    41      /*- problem opening/deleting dest file */
#define ERR_COPYFILE    42      /*- problem copying file contents */
#define ERR_UNLINK      43      /*- problem unlinking file */
#define ERR_FILETYPE    44      /*- not a regular file */
#define ERR_EXEC        45      /*- exec() of vacation program failed */
#define ERR_NOTSUPPORTED 46     /*- feature not enabled */
#define ERR_AUTO_MSG    47      /*- error create .vacation.sq */
#define ERR_PRIVILEGE   125     /*- unexpected privileges problem */
#define ERR_UNEXPECTED  126     /*- other unexpected error */

#ifndef lint
static char     sccsid[] = "$Id: sq_vacation.c,v 1.2 2019-04-22 23:18:46+05:30 Cprogrammer Exp mbhangui $";
#endif

#define REMOTEFILE_OKCHARS \
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.+-_"
static uid_t orig_uid = 0;
static gid_t orig_gid = 0;

#ifdef HAVE_STDARG_H
#include <stdarg.h>
void
die(int status, const char *fmt, ...)
#else
#include <varargs.h>
void
die(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char            buf[2048];
#ifndef HAVE_STDARG_H
	char           *fmt;
	int             status;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	status = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	strerr_warn1(buf, 0);
	_exit(status);
}

#ifdef HAVE_STDARG_H
#include <stdarg.h>
void
die_sys(int status, const char *fmt, ...)
#else
#include <varargs.h>
void
die(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char            buf[2048];
#ifndef HAVE_STDARG_H
	char           *fmt;
	int             status;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	status = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	strerr_warn1(buf, &strerr_sys);
	_exit(status);
}

void
remoteok(char *rmtfile, char *desc)
{
	char *ptr;
	int len;
	if (!rmtfile || !*rmtfile)
		die(ERR_REMOTEFILE, "%s: Remote filename cannot be empty", desc);
	for (len = str_len(REMOTEFILE_OKCHARS), ptr = rmtfile; *ptr; ptr++) {
		if (str_chr(REMOTEFILE_OKCHARS, *ptr) == len)
			die(ERR_REMOTEFILE, "%s: %s: Remote filename contains illegal character(s)", desc, rmtfile);
	}
	if (str_str(rmtfile, ".."))
		die(ERR_REMOTEFILE, "%s: %s: Remote filename cannot have ..", desc);
}

void
localok(char *localfile, char *desc)
{
	if (*localfile != '/')
		die(ERR_LOCALFILE, "%s: %s: must be absolute pathname", desc, localfile);
}

int
do_list(char *src, struct passwd *pw)
{
	struct stat     statbuf;

	if (!src || !*src)
		return ERR_OK;
	remoteok(src, "do_list"); /*- in home directory of user */
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
	if (!stat(src, &statbuf))
	{
		if (!S_ISREG(statbuf.st_mode))
			die(ERR_FILETYPE, "%s: target is not a regular file", src);
	} else
	if (errno == ENOENT)
		return ERR_NOTFOUND;
	else
		die_sys(ERR_SOURCEFILE, "stat: %s", src);
	return ERR_OK;
}

int
do_get(char *src, char *dest, struct passwd *pw)
{
	remoteok(src, "do_get"); /*- in home directory */
	localok(dest, "do_get");
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
#if 0
	if (set_eugid(orig_uid, orig_gid) != 0) {
		die_sys(ERR_PRIVILEGE, "setuid(ruid)\n");
#endif
	if (fappend(src, dest, "w", INDIMAIL_QMAIL_MODE, orig_uid, orig_gid)) {
		if (errno == ENOENT)
			return ERR_NOTFOUND;
		if (access(src, R_OK))
			die_sys(ERR_SOURCEFILE, "access %s", src);
		die_sys(ERR_DESTFILE, "fappend %s", dest);
	}
	return ERR_OK;
}

int
do_put(char *src, char *dest, struct passwd *pw, uid_t uid, gid_t gid)
{
	localok(src, "do_put");
	remoteok(dest, "do_put"); /*- in home directory of user */
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
	if (fappend(src, dest, "w", INDIMAIL_QMAIL_MODE, uid, gid))
		die_sys(ERR_DESTFILE, "fappend %s", dest);
	return ERR_OK;
}

int
do_delete(char *src, struct passwd *pw)
{
	struct stat     statbuf;

	remoteok(src, "do_delete");
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
	if (!lstat(src, &statbuf)) {
		if (S_ISREG(statbuf.st_mode)
#ifdef S_ISLNK
			|| S_ISLNK(statbuf.st_mode)
#endif /* S_ISLNK */
	) {
			if (unlink(src))
				die_sys(ERR_UNLINK, "unlink %s", src);
		} else
			die(ERR_FILETYPE, "%s: delete: not a regular file", src);
	} else
	if (errno == ENOENT)
		return ERR_NOTFOUND;
	else
		die_sys(ERR_DESTFILE, "lstat %s", src);
	return ERR_OK;
}

void
close_connection()
{
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE"))
		iclose();
#else /*- Not QUERY_CACHE */
	iclose();
#endif
}

int
set_eugid(uid_t uid, gid_t gid)
{
	/* need euid=root to set ids */
	if (setreuid(0, 0))
		return -1;
	if (setegid(gid))
		return -1;
	if (setreuid(0, uid))
		return -1;
	return 0;
}

static void
die_nomem()
{
	strerr_warn1("sq_vacation: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *email = 0, *action, *ptr, *cptr, *real_domain, *crypt_pass;
	static stralloc User = {0}, Domain = {0}, passbuf = {0};
	char           *(vacargs[5]);
	int             i, fd, status = -1, match;
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;

	if (argc < ACTION + 1)
		die(ERR_USAGE, "Incorrect usage");
	if (getln(subfdinsmall, &passbuf, &match, '\n') == -1)
		die(ERR_BADPASS, "Could not read password");
	if (!match)
		die(ERR_BADPASS, "Could not read password");
	passbuf.len--;
	passbuf.s[passbuf.len] = 0;
	email = argv[USER];
	action = argv[ACTION];
	orig_uid = getuid();
	orig_gid = getgid();
	for (i = 0, ptr = email; *ptr && *ptr != '@'; i++, ptr++);
	if (!stralloc_copyb(&User, email, i) || !stralloc_0(&User))
		die_nomem();
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	cptr = ptr;
	for (i = 0; *cptr; i++, cptr++);
	if (!stralloc_copyb(&Domain, ptr, i) || !stralloc_0(&Domain))
		die_nomem();
#ifdef QUERY_CACHE
	if (!env_get("QUERY_CACHE")) {
#ifdef CLUSTERED_SITE
		if (sqlOpen_user(email, 0))
#else
		if (iopen((char *) 0))
#endif
			die(ERR_UNEXPECTED, "sq_vacation: unable to connect to IndiMail database");
	}
#else
#ifdef CLUSTERED_SITE
	if (sqlOpen_user(email, 0))
		die(ERR_UNEXPECTED, "sqlOpen_user: unable to connect to IndiMail database");
#else
	if (iopen((char *) 0))
		die(ERR_UNEXPECTED, "iopen: unable to connect to IndiMail database");
#endif
#endif
#ifdef QUERY_CACHE
	if (env_get("QUERY_CACHE")) {
		pw = inquery(PWD_QUERY, email, 0);
		if (!get_assign(Domain.s, 0, &uid, &gid)) 
			die(ERR_USER, "%s: domain does not exist", Domain.s);
	} else {
		if (!get_assign(Domain.s, 0, &uid, &gid)) 
			die(ERR_USER, "%s: domain does not exist", Domain.s);
		if (!(real_domain = get_real_domain(Domain.s)))
			real_domain = Domain.s;
		pw = sql_getpw(User.s, real_domain);
	}
#else
	if (!get_assign(Domain.s, 0, &uid, &gid)) 
		die(ERR_USER, "%s: domain does not exist", Domain.s);
	if (!(real_domain = get_real_domain(Domain.s)))
		real_domain = Domain.s;
	pw = sql_getpw(User.s, real_domain);
#endif
	if (!pw) {
		if(userNotFound)
			die(ERR_USER, "%s@%s: user not found", User.s, Domain.s);
		else
			die(ERR_UNEXPECTED, "inquery failed");
		close_connection();
		die(ERR_USER, "failed to get pw info for %s@%s", User.s, Domain.s);
	}
	crypt_pass = pw->pw_passwd;
	if (pw_comp((unsigned char *) email, (unsigned char *) crypt_pass,
		0, (unsigned char *) passbuf.s, 0))
		die(ERR_BADPASS, "Password does not match");
	if (!str_diffn(action, "list", 4)) {
		if (argc != SRC + 1)
			die(ERR_USAGE, "Incorrect usage for list");
		status = do_list(argv[SRC], pw);
	} else
	if (!str_diffn(action, "get", 3)) {
		if (argc != DEST + 1)
			die(ERR_USAGE, "Incorrect usage for get");
		status = do_get(argv[SRC], argv[DEST], pw);
	} else
	if (!str_diffn(action, "put", 3)) {
		if (argc != DEST + 1)
			die(ERR_USAGE, "Incorrect usage for put");
		status = do_put(argv[SRC], argv[DEST], pw, uid, gid);
	} else
	if (!str_diffn(action, "delete", 6)) {
		if (argc != SRC + 1)
			die(ERR_USAGE, "Incorrect usage for delete");
		status = do_delete(argv[SRC], pw);
		if (setgid(gid))
			die_sys(ERR_PRIVILEGE, "setgid %d", gid);
		else
		if (setuid(uid))
			die_sys(ERR_PRIVILEGE, "setuid %d", uid);
		vacargs[0] = PREFIX"/bin/vmoduser";
		vacargs[1] = "-l";
		vacargs[2] = "-"; /*- remove autoresponder */
		vacargs[3] = email;
		vacargs[4] = 0;
		execv(*vacargs, vacargs);
		die_sys(ERR_EXEC, "execv: %s/bin/vmoduser -l- %s", PREFIX, email);
		_exit(ERR_EXEC);
	} else
	if (!str_diffn(action, "init", 4)) {
		if (argc != ACTION + 1 || !email)
			die(ERR_USAGE, "Incorrect usage for init");
		if (chdir(pw->pw_dir))
			die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
		if (setgid(gid))
			die_sys(ERR_PRIVILEGE, "setgid %d", gid);
		else
		if (setuid(uid))
			die_sys(ERR_PRIVILEGE, "setuid %d", uid);
		if ((fd = open(".vacation.msg", O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR)) == -1)
			die_sys(ERR_AUTO_MSG, ".vacation.msg");
		close(fd);
		vacargs[0] = PREFIX"/bin/vmoduser";
		vacargs[1] = "-l";
		vacargs[2] = ".vacation.msg";
		vacargs[3] = email;
		vacargs[4] = 0;
		execv(*vacargs, vacargs);
		die_sys(ERR_EXEC, "execv: %s/bin/vmoduser -l .vacation.msg %s", PREFIX, email);
	}
	_exit(status);
}
