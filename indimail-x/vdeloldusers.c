/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vdeloldusers.c,v 1.7 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <alloc.h>
#include <strerr.h>
#include <str.h>
#include <scan.h>
#include <subfd.h>
#endif
#include "is_already_running.h"
#include "common.h"
#include "get_localtime.h"
#include "getlastauth.h"
#include "variables.h"
#include "getindimail.h"
#include "iclose.h"
#include "deluser.h"
#include "trashpurge.h"
#include "delunreadmails.h"

#define FATAL            "vdeloldusers: fatal: "
#define WARN             "vdeloldusers: warning: "
#define DEFAULT_AGE      60
#define DEFAULTMAIL_AGE  30

void            SigExit(int);

char          **skipGecos, **mailboxArr;
int             shouldexit;

static char    *usage =
	"usage: vdeloldusers [options]\n"
	"options: -d domain\n"
	"         -a age_in_days (will delete accounts older than this date)\n"
	"                        (default is 2 months or 60 days)\n"
	"         -u age_in_days (will delete mails    older than this date)\n"
	"                        (default is 1 months or 30 days)\n"
	"                        (-1 for not to delete mails)\n"
	"         -t days        (will clean trash for user active for this number of days)\n"
	"                        (default is 1 day)\n"
	"                        (-1 for not to delete trash)\n"
	"         -m mailboxes   (List of mailboxes to be purged. Multiple -m can be given)\n"
	"                        (default value .Trash)\n"
	"         -s gecos       (Skip entries having this gecos. Multiple -s can be given)\n"
	"         -c Remove Mails only\n"
	"         -p Purge entry from Database\n"
	"         -i Mark the user as Inactive\n"
	"         -r Report only\n"
	"         -f Run Fast    (avoids costly stat in Maildir/cur)\n"
	"         -v (verbose)"
	;

static void
die_nomem()
{
	strerr_warn1("vdeloldusers: out of memory", 0);
	_exit(111);
}

