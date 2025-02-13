/*
 * $Id: pam-checkpwd.c,v 1.15 2025-01-22 16:04:00+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "pam-support.h"

#ifndef MAX_BUFF
#define MAX_BUFF 512
#endif

#define isEscape(ch) ((ch) == '"' || (ch) == '\'')

#ifndef lint
static char     sccsid[] = "$Id: pam-checkpwd.c,v 1.15 2025-01-22 16:04:00+05:30 Cprogrammer Exp mbhangui $";
#endif

int             authlen = 512;
static const char     *short_options = "dehs:i:HV";

static struct option long_options[] = {
	{"debug", no_argument, NULL, 'd'},
	{"help", no_argument, NULL, 'h'},
	{"noenv", no_argument, NULL, 'e'},
	{"no-chdir-home", no_argument, NULL, 'H'},
	{"service", required_argument, NULL, 's'},
	{"ident", required_argument, NULL, 'i'},
	{"version", no_argument, NULL, 'V'},
	{NULL, 0, NULL, 0}
};

static const char *usage =
	"Usage: pam-checkpwd [OPTION]... -- prog...\n"
	"\n"
	"Authenticate using PAM and the checkpassword protocol:\n"
	"\t<URL:http://cr.yp.to/checkpwd/interface.html>\n"
	"and run the program specified as 'prog'\n"
	"\n"
	"Options are:\n"
	"  -d, --debug\t\tturn on debugging output\n"
	"  -h, --help\t\tdisplay this help and exit\n"
	"  -e, --noenv\t\tdo not set uid, gid, environment variables,\n"
	"\t\t\tand home directory\n"
	"  -H, --no-chdir-home\tdo not change to home directory\n"
	"  -s, --service=SERVICE\tspecify PAM service name to use\n"
	"\t\t\t(by default use the contents of $PAM_SERVICE)\n"
	"  -i, --ident=ident\tspecify an identifier to use\n"
	"  -V, --version\t\tdisplay version information and exit\n";

int
initialize(char *username, int opt_dont_chdir_home, int debug)
{
	struct passwd  *pw;
	/*- switch to proper uid/gid/groups */
	if (!(pw = getpwnam(username))) {
		if (debug)
			fprintf(stderr, "pam-checkpwd: Error getting information about %s from /etc/passwd: %s\n", username, strerror(errno));
		_exit(111);
	}
	/*- set supplementary groups */
	if (initgroups(username, pw->pw_gid) == -1) {
		fprintf(stderr, "pam-checkpwd: Error setting supplementary groups for user %s: %s\n", username, strerror(errno));
		_exit(111);
	}
	/*- set gid */
	if (setgid(pw->pw_gid) == -1) {
		fprintf(stderr, "pam-checkpwd: setgid(%d) error: %s\n", pw->pw_gid, strerror(errno));
		_exit(111);
	}
	/*- set uid */
	if (setuid(pw->pw_uid) == -1) {
		fprintf(stderr, "pam-checkpwd: setuid(%d) error: %s\n", pw->pw_uid, strerror(errno));
		_exit(111);
	}
	/*- switch to user home directory */
	if (!opt_dont_chdir_home) {
		if (chdir(pw->pw_dir) == -1) {
			fprintf(stderr, "pam-checkpwd: Error changing directory %s: %s\n", pw->pw_dir, strerror(errno));
			_exit(111);
		}
	}
	/*- set $USER */
	if (setenv("USER", username, 1) == -1) {
		fprintf(stderr, "pam-checkpwd: Error setting $USER to %s: %s\n", username, strerror(errno));
		_exit(111);
	}
	/*- set $HOME */
	if (setenv("HOME", pw->pw_dir, 1) == -1) {
		fprintf(stderr, "pam-checkpwd: Error setting $HOME to %s: %s\n", pw->pw_dir, strerror(errno));
		_exit(111);
	}
	/*- set $SHELL */
	if (setenv("SHELL", pw->pw_shell, 1) == -1) {
		fprintf(stderr, "pam-checkpwd: Error setting $SHELL to %s: %s\n", pw->pw_shell, strerror(errno));
		_exit(111);
	}
	return (0);
}

