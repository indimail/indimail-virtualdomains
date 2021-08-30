/*
 * $Log: clearopensmtp.c,v $
 * Revision 1.4  2020-04-01 18:53:20+05:30  Cprogrammer
 * moved getEnvConfig to libqmail
 *
 * Revision 1.3  2019-06-07 15:58:53+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:09:52+05:30  Cprogrammer
 * added missing header strerr.h
 *
 * Revision 1.1  2019-04-18 17:41:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: clearopensmtp.c,v 1.4 2020-04-01 18:53:20+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <getEnvConfig.h>
#include <noreturn.h>
#endif
#include "dbload.h"
#include "iopen.h"
#include "iclose.h"
#include "clear_open_smtp.h"
#include "update_rules.h"
#include "is_already_running.h"

#define FATAL   "clearopensmtp: fatal: "
#define WARN    "clearopensmtp: warning: "

static char    *usage =
	"usage: clearopensmtp [options]\n"
	"options: -V (print version number)\n"
	"         -t (run updaterules)\n"
	"         -s (clear MySQL RELAY TABLE)"
	;

#ifdef HAVE_QMAIL
no_return
#endif
void
print_usage()
{
	char           *tmpstr;

	getEnvConfigStr(&tmpstr, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	strerr_warn5(WARN, usage, " (", tmpstr, ")", 0);
	_exit(100);
}

#ifdef HAVE_QMAIL
no_return
#endif
static void
die_nomem()
{
	strerr_warn1("clearopensmtp: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *mcdfile, *controldir, *tmpstr;
	static stralloc tmpbuf = {0};
	time_t          clear_seconds;
	int             c, errflag = 0, job_type = 1, cluster_opt = 0;

	/*-
	 * default is to clear both MySQL relay table and cdb
	 */
	while ((c = getopt(argc, argv, "ts")) != opteof) {
		switch (c)
		{
		case 'C':
			cluster_opt = 1;
			break;
		case 't': /*- rebuild cdb only */
			job_type = 2;
			break;
		case 's': /*- clear MySQL relay table only */
			job_type = 3;
			break;
		default:
			print_usage(); /*- does not return */
			break;
		}
	}
	if (is_already_running("clearopensmtp")) {
		strerr_warn1("clearopensmtp is already running", 0);
		return (1);
	}
	if (job_type == 1 || job_type == 3) { /*- clear relay table */
		getEnvConfigLong(&clear_seconds, "RELAY_CLEAR_MINUTES", RELAY_CLEAR_MINUTES);
		clear_seconds *= 60;
		errflag = clear_open_smtp(clear_seconds, cluster_opt);
#ifdef CLUSTERED_SITE
		close_db();
#else
		iclose();
#endif
	}
	if (job_type == 1 || job_type == 2) { /*- update rules */
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
		if (*mcdfile == '/') {
			if (!stralloc_copys(&tmpbuf, mcdfile) || !stralloc_0(&tmpbuf))
				die_nomem();
		} else {
			if (*controldir == '/') {
				if (!stralloc_copys(&tmpbuf, controldir) ||
						!stralloc_append(&tmpbuf, "/") ||
						!stralloc_cats(&tmpbuf, mcdfile) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
			} else {
				getEnvConfigStr(&tmpstr, "SYSCONFDIR", SYSCONFDIR);
				if (!stralloc_copys(&tmpbuf, tmpstr) ||
						!stralloc_append(&tmpbuf, "/") ||
						!stralloc_cats(&tmpbuf, controldir) ||
						!stralloc_append(&tmpbuf, "/") ||
						!stralloc_cats(&tmpbuf, mcdfile) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
			}
		}
		if (access(tmpbuf.s, F_OK)) {
			if (job_type == 2 && iopen((char *) 0))
				return (1);
			errflag = update_rules(1);
			iclose();
		}
		unlink("/tmp/clearopensmtp.PID");
	}
	return (errflag);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-roaming-users=y", 0);
	return (0);
}
#endif
