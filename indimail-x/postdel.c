/*
 * $Log: postdel.c,v $
 * Revision 1.2  2022-12-18 19:27:02+05:30  Cprogrammer
 * recoded wait logic
 *
 * Revision 1.1  2022-10-18 14:18:44+05:30  Cprogrammer
 * Initial revision
 *
 *
 * $Id: postdel.c,v 1.2 2022-12-18 19:27:02+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <env.h>
#include <sgetopt.h>
#include <str.h>
#include <fmt.h>
#include <wait.h>
#include <noreturn.h>

#ifndef	lint
static char     rcsid[] = "$Id: postdel.c,v 1.2 2022-12-18 19:27:02+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * From postfix source distribution - sys_exits.h
 */
#define EX_USAGE		64		/*- command line usage error */
#define EX_DATAERR		65		/*- data format error */
#define EX_NOINPUT		66		/*- cannot open input */
#define EX_NOUSER		67		/*- addressee unknown */
#define EX_NOHOST		68		/*- host name unknown */
#define EX_UNAVAILABLE	69		/*- service unavailable */
#define EX_SOFTWARE		70		/*- internal software error */
#define EX_OSERR		71		/*- system error (e.g., can't fork) */
#define EX_OSFILE		72		/*- critical OS file missing */
#define EX_CANTCREAT	73		/*- can't create (user) output file */
#define EX_IOERR		74		/*- input/output error */
#define EX_TEMPFAIL		75		/*- temporary failure */
#define EX_PROTOCOL		76		/*- remote error in protocol */
#define EX_NOPERM		77		/*- permission denied */
#define EX_CONFIG		78		/*- configuration error */

static char    *(vdelargs[]) = { PREFIX"/sbin/vdelivermail", "''", BOUNCE_ALL, 0};
static char    *(filtargs[]) = { PREFIX"/sbin/vfilter", "''", BOUNCE_ALL, 0};
static stralloc user = {0};
static stralloc domain = {0};
static stralloc rpline = {0};

no_return void
die_nomem()
{
	strerr_die1sys(111, "out of memory");
}

static int
get_options(int argc, char **argv, char **ext, char **host, char **sender, int *use_filter)
{
	int             c, i;

	*use_filter = 0;
	*ext = *host = *sender = (char *) 0;
	while ((c = getopt(argc, argv, "fu:r:")) != -1) {
		switch (c)
		{
		case 'f':
			*use_filter = 1;
			break;
		case 'u':
			i = str_rchr(optarg, '@');
			if (optarg[i]) {
				if (!stralloc_copyb(&user, optarg, i) ||
						!stralloc_0(&user))
					die_nomem();
				*ext = user.s;
				if (!stralloc_copys(&domain, optarg + i + 1) ||
						!stralloc_0(&domain))
					die_nomem();
				*host = domain.s;
			} else {
				*ext = optarg;
				*host = (char *) 0;
			}
			break;
		case 'r':
			*sender=optarg;
			break;
		default:
			strerr_die1x(100, "USAGE: postdel [-f] -u recipient -r sender");
			return(1);
		}
	}
	if(!*ext || !*host || !*sender)
		strerr_die1x(100, "USAGE: postdel [-f] -u recipient -r sender");
	return(0);
}

int
main(int argc, char **argv)
{
	char           *ext, *host, *sender;
	int             i, werr, use_filter, wait_status;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	pid_t           pid;

	if(get_options(argc, argv, &ext, &host, &sender, &use_filter))
		_exit(EX_USAGE);
	if (!stralloc_copyb(&rpline, "Return-Path: <", 14) ||
			!stralloc_cats(&rpline, sender) ||
			!stralloc_append(&rpline, ">") ||
			!stralloc_0(&rpline))
		die_nomem();
	if (!env_put2("EXT", ext) ||
			!env_put2("HOST", host) ||
			!env_put2("SENDER", sender) ||
			!env_put2("RPLINE", rpline.s) ||
			!env_put2("MTA", "Postfix"))
		die_nomem();
	switch (pid = vfork())
	{
		case -1:
			strerr_die1sys(EX_OSERR, "postdel: fatal: ");
		case 0:
#ifdef MAKE_SEEKABLE
			if (!env_put2("MAKE_SEEKABLE", "1"))
				die_nomem();
#endif
			if(use_filter)
				execv(*filtargs, filtargs);
			else
				execv(*vdelargs, vdelargs);
			strerr_die1sys(EX_OSERR, "postdel: fatal: ");
		default:
			for(;;) {
				if (!(i = wait_pid(&wait_status, pid)))
					break;
				else
				if (i == -1) {
#ifdef ERESTART
					if(errno == error_intr || errno == error_restart)
#else
					if(errno == error_intr)
#endif
						continue;
					strerr_die1sys(EX_TEMPFAIL, "postdel: fatal: waitpid: ");
				} else
				if (!(i = wait_handler(wait_status, &werr)))
					continue;
				else
				if (werr == -1) {
					strnum1[fmt_ulong(strnum1, pid)] = 0;
					strerr_warn3("postdel: ", strnum1, ": internal wait handler error", 0);
					strerr_die1sys(EX_SOFTWARE, "postdel: fatal: internal wait hanlder error: ");
				} else
				if (werr) {
					strnum1[fmt_ulong(strnum1, pid)] = 0;
					strnum2[fmt_uint(strnum2, werr)] = 0;
					strerr_die4x(EX_TEMPFAIL, "postdel: ", strnum1, ": killed by signal ", strnum2);
				}
				if (i) {
					strnum1[fmt_uint(strnum1, i)] = 0;
					strerr_die3x(EX_TEMPFAIL, "postdel: fatal: vdelivermail exited with ", strnum1, " return code");
				}
			} /*- for(;;) */
			break;
	}
	_exit(EX_TEMPFAIL); return(0); /*- for stupid solaris */
}