/*
 * function to expand a string into command line
 * arguments. To free memory allocated by this
 * function the following should be done
 *
 * free(argv); free(argv[0]);
 *
 */
char          **
MakeArgs(char *cmmd)
{
	char           *ptr, *sptr, *marker;
	char          **argv;
	int             argc, idx;

	for (ptr = cmmd;*ptr && isspace((int) *ptr);ptr++);
	idx = strlen(ptr);
	if (!(sptr = (char *) malloc((idx + 1) * sizeof(char))))
		return((char **) 0);
	strcpy(sptr, ptr);
	/*-
	 * Get the number of arguments by counting
	 * white spaces. Allow escape via the double
	 * quotes character at the first word
	 */
	for (argc = 0, ptr = sptr;*ptr;) {
		for (;*ptr && isspace((int) *ptr);ptr++);
		if (!*ptr)
			break;
		argc++;
		marker = ptr;
		for (;*ptr && !isspace((int) *ptr);ptr++) {
			if (ptr == marker && isEscape(*ptr)) {
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
			}
		}
	}
#ifdef DEBUG
	printf("argc = %d\n", argc);
#endif
	/*
	 * Allocate memory to store the arguments
	 * Do not bother extra bytes occupied by
	 * white space characters.
	 */
	if (!(argv = (char **) malloc((argc + 1) * sizeof(char *))))
		return ((char **) NULL);
	for (idx = 0, ptr = sptr;*ptr;) {
		for (;*ptr && isspace((int) *ptr);ptr++)
			*ptr = 0;
		if (!*ptr)
			break;
		if (*ptr == '$')
			argv[idx++] = getenv(ptr + 1);
		else
			argv[idx++] = ptr;
		marker = ptr;
		for (;*ptr && !isspace((int) *ptr);ptr++) {
			if (ptr == marker && isEscape(*ptr)) {
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
				else {/*- Remove the quotes */
					argv[idx - 1] += 1;
					*ptr = 0;
				}
			}
		}
	}
	argv[idx++] = (char *) 0;
#ifdef DEBUG
	for (idx = 0;idx <= argc;idx++)
		printf("argv[%d] = [%s]\n", idx, argv[idx]);
#endif
	return (argv);
}

static 
int pipe_exec(char **argv, char *tmpbuf, int len)
{
	int             pipe_fd[2];

	if (getenv("DEBUG"))
		fprintf(stderr, "pam-checkpwd: pid [%d] executing authmodule [%s]\n", getpid(), argv[0]);
	if (pipe(pipe_fd) == -1) {
		fprintf(stderr, "pam-checkpwd: pipe_exec: pipe: %s\n", strerror(errno));
		return(-1);
	}
	if (dup2(pipe_fd[0], 3) == -1 || dup2(pipe_fd[1], 4) == -1) {
		fprintf(stderr, "pam-checkpwd: pipe_exec: dup2: %s\n", strerror(errno));
		return(-1);
	}
	if (pipe_fd[0] != 3 && pipe_fd[0] != 4)
		close(pipe_fd[0]);
	if (pipe_fd[1] != 3 && pipe_fd[1] != 4)
		close(pipe_fd[1]);
	if (write(4, tmpbuf, len) != len) {
		fprintf(stderr, "pam-checkpwd: pipe_exec: %s: %s\n", argv[0], strerror(errno));
		return(-1);
	}
	close(4);
	execvp(argv[0], argv);
	fprintf(stderr, "pam-checkpwd: pipe_exec: %s: %s\n", argv[0], strerror(errno));
	return(-1);
}

