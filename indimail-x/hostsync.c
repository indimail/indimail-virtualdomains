/*
 * $Log: hostsync.c,v $
 * Revision 1.3  2019-06-07 15:59:55+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:11:15+05:30  Cprogrammer
 * added missing strerr.h header
 *
 * Revision 1.1  2019-04-18 08:36:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef lint
static char     sccsid[] = "$Id: hostsync.c,v 1.3 2019-06-07 15:59:55+05:30 mbhangui Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <sgetopt.h>
#endif
#include "is_already_running.h"
#include "is_distributed_domain.h"
#include "sql_getflags.h"
#include "common.h"
#include "iclose.h"
#include "variables.h"
#include "open_master.h"
#include "cntrl_clearaddflag.h"
#include "cntrl_cleardelflag.h"

#define FATAL   "hostsync: fatal: "
#define WARN    "hostsync: warning: "

static char    *usage =
	"usage: hostsync [options]\n"
	"options: -d domain\n"
	"         -v (verbose)"
	;

int
get_options(int argc, char **argv, char **domain)
{
	int             c;

	*domain = 0;
	while ((c = getopt(argc, argv, "vd:")) != opteof) {
		switch (c)
		{
		case 'd':
			*domain = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (!*domain) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	int             first, ret;
	char           *domain;
	char            strnum[FMT_ULONG];
	struct passwd  *pw;

	if (get_options(argc, argv, &domain))
		return (1);
	if (is_already_running("hostsync")) {
		strerr_warn1("hostsync is already running", 0);
		return (1);
	}
	if ((ret = is_distributed_domain(domain)) == -1) {
		strerr_warn3("hostsync: unable to verify ", domain, " as a distributed domain", 0);
		(void) unlink("/tmp/hostsync.PID");
		return (1);
	} else
	if (!ret) {
		strerr_warn3("hostsync: ", domain, " is not a distributed domain", 0);
		unlink("/tmp/hostsync.PID");
		return (1);
	}
	if (open_master()) {
		strerr_warn1("hostsync: failed to open master db", 0);
		unlink("/tmp/hostsync.PID");
		return (1);
	}
	for (first = 1;;) {
		if (!(pw = sql_getflags(domain, first)))
			break;
		switch (ret = pw->pw_uid)
		{
		case ADD_FLAG:
			if (verbose)
				out("hostsync", "Adding      ");
			cntrl_clearaddflag(pw->pw_name, domain, pw->pw_passwd);
			break;
		case DEL_FLAG:
			if (verbose)
				out("hostsync", "Deleting    ");
			cntrl_cleardelflag(pw->pw_name, domain);
			break;
		default:
			if (verbose)
				out("hostsync", "Ignoring??? ");
			strnum[fmt_uint(strnum, (unsigned int) ret)] = 0;
			strerr_warn2("hostsync: Invalid case ", strnum, 0);
		}
		if (verbose) {
			out("hostsync", "user ");
			out("hostsync", pw->pw_name);
			out("hostsync", "@");
			out("hostsync", domain);
			out("hostsync", "\n");
		}
		first++;
		flush("hostsync");
	}
	if (verbose) {
		strnum[fmt_uint(strnum, (unsigned int) (first - 1))] = 0;
		out("hostsync", "Synced ");
		out("hostsync", strnum);
		out("hostsync", " entries\n");
		flush("hostsync");
	}
	iclose();
	unlink("/tmp/hostsync.PID");
	return (0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return (0);
}
#endif
