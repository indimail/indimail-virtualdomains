/*
 * $Log: mgmtpass.c,v $
 * Revision 1.5  2024-05-22 22:38:15+05:30  Cprogrammer
 * fixed handling of command line arguments
 *
 * Revision 1.4  2023-03-20 10:14:24+05:30  Cprogrammer
 * fixed command line argument handling
 *
 * Revision 1.3  2022-08-05 21:10:45+05:30  Cprogrammer
 * added encrypt_flag argument to mgmtsetpass()
 *
 * Revision 1.2  2019-04-22 23:13:52+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-18 08:31:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: mgmtpass.c,v 1.5 2024-05-22 22:38:15+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "mgmtpass: fatal: "
#define WARN    "mgmtpass: warning: "

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef sun
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <strerr.h>
#endif
#include "mgmtpassfuncs.h"
#include "variables.h"

static char    *usage =
	"options: -u user   (specify username)\n"
	"         -a passwd (adds user with specified password)\n"
	"         -p passwd (resets/changes password of existing user)\n"
	"         -i        (displays user information/stats)\n"
	"          or\n"
	"         -l        (lists admin users)"
	;

int
main(int argc, char **argv)
{
	int             idx;
	time_t          tmval;
	char           *user, *pass, *ptr;

	for (user = pass = 0, idx = 1; idx < argc; idx++) {
		if (argv[idx][0] != '-')
			continue;
		switch (argv[idx][1])
		{
		case 'u':
			user = *(argv + idx + 1);
			break;
		case 'l':
			return (mgmtlist());
		case 'p':
			if (!user || !*user) {
				strerr_warn1("user not specified", 0);
				return (1);
			}
			if (getuid() || geteuid()) {
				strerr_warn1("mgmtpass: Only superuser can specify -p option", 0);
				return (1);
			}
			if (*(argv + idx + 1)) {
				tmval = time(0);
				return (mgmtsetpass(user, *(argv + idx + 1), getuid(), getgid(), tmval, tmval, 1));
			}
			break;
		case 'i':
			if (!user || !*user) {
				strerr_warn1("user not specified", 0);
				return (1);
			}
			if (getuid() && geteuid()) {
				strerr_warn1("mgmtpass: Only superuser can specify -p option", 0);
				return (1);
			} else
				return (mgmtpassinfo(user, 1));
			break;
		case 'a':
			if (!user || !*user) {
				strerr_warn1("user not specified", 0);
				return (1);
			}
			if (str_diffn(user, "admin", 6) && mgmtpassinfo("admin", 0) && userNotFound) {
				if (!(ptr = (char *) getpass("New Admin password: ")))
					return (1);
				tmval = time(0);
				strerr_warn1("Creating user admin", 0);
				if (mgmtadduser("admin", ptr, getuid(), getgid(), tmval, tmval))
					return (1);
			}
			if ((getuid() || geteuid()) && getpassword("admin"))
				return (1);
			if (!mgmtpassinfo(user, 0)) {
				strerr_warn3("mgmtpass: User ", user, " exists", 0);
				return (1);
			}
			pass = *(argv + idx + 1);
			if (!pass && !(pass = (char *) getpass("New password: ")))
				return (1);
			tmval = time(0);
			return (mgmtadduser(user, pass, getuid(), getgid(), tmval, tmval));
		default:
			strerr_warn1("usage: mgmtpass -u user [-a passwd | -p passwd] [-i]", 0);
			strerr_warn1("usage: mgmtpass -l (list users)]", 0);
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (!user)
	{
		strerr_warn1("usage: mgmtpass -u user [-a passwd | -p passwd] [-i]", 0);
		strerr_warn1("usage: mgmtpass -l (list users)]", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (setpassword(user));
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return (1);
}
#endif
