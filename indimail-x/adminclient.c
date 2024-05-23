/*
 * $Log: adminclient.c,v $
 * Revision 1.6  2024-05-23 17:06:27+05:30  Cprogrammer
 * added paranoid check for command mismatch
 *
 * Revision 1.5  2023-08-08 10:34:36+05:30  Cprogrammer
 * use strerr_tls for reporting tls error
 *
 * Revision 1.4  2023-01-03 21:05:41+05:30  Cprogrammer
 * added -r option to specify certification revocation list
 *
 * Revision 1.3  2021-03-04 11:51:59+05:30  Cprogrammer
 * added -m option to match host with common name
 * added -C option to specify cafile
 *
 * Revision 1.2  2019-04-22 22:24:12+05:30  Cprogrammer
 * added missing header strerr.h
 *
 * Revision 1.1  2019-04-20 08:10:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef lint
static char     sccsid[] = "$Id: adminclient.c,v 1.6 2024-05-23 17:06:27+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <strmsg.h>
#include <fmt.h>
#include <str.h>
#include <env.h>
#include <tls.h>
#endif
#include "auth_admin.h"
#include "adminCmmd.h"
#include "variables.h"

char            strnum[FMT_ULONG];

static void
die_nomem()
{
	strerr_warn1("adminclient: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *admin_user, *admin_pass, *admin_host, *admin_port, *cmmd,
				   *cmdptr1, *cmdptr2, *certfile, *cafile, *crlfile;
	static stralloc cmdbuf = {0}, cmdName = {0};
	int             sfd, i, j, k, input_read, match_cn = 0, cmdlen1, cmdlen2;

	certfile = cafile = crlfile = NULL;
	admin_user = admin_pass = admin_host = admin_port = cmmd = NULL;
	input_read = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			continue;
		switch (argv[i][1])
		{
		case 'h':
			admin_host = *(argv + i + 1);
			break;
		case 'p':
			admin_port = *(argv + i + 1);
			break;
		case 'u':
			admin_user = *(argv + i + 1);
			break;
		case 'P':
			admin_pass = *(argv + i + 1);
			break;
		case 'c':
			cmmd = *(argv + i + 1);
			break;
		case 'i':
			input_read = 1;
			break;
		case 'n':
			certfile = *(argv + i + 1);
			break;
		case 'C':
			cafile = *(argv + i + 1);
			break;
		case 'r':
			crlfile = *(argv + i + 1);
			break;
		case 'm':
			match_cn = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_warn1("usage: adminclient -h adminHost -p adminPort -u adminUser -P adminPasswd [-n certfile] [-i] -c cmmd [-v]", 0);
			strnum[0] = argv[i][1];
			strnum[1] = 0;
			strerr_warn2("invalid option ", strnum, 0);
			return (1);
		}
	}
	if (!admin_host && !(admin_host = (char *) env_get("ADMIN_HOST"))) {
		strerr_warn1("usage: adminclient -h adminHost -p adminPort -u adminUser -P adminPasswd [-n certfile] [-i] -c cmmd [-v]", 0);
		return (1);
	} else
	if (!admin_port && !(admin_port = (char *) env_get("ADMIN_PORT"))) {
		strerr_warn1("usage: adminclient -h adminHost -p adminPort -u adminUser -P adminPasswd [-n certfile] [-i] -c cmmd [-v]", 0);
		return (1);
	} else
	if (!admin_user && !(admin_user = (char *) env_get("ADMIN_USER"))) {
		strerr_warn1("usage: adminclient -h adminHost -p adminPort -u adminUser -P adminPasswd [-n certfile] [-i] -c cmmd [-v]", 0);
		return (1);
	} else
	if (!admin_pass && !(admin_pass = (char *) env_get("ADMIN_PASS"))) {
		strerr_warn1("usage: adminclient -h adminHost -p adminPort -u adminUser -P adminPasswd [-n certfile] [-i] -c cmmd [-v]", 0);
		return (1);
	} else
	if (!cmmd) {
		strerr_warn1("usage: adminclient -h adminHost -p adminPort -u adminUser -P adminPasswd [-n certfile] [-i] -c cmmd [-v]", 0);
		return (1);
	}
	if (verbose)
		strmsg_out7("connecting to ", admin_host, "@", admin_port, " as ", admin_user, "\n");
	if ((sfd = auth_admin(admin_user, admin_pass, admin_host, admin_port, certfile, cafile, crlfile, match_cn)) == -1) {
		strerr_warn7("adminclient: auth_admin: ", admin_host, "@", admin_port, " as ", admin_user, ": ", &strerr_tls);
		return (1);
	}
	if (verbose)
		strmsg_out1("connected\n");
	for (cmdptr1 = cmmd; *cmdptr1 && isspace((int) *cmdptr1); cmdptr1++);
	cmdptr2 = cmdptr1;
	for (; *cmdptr2 && !isspace((int) *cmdptr2); cmdptr2++);
	/*- copy program path without arguments */
	if (!stralloc_copyb(&cmdName, cmdptr1, cmdptr2 - cmdptr1) ||
			!stralloc_0(&cmdName))
		die_nomem();
	cmdName.len--;
	i = str_rchr(cmdName.s, '/');
	if (cmdName.s[i]) {
		cmdptr1 = cmdName.s + i + 1;
		cmdlen1 = cmdName.len - i - 1;
	} else {
		cmdptr1 = cmdName.s;
		cmdlen1 = cmdName.len;
	}
	for (k = 0; adminCommands[k].name; k++) {
		j = str_rchr(adminCommands[k].name, '/');
		if (adminCommands[k].name[j])
			cmdptr2 = adminCommands[k].name + j + 1;
		else
			cmdptr2 = adminCommands[k].name;
		cmdlen2 = str_len(cmdptr2);
		if (!str_diffn(cmdptr1, cmdptr2, cmdlen1 > cmdlen2 ? cmdlen1 : cmdlen2)) {
			if (!stralloc_copyb(&cmdbuf, strnum, fmt_uint(strnum, (unsigned int) k)) ||
					!stralloc_append(&cmdbuf, " ") ||
					!stralloc_cats(&cmdbuf, cmmd) ||
					!stralloc_0(&cmdbuf))
				die_nomem();
			cmdbuf.len--;
			if (verbose)
				strmsg_out5("executing command no ", strnum, " [", cmmd, "]\n");
			return (adminCmmd(sfd, input_read, cmdbuf.s, cmdbuf.len));
		}
	}
	strerr_warn2("adminclient: invalid or unauthorized command ", cmmd, 0);
	return (1);
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