static int
runcmmd(char *cmmd, int useP, int debug)
{
	char          **argv;
	int             status, i, retval;
	pid_t           pid;
	void            (*pstat[2]) (int);

	switch ((pid = fork()))
	{
	case -1:
		_exit(1);
	case 0:
		if (!(argv = MakeArgs(cmmd)))
			_exit(1);
		if (useP)
			execvp(*argv, argv);
		else
			execv(*argv, argv);
		perror(*argv);
		_exit(1);
	default:
		break;
	}
	if ((pstat[0] = signal(SIGINT, SIG_IGN)) == SIG_ERR)
		return (-1);
	else
	if ((pstat[1] = signal(SIGQUIT, SIG_IGN)) == SIG_ERR)
		return (-1);
	for (retval = -1;;) {
		i = wait(&status);
#ifdef ERESTART
		if (i == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (i == -1 && errno == EINTR)
#endif
			continue;
		else
		if (i == -1)
			break;
		if (i != pid)
			continue;
		if (WIFSTOPPED(status) || WIFSIGNALED(status)) {
			if (debug)
				fprintf(stderr, "%d: killed by signal %d\n", getpid(), WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status));
			retval = -1;
		} else
		if (WIFEXITED(status)) {
			retval = WEXITSTATUS(status);
			if (debug)
				fprintf(stderr, "%d: normal exit return status %d\n", getpid(), retval);
		}
		break;
	}
	(void) signal(SIGINT, pstat[0]);
	(void) signal(SIGQUIT, pstat[1]);
	return (retval);
}

