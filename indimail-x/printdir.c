/*
 * $Log: printdir.c,v $
 * Revision 1.3  2021-07-08 11:44:23+05:30  Cprogrammer
 * add check for misconfigured assign file
 *
 * Revision 1.2  2020-06-16 17:55:46+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.1  2019-04-14 21:03:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <open.h>
#include <error.h>
#include <substdio.h>
#include <getln.h>
#include <scan.h>
#include <setuserid.h>
#endif
#include "get_indimailuidgid.h"
#include "get_assign.h"
#include "variables.h"
#include "print_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: printdir.c,v 1.3 2021-07-08 11:44:23+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("printdir: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	char           *ptr;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], inbuf[512];
	static stralloc tmpbuf = {0}, line = {0};
	int             i, fd, match, users_per_level = 0;
	struct substdio ssin;
	uid_t           uid, myuid;
	gid_t           gid;

	ptr = argv[0];
	i = str_rchr(ptr, '/');
	if (ptr[i])
		ptr = argv[0] + i + 1;
	if (argc != 2) {
		strerr_warn3("usage: ", ptr, " domain_name", 0);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	if (!(ptr = get_assign(argv[1], 0, &uid, &gid))) {
		strerr_warn3("printdir: domain ", argv[1], " does not exist", 0);
		return (-1);
	}
	if (!uid)
		strerr_die3x(100, "printdir: domain ", argv[1], " with uid 0");
	if ((myuid = getuid()) != uid) {
		if (setuser_privileges(uid, gid, "indimail")) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn5("printdir: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
			return (1);
		}
	}
	if (!stralloc_copys(&tmpbuf, ptr) ||
			!stralloc_catb(&tmpbuf, "/.users_per_level", 17) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent) {
			strerr_warn3("printdir: ", tmpbuf.s, ": ", &strerr_sys);
			return (1);
		}
	} else {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("printdir: read: ", tmpbuf.s, ": ", &strerr_sys);
		} else
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		if (line.len) {
			scan_int(line.s, &users_per_level);
		}
		close(fd);
	}
	if (!stralloc_copys(&tmpbuf, ptr) ||
			!stralloc_catb(&tmpbuf, "/.filesystems", 13) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	print_control(tmpbuf.s, argv[1], users_per_level, 0);
	return (0);
}
