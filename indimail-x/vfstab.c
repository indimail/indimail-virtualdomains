/*
 * $Log: vfstab.c,v $
 * Revision 1.4  2022-10-20 11:58:52+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.3  2019-06-07 15:52:18+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:19:02+05:30  Cprogrammer
 * replaced atol() with scan_int()
 *
 * Revision 1.1  2019-04-18 08:33:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#define XOPEN_SOURCE = 600
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <subfd.h>
#include <substdio.h>
#include <qprintf.h>
#include <fmt.h>
#include <scan.h>
#include <sgetopt.h>
#endif
#include "variables.h"
#include "vfstab.h"
#include "getFreeFS.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfstab.c,v 1.4 2022-10-20 11:58:52+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL         "vfstab: fatal: "
#define WARN          "vfstab: warning: "
#define FSTAB_SELECT  0
#define FSTAB_INSERT  1
#define FSTAB_DELETE  2
#define FSTAB_UPDATE  3
#define FSTAB_STATUS  4
#define FSTAB_NEWFS   5
#define FSTAB_BALANCE 6

static char    *usage =
	"usage: vfstab [options] [ -dilu|-o status -n user_quota -q size_quota -m mdaHost filesystem | -s [-m host] | -b ]\n"
	"options: -V (print version number)\n"
	"         -v (verbose )\n"
	"         -d (delete local/remote filesystem)\n"
	"         -i (insert local/remote filesystem)\n"
	"         -u (update local/remote filesystem)\n"
	"         -o (make   local/remote filesystem offline/online, 0 - Offline, 1 - Online)\n"
	"         -l (add    local filesystem)\n"
	"         -n max number of users\n"
	"         -q max size of filesystem\n"
	"         -m mdaHost\n"
	"         -s [-m mdaHost] (show filesystems)\n"
	"         -b (balance filesystems)"
	;

int
get_options(int argc, char **argv, int *fstabAction, int *FStabstatus, char **mdaHost,
		char **fileSystem, long *size_quota, long *user_quota)
{
	int             c;

	*mdaHost = *fileSystem = 0;
	*fstabAction = FSTAB_SELECT;
	while ((c = getopt(argc, argv, "vdiusblo:m:q:n:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			*fstabAction = FSTAB_SELECT;
			break;
		case 'd':
			*fstabAction = FSTAB_DELETE;
			break;
		case 'i':
			*fstabAction = FSTAB_INSERT;
			break;
		case 'u':
			*fstabAction = FSTAB_UPDATE;
			break;
		case 'l':
			*fstabAction = FSTAB_NEWFS;
			break;
		case 'm':
			*mdaHost = optarg;
			break;
		case 'q':
			scan_ulong(optarg, (unsigned long *) size_quota);
			break;
		case 'n':
			scan_ulong(optarg, (unsigned long *) user_quota);
			break;
		case 'o':
			if (*fstabAction == FSTAB_SELECT)
				*fstabAction = FSTAB_STATUS;
			scan_int(optarg, FStabstatus);
			if (*FStabstatus != 0 && *FStabstatus != 1) {
				strerr_warn1("vfstab: status should be 0 or 1", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			break;
		case 'b':
			*fstabAction = FSTAB_BALANCE;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc)
		*fileSystem = argv[optind++];
	if ((*fstabAction != FSTAB_SELECT && *fstabAction != FSTAB_BALANCE) && !*fileSystem) {
		strerr_warn1("vfstab: Must specify FileSystem", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	} else
	if (*fstabAction != FSTAB_SELECT && *fstabAction != FSTAB_NEWFS && *fstabAction != FSTAB_BALANCE && !*mdaHost) {
		strerr_warn1("vfstab: must supply MDA IP Address", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	} else
	if (*fstabAction == FSTAB_INSERT && (*size_quota == -1 && *user_quota == -1)) {
		strerr_warn1("vfstab: Must specify Max Size or Max Users", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	} else
	if (*fstabAction == FSTAB_UPDATE && (*size_quota == -1 && *user_quota == -1)) {
		strerr_warn1("vfstab: Must specify Max Size or Max Users", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	} 
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vfstab: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char          *mdaHost, *fileSystem, *tmpfstab;
	static stralloc tmp = {0};
	char            strnum[FMT_ULONG];
	int             status, flag, retval, fstabAction, FStabstatus = -1;
	long            max_users, cur_users, max_size, cur_size;
	long            size_quota = -1, user_quota = -1;


	if (get_options(argc, argv, &fstabAction, &FStabstatus, &mdaHost,
			&fileSystem, &size_quota, &user_quota))
		return (0);
	switch (fstabAction)
	{
	case FSTAB_SELECT:
		if (!stralloc_copys(&tmp, mdaHost) || !stralloc_0(&tmp))
			die_nomem();
		tmp.len--;
		for (flag = 0, retval = 1;;) {
			if (!(tmpfstab = vfstab_select(&tmp, &status, &max_users, &cur_users, &max_size, &cur_size)))
				break;
			if (!flag++) {
				if (substdio_put(subfdoutsmall, "File System          Host                 Status   Max Users  Cur Users       Max Size        Cur Size Load\n", 108))
					strerr_warn1("unable to write output: ", &strerr_sys);
				retval = 0;
			}
			subprintf(subfdoutsmall, "%-20s %-20s %s %10ld %10ld %10ld  Kb  %10ld  Kb  %6.4f\n",
					tmpfstab, mdaHost, status == FS_ONLINE ? "ONLINE  " : "OFFLINE ",
					max_users, cur_users, max_size/1024, cur_size/1024,
					cur_size ? ((float) (cur_users * 1024 * 1024)/ (float) cur_size) : 0.1);
			if (substdio_flush(subfdoutsmall))
				strerr_warn1("unable to write output: ", &strerr_sys);
		}
		break;
	case FSTAB_INSERT:
		retval = vfstab_insert(fileSystem, mdaHost, FS_ONLINE, user_quota, size_quota);
		break;
	case FSTAB_DELETE:
		retval = vfstab_delete(fileSystem, mdaHost);
		break;
	case FSTAB_UPDATE:
		retval = vfstab_update(fileSystem, mdaHost, user_quota, size_quota, FStabstatus);
		break;
	case FSTAB_STATUS:
		retval = vfstab_status(fileSystem, mdaHost, FStabstatus);
		break;
	case FSTAB_NEWFS:
		retval = vfstabNew(fileSystem, user_quota, size_quota);
		break;
	case FSTAB_BALANCE:
		if (!(tmpfstab = getFreeFS())) {
			strerr_warn1("vfstab: balancing failed", 0);
			return (1);
		} else
			strerr_warn2("Filesystem Selected ", tmpfstab, 0);
		retval = 0;
		break;
	default:
		retval = 1;
		strnum[fmt_uint(strnum, fstabAction)] = 0;
		strerr_warn3("error, FSTAB Action is invalid [", strnum, "]", 0);
		break;
	}
	return (retval);
}
