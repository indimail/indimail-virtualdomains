/*
 * $Id: vrenameuser.c,v 1.8 2025-05-13 20:37:40+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <env.h>
#include <strerr.h>
#include <fmt.h>
#include <env.h>
#include <sgetopt.h>
#include <setuserid.h>
#include <substdio.h>
#include <strerr.h>
#include <error.h>
#include <open.h>
#include <getln.h>
#endif
#include "parse_email.h"
#include "get_assign.h"
#include "get_indimailuidgid.h"
#include "variables.h"
#include "check_group.h"
#include "iclose.h"
#include "renameuser.h"
#include "post_handle.h"

#ifndef	lint
static char     sccsid[] = "$Id: vrenameuser.c,v 1.8 2025-05-13 20:37:40+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vrenameuser: fatal: "
#define WARN    "vrenameuser: warning: "

static char    *usage =
	"usage: vrenameuser [options] old_email_address new_email_address\n"
	"options: -V (print version number)\n"
	"options: -v (verbose)"
	;

int
get_options(int argc, char **argv, char **oldEmail, char **newEmail)
{
	int             c;

	*oldEmail = *newEmail = 0;
	while ((c = getopt(argc, argv, "v")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}

	if (optind < argc)
		*oldEmail = argv[optind++];
	if (optind < argc)
		*newEmail = argv[optind++];
	if (!*oldEmail || !*newEmail) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

void
set_basepath(char *domain_dir)
{
	static stralloc tmpbuf = {0}, line = {0}, envbuf = {0};
	int             fd, match;
	struct substdio ssin;
	char            inbuf[512];

	if (!stralloc_copys(&tmpbuf, domain_dir) ||
			!stralloc_catb(&tmpbuf, "/.base_path", 11) ||
			!stralloc_0(&tmpbuf))
		strerr_die1x(111, "vrenameuser: out of memory");
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "vrenameuser: open: ", tmpbuf.s, ": ");
	} else {
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die3sys(111, "vrenameuser: read: ", tmpbuf.s, ": ");
		close(fd);
		if (line.len < 3)
			strerr_die3x(1, "vrenameuser", tmpbuf.s, ": incomplete line");
		if (match)
			line.len--;
		if (!stralloc_copy(&envbuf, &line) || !stralloc_0(&envbuf))
			strerr_die1x(111, "vrenameuser: out of memory");
	}
	if (envbuf.len && !env_put2("BASE_PATH", envbuf.s))
		strerr_die1x(111, "vrenameuser: out of memory");
}

int
main(int argc, char **argv)
{
	uid_t           uid1, uid2, myuid;
	gid_t           gid1, gid2, mygid;
	static stralloc oldUser = {0}, oldDomain = {0}, newUser = {0}, newDomain = {0}, Dir = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	char           *ptr, *base_argv0, *oldEmail, *newEmail;
	int             i;

	if(get_options(argc, argv, &oldEmail, &newEmail))
		return (1);
	parse_email(oldEmail, &oldUser, &oldDomain);
	parse_email(newEmail, &newUser, &newDomain);
	if(!get_assign(oldDomain.s, &Dir, &uid1, &gid1))
		strerr_die3x(1, WARN, oldDomain.s, ": domain does not exist");
	if (!uid1)
		strerr_die4x(100, WARN, "domain ", oldDomain.s, " with uid 0");
	if(!get_assign(newDomain.s, 0, &uid2, &gid2))
		strerr_die3x(1, WARN, newDomain.s, ": domain does not exist");
	if (!uid2)
		strerr_die4x(100, WARN, "domain ", newDomain.s, " with uid 0");
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	myuid = getuid();
	mygid = getgid();
	if (myuid != 0 && myuid != indimailuid && mygid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
		strerr_die6x(100, WARN, "you must be root or domain user (uid=", strnum1, ", gid=", strnum2, ") to run this program");
	}
	if (uid1 != uid2 && setuid(0))
		strerr_die2sys(111, FATAL, "setuid-root: ");
	else {
		if (setuser_privileges(uid1, gid1, "indimail")) {
			strnum1[fmt_ulong(strnum1, uid1)] = 0;
			strnum2[fmt_ulong(strnum2, gid1)] = 0;
			strerr_die6sys(111, FATAL, "setuser_privilege: (", strnum1, "/", strnum2, "): ");
		}
	}
	set_basepath(Dir.s);
	if (renameuser(&oldUser, &oldDomain, &newUser, &newDomain)) {
		iclose();
		return (1);
	}
	iclose();
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s %s", LIBEXECDIR, base_argv0, oldEmail, newEmail));
	} else {
		if (uid1 != uid2 && setuser_privileges(myuid, mygid, "indimail")) {
			strnum1[fmt_ulong(strnum1, myuid)] = 0;
			strnum2[fmt_ulong(strnum2, mygid)] = 0;
			strerr_die6sys(111, FATAL, "setuser_privilege: (", strnum1, "/", strnum2, "): ");
		}
		return (post_handle("%s %s %s", ptr, oldEmail, newEmail));
	}
}
/*
 * $Log: vrenameuser.c,v $
 * Revision 1.8  2025-05-13 20:37:40+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.7  2023-03-23 22:25:30+05:30  Cprogrammer
 * set BASE PATH of original domain for the new user
 *
 * Revision 1.6  2023-03-22 14:49:10+05:30  Cprogrammer
 * run POST_HANDLE program (if set) with domain user uid/gid
 *
 * Revision 1.5  2022-10-20 11:59:15+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.4  2021-07-08 11:50:02+05:30  Cprogrammer
 * add check for misconfigured assign file
 *
 * Revision 1.3  2020-06-16 17:56:51+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.2  2019-06-07 15:42:29+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:33:39+05:30  Cprogrammer
 * Initial revision
 *
 */