int
main(int argc, char **argv)
{
	char           *ptr, *tmpbuf, *login, *response, *challenge;
	char            buf[MAX_BUFF];
	int             opt_dont_set_env = 0, opt_dont_chdir_home = 0,
					debug = 0, c, count, offset, status, option_index = 0,
					native_checkpassword = 0, s_optind;
	char           *service_name = 0;
	struct passwd  *pw;

	if (argc < 2)
		_exit(2);
	/*- process command line options */
	s_optind = optind = opterr = 0;
	while (1) {
		if ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) == -1)
			break;
		switch (c) {
		case 'd':
			debug = 1;
			break;
		case 'H':
			opt_dont_chdir_home = 1;
			break;
		case 'e':
			opt_dont_set_env = 1;
			break;
		case 'h':
			fprintf(stderr, "%s", usage);
			_exit(0);
		case 's':
			service_name = optarg;
			break;
		case 'i':
			if (setenv("AUTHSERVICE", optarg, 1) == -1) {
				fprintf(stderr, "pam-checkpwd: Error setting $AUTHSERVICE to %s: %s\n",
					optarg, strerror(errno));
				_exit(111);
			}
			break;
		case 'V':
			fprintf(stderr, "%s: %s\n", PACKAGE, VERSION);
			_exit(2);
		case '?':
			fprintf(stderr, "pam-checkpwd: Invalid command line, see --help\n");
			_exit(2);
		}
	}
	s_optind = optind;
	if (optind >= argc) {
		fprintf(stderr, "pam-checkpwd: Expected argument after options\n");
		_exit(2);
	}
	if (!service_name && !(service_name = getenv("PAM_SERVICE"))) {
		fprintf(stderr, "pam-checkpwd: PAM service name not specified\n");
		_exit(2);
	}
	if (!(tmpbuf = calloc(1, (authlen + 1) * sizeof(char)))) {
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		fprintf(stderr, "pam-checkpwd: malloc-%d: %s\n", authlen + 1, strerror(errno));
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
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			fprintf(stderr, "pam-checkpwd: read: %s\n", strerror(errno));
			_exit(111);
		}
		else
		if (!count)
			break;
		offset += count;
		if (offset >= (authlen + 1))
			_exit(2);
	}
	if (debug)
		fprintf(stderr, "pam-checkpwd: read %d bytes\n", offset);
	count = 0;
	login = tmpbuf + count; /*- username */
	for(;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset) {
		if (debug)
			fprintf(stderr, "pam-checkpwd: no username\n");
		_exit(2);
	}
	count++;
	challenge = tmpbuf + count; /*- challenge */
	for(;tmpbuf[count] && count < offset;count++);
	if (count == offset || (count + 1) == offset) {
		if (debug)
			fprintf(stderr, "pam-checkpwd: no challenge\n");
		_exit(2);
	}
	response = tmpbuf + count + 1; /*- response */
	if (debug)
		fprintf(stderr, "%s: pid [%d] login [%s] challenge [%s] response [%s]\n", 
			argv[0], getpid(), login, challenge, response);
	if (!opt_dont_set_env) {
		if (unsetenv("HOME") || unsetenv("SHELL") || unsetenv("USER")) {
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
	}
	native_checkpassword = (getenv("NATIVE_CHECKPASSWORD") || getenv("DOVECOT_VERSION")) ? 1 : 0;
	if (native_checkpassword) {
		if (unsetenv("userdb_uid") || unsetenv("userdb_gid") ||
				unsetenv("EXTRA")) {
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
	}
	/*- authenticate using PAM */
	if ((status = auth_pam(service_name, login, challenge, debug))) {
		native_checkpassword ? _exit (1) : pipe_exec(argv + s_optind, tmpbuf, offset);
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	if (!opt_dont_set_env)
		initialize(login, opt_dont_chdir_home, debug);
	if ((ptr = (char *) getenv("POSTAUTH")) && !access(ptr, X_OK)) {
		snprintf(buf, MAX_BUFF, "%s %s", ptr, login);
		status = runcmmd(buf, 1, debug);
	}
	if (native_checkpassword) { /*- support dovecot checkpassword */
		if (setenv("userdb_uid", "indimail", 1) || setenv("userdb_gid", "indimail", 1)) {
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
		/*- dovecot requires HOME */
		if (opt_dont_set_env) {
			if (!(pw = getpwnam(login))) {
				if (debug)
					fprintf(stderr, "pam-checkpwd: /etc/passwd: %s: %s\n", login, strerror(errno));
				printf("454-%s (#4.3.0)\r\n", strerror(errno));
				fflush(stdout);
				_exit(111);
			}
			if (setenv("HOME", pw->pw_dir, 1) == -1) {
				printf("454-%s (#4.3.0)\r\n", strerror(errno));
				fflush(stdout);
				_exit(111);
			}
		}
		if ((ptr = getenv("EXTRA")))
			snprintf(buf, MAX_BUFF, "userdb_uid userdb_gid %s", ptr);
		else
			snprintf(buf, MAX_BUFF, "userdb_uid userdb_gid");
		if (setenv("EXTRA", buf, 1)) {
			printf("454-%s (#4.3.0)\r\n", strerror(errno));
			fflush(stdout);
			_exit (111);
		}
		execv(argv[1], argv + 1);
		printf("454-%s (#4.3.0)\r\n", strerror(errno));
		fflush(stdout);
		_exit (111);
	}
	_exit(status);
	/*- Not reached */
	return(0);
}

void
getversion_checkpassword_pam_c()
{
	printf("%s\n", sccsid);
}

/*
 * $Log: pam-checkpwd.c,v $
 * Revision 1.15  2025-01-22 16:04:00+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.14  2021-01-27 18:47:28+05:30  Cprogrammer
 * renamed use_dovecot to native_checkpassword
 *
 * Revision 1.13  2021-01-27 16:51:36+05:30  Cprogrammer
 * set HOME for dovecot
 *
 * Revision 1.12  2021-01-27 13:28:02+05:30  Cprogrammer
 * dovecot support added
 *
 * Revision 1.11  2020-09-28 13:33:09+05:30  Cprogrammer
 * added pid in debug statements
 *
 * Revision 1.10  2020-09-28 12:46:38+05:30  Cprogrammer
 * put authmodule name in error logs
 *
 * Revision 1.9  2018-09-12 21:14:32+05:30  Cprogrammer
 * call initialize post successful auth
 *
 * Revision 1.8  2018-09-12 18:50:30+05:30  Cprogrammer
 * coded indented
 *
 * Revision 1.7  2018-09-11 10:14:14+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.6  2010-05-05 20:13:23+05:30  Cprogrammer
 * added -i option to added service identifier
 *
 * Revision 1.5  2009-10-11 09:38:31+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.4  2009-10-07 22:56:59+05:30  Cprogrammer
 * removed --stdout option
 *
 * Revision 1.3  2009-10-07 10:18:26+05:30  Cprogrammer
 * added initialize() routine
 *
 * Revision 1.2  2009-10-07 09:57:37+05:30  Cprogrammer
 * removed indimail dependency
 *
 * Revision 1.1  2009-10-06 13:50:14+05:30  Cprogrammer
 * Initial revision
 *
 * This version of pam-checkpwd was written by hacking checkpassword-pam
 * written by Alexey Mahotkin <alexm@hsys\&.msk\&.ru>
 */