int
get_options(int argc, char **argv, char **Domain, int *Age, int *mailAge,
		int *trashAge, int *purge_db, int *report_only, int *fast_option,
		int *c_option, int *i_option, int *p_option)
{
	int             c, len, gecosCount, mailboxCount;
	char           *ptr, *gecosarr, *mailboxarr;
	static int      gecoslen, mailboxlen;
	static char    *(folderlist[]) = { ".Trash", ".BulkMail", 0 };

	*Domain = 0;
	gecosCount = gecoslen = mailboxCount = mailboxlen = 0;
	gecosarr = (char *) 0;
	mailboxarr = (char *) 0;
	*Age = DEFAULT_AGE;
	*trashAge = 1;
	*mailAge = DEFAULTMAIL_AGE;
	*purge_db = 0;
	c_option = i_option = p_option = 0;
	while ((c = getopt(argc, argv, "vrpicfd:a:u:t:m:s:")) != opteof) {
		switch (c)
		{
		case 'd':
			for (ptr = optarg; *ptr; ptr++) {
				if (isupper(*ptr))
					strerr_die4x(100, WARN, "domain [", optarg, "] has an uppercase character");
			}
			*Domain = optarg;
			break;
		case 's':
			if (!alloc_re((void **) gecosarr, gecoslen, (len = str_len(optarg)) + 1 + gecoslen))
				die_nomem();
			gecosCount++;
			str_copy(gecosarr + gecoslen, optarg);
			gecoslen += (len + 1);
			break;
		case 'a': /*- Inactive Days */
			scan_int(optarg, Age);
			break;
		case 'u': /*- Max age of mails */
			scan_int(optarg, mailAge);
			break;
		case 't': /*- Max age of mails in Trash or in any folders specified by 'm' option */
			scan_int(optarg, trashAge);
			break;
		case 'm':
			if (!alloc_re((void *) mailboxarr, mailboxlen, (len = str_len(optarg)) + 2 + mailboxlen))
				die_nomem();
			mailboxCount++;
			mailboxarr[mailboxlen] = '.';
			str_copy(mailboxarr + mailboxlen + 1, optarg);
			mailboxlen += (len + 2);
			break;
		case 'r':
			*report_only = 1;
		case 'v':
			verbose = 1;
			break;
		case 'f':
			*fast_option = 1;
			break;
		case 'c': /*- Remove Mails only */
			*c_option = 1;
			if (p_option || i_option) {
				strerr_warn1("only one of c, i, p option can be specified", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*purge_db = 0;
			break;
		case 'p': /*- delete the user from the database */
			*p_option = 1;
			if (c_option || i_option) {
				strerr_warn1("only one of c, i, p option can be specified", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*purge_db = 1;
			break;
		case 'i': /*- Delete the home directory and move the user to indibak (inactivate) */
			if (p_option || c_option) {
				strerr_warn1("only one of c, i, p option can be specified", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			*i_option = 1;
			*purge_db = 2;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (!*Domain) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (gecosCount) {
		if (!(skipGecos = (char **) alloc(sizeof(char *) * (gecosCount + 2))))
			die_nomem();
		for (c = 0, ptr = gecosarr; c < gecosCount; c++) {
			skipGecos[c] = ptr;
			ptr += str_len(ptr) + 1;
		}
		skipGecos[gecosCount] = "MailGroup"; /*- By default this should be skipped */
		skipGecos[gecosCount + 1] = (char *) 0;
	}
	if (mailboxCount) {
		if (!(mailboxArr = (char **) alloc(sizeof(char *) * (mailboxCount + 1))))
			die_nomem();
		for (c = 0, ptr = mailboxarr; c < mailboxCount; c++) {
			mailboxArr[c] = ptr;
			ptr += str_len(ptr) + 1;
		}
		mailboxArr[mailboxCount] = (char *) 0;
	} else
		mailboxArr = folderlist; /*- Only Trash */
	return (0);
}

int
LocateUser(char **Table, char *username, int init_flag)
{
	static char   **Table_ptr, **ptr;
	int             ret;

	if (!Table_ptr || init_flag)
		Table_ptr = Table;
	for (ptr = Table_ptr; ptr && *ptr; ptr++) {
		ret = str_diff(*ptr, username);
		if (!ret) {
			**ptr = ' ';
			Table_ptr = ptr + 1;
			return (1);
		}
	}
	strerr_warn3("Warning [", username, "] not found", 0);
	return (0);
}

int
main(int argc, char **argv)
{
	register char **ptr, **tmp, **lastauthptr, **indimailptr;
	unsigned long   totalcount, activecount = 0, count, purged;
	char           *Domain;
	struct stat     statbuf;
	static stralloc CurDir = {0};
	int             Age, mailAge, trashAge, purge_db, report_only,
					fast_option, len, isAtty, do_lastauth,
					c_option, i_option, p_option;
	time_t          tmval, diff;

	c_option = i_option = p_option = 0;
	if (get_options(argc, argv, &Domain, &Age, &mailAge, &trashAge,
			&purge_db, &report_only, &fast_option,
			&c_option, &i_option, &p_option))
		return (1);
	if (is_already_running("vdeloldusers")) {
		strerr_warn1("vdeloldusers is already running", 0);
		return (1);
	}
	isAtty = (isatty(1) && isatty(2));
	signal(SIGUSR1, SigExit);
	if (verbose)
		subprintfe(subfdout, "vdeloldusers", "Total  User List %s", get_localtime());
	if (c_option || i_option || p_option)
		indimailptr = getindimail(Domain, 1, skipGecos, &totalcount);
	else /*- For deleting trash ignore skipGecos */
		indimailptr = getindimail(Domain, 1, 0, &totalcount);
	if (totalcount == -1) {
		strerr_warn1("could not get entries from indimail table", 0);
		iclose();
		if (unlink("/tmp/vdeloldusers.PID") == -1)
			strerr_warn1("unlink: /tmp/vdeloldusers.PID", &strerr_sys);
		return (1);
	}
	if (verbose)
		subprintfe(subfdout, "vdeloldusers", " Total Entries %lu\n", totalcount);
	if (!c_option && !i_option && !p_option)
		goto trash_clean;
	if (verbose)
		subprintfe(subfdout, "vdeloldusers", "Active  User List %s", get_localtime());
	if (c_option || i_option || p_option)
		lastauthptr = getlastauth(Domain, Age, 1, 1, &activecount);
	else
		lastauthptr = 0;
	if (activecount == -1) {
		strerr_warn1("could not get entries from lastauth table", 0);
		iclose();
		if (unlink("/tmp/vdeloldusers.PID") == -1)
			strerr_warn1("unlink: /tmp/vdeloldusers.PID", &strerr_sys);
		return (1);
	}
	if (verbose)
		subprintfe(subfdout, "vdeloldusers", "%lu Active Entries\n", activecount);
	if (verbose)
		subprintfe(subfdout, "vdeloldusers", "Dedup  User List %s", get_localtime());
	if (c_option || i_option || p_option) {
		tmval = time(0);
		for (purged = count = 0, ptr = lastauthptr; ptr && *ptr; ptr++) {
			if (LocateUser(indimailptr, *ptr, 0))
				purged++;
			diff = time(0) - tmval;
			count++;
			if (verbose && isAtty) {
				if (diff)
					subprintfe(subfdout, "vdeloldusers", "\r%7lu %.2f", count, (float) count/diff);
				else
					subprintfe(subfdout, "vdeloldusers", "\r%7lu Inf", count);
				flush("vdeloldusuers");
			}
		}
		if (verbose && isAtty)
			out("vdeloldusers", "\n");
	}
	if (totalcount == activecount)	/*- all records are active users */
		purge_db = 0;
	tmval = time(0);
	for (purged = 0, ptr = indimailptr; ptr && *ptr; ptr++) {
		if (report_only) {
			if (**ptr != ' ') {
				if (fast_option) {
					purged++;
					if (verbose)
						subprintfe(subfdout, "vdeloldusers", "%s\n", *ptr);
					continue;
				}
				len = str_len(*ptr);
				if (!stralloc_copys(&CurDir, *ptr + len + 2) ||
						!stralloc_catb(&CurDir, "/Maildir/cur", 12) ||
						!stralloc_0(&CurDir))
					die_nomem();
				if (!stat(CurDir.s, &statbuf) && ((tmval - statbuf.st_mtime) / 86400 <= Age)) {
					strerr_warn7("user ", *ptr, "@", Domain, " Dir ", *ptr + len + 2, " not old", 0);
					continue;
				}
				purged++;
				if (verbose)
					subprintfe(subfdout, "vdeloldusers", "%s\n", *ptr);
			}
			continue;
		}
		len = str_len(*ptr);
		if (**ptr == ' ') {
			if (shouldexit) {
				iclose();
				if (unlink("/tmp/vdeloldusers.PID") == -1)
					strerr_warn1("unlink: /tmp/vdeloldusers.PID", &strerr_sys);
				return (0);
			}
			if (mailAge > 0) {
				/*- Delete from Maildir/cur */
				delunreadmails(*ptr + len + 2, 2, mailAge);
				/*- Delete from Maildir/new */
				delunreadmails(*ptr + len + 2, 1, mailAge);
			}
			**ptr = *(*ptr + len + 1); /*- Restore the first char */
			continue;
		}
		if (!purge_db)
			continue;
		if (!stralloc_copys(&CurDir, *ptr + len + 2) ||
				!stralloc_catb(&CurDir, "/Maildir/cur", 12) ||
				!stralloc_0(&CurDir))
			die_nomem();
		if (!fast_option && !stat(CurDir.s, &statbuf) && ((tmval - statbuf.st_mtime) / 86400 <= Age)) {
			strerr_warn7("user ", *ptr, "@", Domain, " Dir ", *ptr + len + 2, " not old", 0);
			continue;
		}
		if (verbose && !purged)
			subprintfe(subfdout, "vdeloldusers", "Purging Inactive Users %s\n", get_localtime());
		if (!deluser(*ptr, Domain, purge_db))
			purged++;
	}
	if (verbose && !report_only) {
		subprintfe(subfdout, "vdeloldusers", "Purged %lu/%lu Inactive Users\n", purged, totalcount);
		flush("vdeloldusuers");
	}
	if (c_option || i_option || p_option) {
		for (ptr = lastauthptr; ptr && *ptr; ptr++)
			alloc_free((char *) *ptr);
		alloc_free((char *) lastauthptr);
	}
	if (report_only) {
		if (verbose)
			subprintfe(subfdout, "vdeloldusers", "Program Complete %s\n", get_localtime());
		iclose();
		if (unlink("/tmp/vdeloldusers.PID") == -1)
			strerr_warn1("unlink: /tmp/vdeloldusers.PID", &strerr_sys);
		return (0);
	}
trash_clean:
	if (mailboxArr) {
		for (tmp = mailboxArr, do_lastauth = 1; *tmp; tmp++) {
			if (!str_diffn(*tmp, ".Trash", 7)) {
				/*-
				 * Trash gets filled only if the user logon and delete mails.
				 * Folders like .BulkMail gets filled without the need for
				 * the user to logon. Hence for such folders include inactive
				 * users too.
				 */
				do_lastauth = 0;
				break;
			}
		}
		if (do_lastauth) {
			if (verbose)
				subprintfe(subfdout, "vdeloldusers", "Deleting Trash %s\n", get_localtime());
			/*- Get Users Active in the Last trashAge Days */
			lastauthptr = getlastauth(Domain, trashAge, 1, 1, &activecount);
			for (count = 0, ptr = lastauthptr; ptr && *ptr; ptr++)
				LocateUser(indimailptr, *ptr, !count++);
		} else {
			if (verbose) {
				subprintfe(subfdout, "vdeloldusers", "Deleting Folders ");
				for (tmp = mailboxArr; *tmp; tmp++)
					subprintfe(subfdout, "vdeloldusers", " %s", *tmp);
				subprintfe(subfdout, "vdeloldusers",  "%s", get_localtime());
				flush("vdeloldusuers");
			}
		}
		for (ptr = indimailptr; ptr && *ptr; ptr++) {
			if (shouldexit) {
				iclose();
				if (unlink("/tmp/vdeloldusers.PID") == -1)
					strerr_warn1("unlink: /tmp/vdeloldusers.PID", &strerr_sys);
				return (0);
			}
			if (do_lastauth) {
				if (**ptr == ' ') {
					for (tmp = mailboxArr; *tmp; tmp++)
						mailboxpurge(*ptr + str_len(*ptr) + 2, *tmp, trashAge, 1);
				}
			} else
			for (tmp = mailboxArr; *tmp; tmp++)
				mailboxpurge(*ptr + str_len(*ptr) + 2, *tmp, trashAge, 1);
		}
	} /*- if (mailboxArr) */
	if (verbose)
		subprintfe(subfdout, "vdeloldusers", "Program Complete %s\n", get_localtime());
	iclose();
	if (unlink("/tmp/vdeloldusers.PID") == -1)
		strerr_warn1("unlink: /tmp/vdeloldusers.PID", &strerr_sys);
	return (0);
}

void
SigExit(int x)
{
	shouldexit = 1;
	signal(SIGUSR1, SigExit);
	return;
}
#else
int
main()
{
	strerr_warn1("auth logging was not enabled, reconfigure with --enable-auth-logging=y", 0);
	return (1);
}
#endif
/*
 * $Log: vdeloldusers.c,v $
 * Revision 1.7  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.5  2022-10-20 11:58:41+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.4  2021-05-03 12:48:00+05:30  Cprogrammer
 * fix compiler warnings
 *
 * Revision 1.3  2020-10-01 18:31:05+05:30  Cprogrammer
 * initialize activecount variable
 *
 * Revision 1.2  2019-06-07 15:53:48+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:39:54+05:30  Cprogrammer
 * Initial revision
 *
 */
