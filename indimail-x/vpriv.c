/*
 * $Log: vpriv.c,v $
 * Revision 1.4  2022-10-20 11:59:12+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.3  2019-06-07 15:44:10+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:19:52+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-15 12:29:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vpriv.c,v 1.4 2022-10-20 11:59:12+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <fmt.h>
#include <strerr.h>
#include <subfd.h>
#include <sgetopt.h>
#endif
#include "get_indimailuidgid.h"
#include "variables.h"
#include "qprintf.h"
#include "vpriv.h"
#include "check_group.h"
#include "mgmtpassfuncs.h"

#define V_PRIV_SELECT 0
#define V_PRIV_INSERT 1
#define V_PRIV_DELETE 2
#define V_PRIV_UPDATE 3
#define V_PRIV_DELUSR 4
#define V_PRIV_GRANT  5
#define FATAL         "vpriv: fatal: "
#define WARN          "vpriv: warning: "

static char    *usage =
	"usage: vpriv [options] user CommandLineSwitches\n"
	"options: -V ( print version number )\n"
	"         -v ( verbose )\n"
	"         -s ( show privileges )\n"
	"         -d program (remove privilege to run program)\n"
	"         -i program (add privilege to run program)\n"
	"         -m program (modify privilege)\n"
	"         -D Delete All Privileges for user\n"
	"         -a Set All Privileges for user"
	;

int
get_options(int argc, char **argv, char **user, char **program,
	char **cmdargs, char **oldcmdargs, int *action)
{
	int             c;
	extern char    *optarg;
	extern int      optind;

	verbose = 0;
	*action = -1;
	*user = *program = *cmdargs = 0;
	while ((c = getopt(argc, argv, "vasDd:i:m:")) != opteof)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'a':
			*action = V_PRIV_GRANT;
			break;
		case 's':
			*action = V_PRIV_SELECT;
			break;
		case 'D':
			*action = V_PRIV_DELUSR;
			break;
		case 'd':
			*action = V_PRIV_DELETE;
			*program = optarg;
			break;
		case 'i':
			*action = V_PRIV_INSERT;
			*program = optarg;
			break;
		case 'm':
			*action = V_PRIV_UPDATE;
			*program = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (*action == -1) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (optind < argc)
		*user = argv[optind++];
	if (optind < argc)
		*cmdargs = argv[optind++];
	if (*action != V_PRIV_SELECT && !*user) {
		strerr_warn1("vpriv: must supply user", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if (*action == V_PRIV_GRANT)
		return (0);
	if (!*program && (*action == V_PRIV_INSERT || *action == V_PRIV_UPDATE
		|| *action == V_PRIV_DELETE))
	{
		strerr_warn1("vpriv: must supply program", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	if ((*action == V_PRIV_UPDATE || *action == V_PRIV_INSERT ) && !*cmdargs) {
		strerr_warn1("vpriv: must supply Command Line Switches", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	int             action, err, i;
	char           *ptr, *user, *program, *cmdargs, *oldcmdargs;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	uid_t           uid;
	gid_t           gid;

	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
		strerr_warn5("vpriv: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	if (get_options(argc, argv, &user, &program, &cmdargs, &oldcmdargs, &action))
		return (1);
	if (action != V_PRIV_SELECT) {
		if (!user || !*user) {
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
		if (mgmtpassinfo(user, 0) && userNotFound)
			return (1);
	}
	switch (action)
	{
	case V_PRIV_SELECT:
		for(;;) {
			if (!(ptr = vpriv_select(&user, &program)))
				break;
			if (subprintf(subfdoutsmall, "%-20s --> %-20s %s\n", user, program, ptr) == -1)
				strerr_die1sys(111, "unable to write to stdout");
		}
		if (substdio_flush(subfdoutsmall) == -1)
			strerr_die1sys(111, "unable to write to stdout");
		break;
	case V_PRIV_INSERT:
		return (vpriv_insert(user, program, cmdargs));
		break;
	case V_PRIV_DELETE: /*- Delete program */
		return (vpriv_delete(user, program));
		break;
	case V_PRIV_UPDATE:
		return (vpriv_update(user, program, cmdargs));
		break;
	case V_PRIV_DELUSR:
		return (vpriv_delete(user, 0));
		break;
	case V_PRIV_GRANT:
		for (err = i = 0; adminCommands[i].name; i++) {
			if (vpriv_insert(user, adminCommands[i].name, "*"))
				err = 1;
		}
		return (err);
		break;
	default:
		strnum1[fmt_int(strnum1, action)] = 0;
		strerr_warn3("vpriv: error, Action [", strnum1, "] is invalid ", 0);
		return (1);
	}
	return (0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y\n");
	return (0);
}
#endif
