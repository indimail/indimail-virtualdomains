/*
 * $Log: systpass.c,v $
 * Revision 1.2  2019-07-10 12:57:59+05:30  Cprogrammer
 * print more error information in print_error
 *
 * Revision 1.1  2019-04-18 08:36:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAS_SHADOW
#include <shadow.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <error.h>
#include <env.h>
#endif
#include "common.h"
#include "pw_comp.h"
#include "pipe_exec.h"
#include "runcmmd.h"

#ifndef lint
static char     sccsid[] = "$Id: systpass.c,v 1.2 2019-07-10 12:57:59+05:30 Cprogrammer Exp mbhangui $";
#endif

int             authlen = 512;

void
print_error(char *str)
{
	out("syspass", "454-");
	out("vchkpass", str);
	out("vchkpass", ": ");
	out("syspass", error_str(errno));
	out("syspass", " (#4.3.0)\r\n");
	flush("syspass");
}

#ifdef ENABLE_PASSWD
int
main(int argc, char **argv)
{
	char           *ptr, *tmpbuf, *login, *response, *challenge, *stored;
	char            strnum[FMT_ULONG];
	static stralloc buf = {0};
	int             i, count, offset, status;
	struct passwd  *pw;
#ifdef HAS_SHADOW
	struct spwd    *spw;
#endif

	if (argc < 2)
		_exit(2);
	if (!(tmpbuf = alloc((authlen + 1) * sizeof(char)))) {
		print_error("out of memory");
		strnum[fmt_uint(strnum, (unsigned int) authlen + 1)] = 0;
		strerr_warn3("alloc-", strnum, ": ", &strerr_sys);
		_exit(111);
	}
	for (offset = 0;;) {
		do
		{
			count = read(3, tmpbuf + offset, authlen + 1 - offset);
#ifdef ERESTART
		} while(count == -1 && (errno == EINTR || errno == ERESTART));
#else
		} while(count == -1 && errno == EINTR);
#endif
		if (count == -1) {
			print_error("read error");
			strerr_warn1("syspass: read: ", &strerr_sys);
			_exit(111);
		}
		else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
			_exit(2);
	}
	count = 0;
	login = tmpbuf + count; /*- username */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	count++;
	challenge = tmpbuf + count; /*- challenge */
	for (;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset)
		_exit(2);
	response = tmpbuf + count + 1; /*- response */
	i = str_chr(login, '@');
	if (login[i])
		login[i] = 0;
	if (!(pw = getpwnam(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset);
		else
			strerr_warn1("syspass: getpwnam: ", &strerr_sys);
		print_error("getpwnam");
		_exit (111);
	}
	stored = pw->pw_passwd;
#ifdef HAS_SHADOW
	if (!(spw = getspnam(login))) {
		if (errno != ETXTBSY)
			pipe_exec(argv, tmpbuf, offset);
		else
			strerr_warn1("syspass: getspnam: ", &strerr_sys);
		print_error("getspnam");
		_exit (111);
	}
	stored = spw->sp_pwdp;
#endif
#ifdef DEBUG
	fprintf(stderr, "%s: login [%s] challenge [%s] response [%s] pw_passwd [%s]\n", 
		argv[0], login, challenge, response, stored);
#endif
	if (pw_comp((unsigned char *) login, (unsigned char *) stored,
		(unsigned char *) (*response ? challenge : 0),
		(unsigned char *) (*response ? response : challenge), 0))
	{
		pipe_exec(argv, tmpbuf, offset);
		print_error("exec");
		_exit (111);
	}
	status = 0;
	if ((ptr = (char *) env_get("POSTAUTH")) && !access(ptr, X_OK)) {
		if (stralloc_copys(&buf, ptr) ||
				!stralloc_append(&buf, " ") ||
				!stralloc_cats(&buf, login) ||
				!stralloc_0(&buf)) {
			strerr_warn1("syspass: out of memory: ", &strerr_sys);
			_exit (111);
		}
		status = runcmmd(buf.s, 0);
	}
	_exit(status);
	/*- Not reached */
	return (0);
}
#else
int
main(int argc, char **argv)
{
	if (argc < 2)
		_exit(2);
	execvp(argv[1], argv + 1);
	strerr_warn1("syspass: ", argv[1], ": ", &strerr_sys);
	print_error("exec");
	_exit(111);
	/*- Not reached */
	return (0);
}
#endif
