/*
 * $Log: chowkidar.c,v $
 * Revision 1.2  2019-06-07 15:58:31+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:39:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <env.h>
#include <scan.h>
#include <substdio.h>
#include <subfd.h>
#include <getEnvConfig.h>
#endif
#include "LoadBMF.h"
#include "spam.h"
#include "common.h"
#include "variables.h"

#define BADMAIL 1
#define BADRCPT 2
#define SPAMDB  3

#ifndef	lint
static char     sccsid[] = "$Id: chowkidar.c,v 1.2 2019-06-07 15:58:31+05:30 mbhangui Exp mbhangui $";
#endif

#define FATAL   "chowkidar: fatal: "
#define WARN    "chowkidar: warning: "

static char    *usage =
	"Usage: chowkidar [-V] [-r] [-f filename]\n"
	"       [-b badmailfrom_file | -t badrcptto_file | -s spamdb_file]\n"
	"       [-B | -T | -S] [-q] -n count\n"
	"options:  -f multilog logfile\n"
	"          -n Spam Threshold Count\n"
	"          -V display version number\n"
	"          -v set verbose mode\n"
#ifdef CLUSTERED_SITE
	"          -r sync mode operation\n"
#endif
	"          -q silent mode (for sync mode operation)\n"
	"          -b badmailfrom filename (badmailfrom or any other filename)\n"
	"          -B Select badmailfrom mode\n"
	"                                   or\n"
	"          -t badrcptto   filename (badrcptto or any other filename)\n"
	"          -T Select badrcptto mode\n"
	"                                   or\n"
	"          -s spamdb      filename (spamdb or any other filename)\n"
	"          -S Select spamdb mode"
	;

static void
die_nomem()
{
	strerr_warn1("chowkidar: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *ptr, *revision = "$Revision: 1.2 $";
	int             spamNumber, spamFilter, c, silent, type, relative;
	static stralloc ignfile = {0}, bad_from_rcpt_file = {0};
	char           *filename = (char *) 0, *outfile = (char *) 0;
	char           *sysconfdir, *controldir, *ign;
#ifdef CLUSTERED_SITE
	int             i, j, total, sync_mode;
	char          **Ptr, **bmfptr;
#endif

#ifdef CLUSTERED_SITE
	type = sync_mode = silent = 0;
#else
	type = silent = 0;
#endif
	spamNumber = spamFilter = 0;
	filename = (char *) 0;
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
#ifdef CLUSTERED_SITE
	while ((c = getopt(argc, argv, "f:t:b:s:n:o:BTSVrqv")) != opteof)
#else
	while ((c = getopt(argc, argv, "f:t:b:s:n:o:BTSVqv")) != opteof)
#endif
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'o':
			outfile = optarg;
			break;
		case 'q':
			silent = 1;
			break;
#ifdef CLUSTERED_SITE
		case 'r':
			sync_mode = 1;
			break;
#endif
		case 'V':
			ptr = revision + 11;
			if (*ptr) {
				out("chowkidar", "IndiMail Chowkidar Version ");
				for (;*ptr;ptr++) {
					if (isspace((int) *ptr))
						break;
					if (substdio_put(subfdout, ptr, 1))
						strerr_die1sys(111, "unable to write to stdout; ");
				}
				if (substdio_put(subfdout, "\n", 1))
					strerr_die1sys(111, "unable to write to stdout; ");
				if (substdio_flush(subfdout))
					strerr_die1sys(111, "unable to write to stdout; ");
			}
			return(0);
			break;
		case 'n':
			scan_int(optarg, &spamNumber);
			break;
		case 'f':
			filename = optarg;
			break;
		case 'b':
			outfile = optarg;
		case 'B':
			if (!outfile) {
				outfile = "badmailfrom";
				if (chdir(sysconfdir) || chdir(controldir)) {
					strerr_warn1("unable to cd to sysconfdir or controldir: ", &strerr_sys);
					return (1);
				}
			}
			if (type) {
				strerr_warn1("only one of -b|-B, -t|-T, -s|-S can be selected", 0);
				return(1);
			}
			type = BADMAIL;
			break;
		case 't':
			outfile = optarg;
		case 'T':
			if (!outfile) {
				outfile = "badrcptto";
				if (chdir(sysconfdir) || chdir(controldir)) {
					strerr_warn1("unable to cd to sysconfdir or controldir: ", &strerr_sys);
					return (1);
				}
			}
			if (type) {
				strerr_warn1("only one of -b|-B, -t|-T, -s|-S can be selected", 0);
				return(1);
			}
			type = BADRCPT;
			break;
		case 's':
			outfile = optarg;
		case 'S':
			if (!outfile) {
				outfile = "spamdb";
				if (chdir(sysconfdir) || chdir(controldir)) {
					strerr_warn1("unable to cd to sysconfdir or controldir: ", &strerr_sys);
					return (1);
				}
			}
			if (type) {
				strerr_warn1("only one of -b|-B, -t|-T, -s|-S can be selected", 0);
				return(1);
			}
			type = SPAMDB;
			break;
		default:
			spamNumber = -1;
			strerr_warn2(WARN, usage, 0);
			return(1);
		}
	}
#ifdef CLUSTERED_SITE
	if (!type || (!sync_mode && !filename) || !outfile)
#else
	if (!type || !filename || !outfile)
#endif
	{
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
#ifdef CLUSTERED_SITE
	if (sync_mode) {
		if (outfile && *outfile) {
			i = str_chr(outfile, '.');
			j = str_chr(outfile, '/');
			if (outfile[i] || outfile[j]) {
				strerr_warn1("PATH not allowed in badmailfrom/badrcptto/spamdb file", 0);
				return(1);
			}
		}
		if (!(bmfptr = LoadBMF(&total, outfile))) {
			strerr_warn1("LoadBMF: no entries loaded", 0);
			return(1);
		}
		for (Ptr = bmfptr;!silent && Ptr && *Ptr;Ptr++)
			strerr_warn1(*Ptr, 0);
		return(0);
	} 
#endif
	/*- Update mode */
	if (!readLogFile(filename, type, spamNumber)) {
		/*
		 * ignore list is our list of priviliged mail users 
		 * firstly we read our ignore file  
		 * ... and from qmail's badmailfrom, since we do not want
		 * to have duplicate spammer addresses 
		 */
		i = str_chr(outfile, '.');
		j = str_chr(outfile, '/');
		if (outfile[i] || outfile[j]) {
			if (!stralloc_copys(&bad_from_rcpt_file, outfile) ||
					!stralloc_0(&bad_from_rcpt_file))
				die_nomem();
		} else {
			if (relative) {
				if (!stralloc_copys(&bad_from_rcpt_file, sysconfdir) ||
						!stralloc_append(&bad_from_rcpt_file, "/") ||
						!stralloc_cats(&bad_from_rcpt_file, controldir) ||
						!stralloc_append(&bad_from_rcpt_file, "/") ||
						!stralloc_cats(&bad_from_rcpt_file, outfile) ||
						!stralloc_0(&bad_from_rcpt_file))
					die_nomem();
			} else {
				if (!stralloc_copys(&bad_from_rcpt_file, controldir) ||
						!stralloc_append(&bad_from_rcpt_file, "/") ||
						!stralloc_cats(&bad_from_rcpt_file, outfile) ||
						!stralloc_0(&bad_from_rcpt_file))
					die_nomem();
			}
		}
		switch (type)
		{
			case BADMAIL:
				if (relative) {
					if (!stralloc_copys(&ignfile, sysconfdir) ||
							!stralloc_append(&ignfile, "/") ||
							!stralloc_cats(&ignfile, controldir) ||
							!stralloc_catb(&ignfile, "/badmailpatterns", 16) ||
							!stralloc_0(&ignfile))
						die_nomem();
				} else {
					if (!stralloc_copys(&ignfile, controldir) ||
							!stralloc_catb(&ignfile, "/badmailpatterns", 16) ||
							!stralloc_0(&ignfile))
						die_nomem();
				}
				if (loadIgnoreList(ignfile.s))
					return(1);
				break;
			case BADRCPT:
				if (relative) {
					if (!stralloc_copys(&ignfile, sysconfdir) ||
							!stralloc_append(&ignfile, "/") ||
							!stralloc_cats(&ignfile, controldir) ||
							!stralloc_catb(&ignfile, "/badrcptpatterns", 16) ||
							!stralloc_0(&ignfile))
						die_nomem();
				} else {
					if (!stralloc_copys(&ignfile, controldir) ||
							!stralloc_catb(&ignfile, "/badrcptpatterns", 16) ||
							!stralloc_0(&ignfile))
						die_nomem();
				}
				if (loadIgnoreList(ignfile.s))
					return(1);
				break;
			case SPAMDB:
				if (relative) {
					if (!stralloc_copys(&ignfile, sysconfdir) ||
							!stralloc_append(&ignfile, "/") ||
							!stralloc_cats(&ignfile, controldir) ||
							!stralloc_catb(&ignfile, "/spamignorepatterns", 19) ||
							!stralloc_0(&ignfile))
						die_nomem();
				} else {
					if (!stralloc_copys(&ignfile, controldir) ||
							!stralloc_catb(&ignfile, "/spamignorepatterns", 19) ||
							!stralloc_0(&ignfile))
						die_nomem();
				}
				if (loadIgnoreList(ignfile.s))
					return(1);
				break;
		}
		if (relative) {
			if (!(ign = env_get("SPAMIGNORE")))
				ign = "spamignore";
			if (!stralloc_copys(&ignfile, sysconfdir) ||
					!stralloc_append(&ignfile, "/") ||
					!stralloc_cats(&ignfile, controldir) ||
					!stralloc_cats(&ignfile, ign) ||
					!stralloc_0(&ignfile))
				die_nomem();
		} else {
			if (!(ign = env_get("SPAMIGNORE")))
				ign = "spamignore";
			if (!stralloc_copys(&ignfile, controldir) ||
					!stralloc_cats(&ignfile, ign) ||
					!stralloc_0(&ignfile))
				die_nomem();
		}
		if (!loadIgnoreList(ignfile.s) && !loadIgnoreList(bad_from_rcpt_file.s))
			return(spamReport(spamNumber, outfile));
	}
	return(1);
}
