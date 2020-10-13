/*
 * $Log: vreorg.c,v $
 * Revision 1.5  2020-10-14 00:20:59+05:30  Cprogrammer
 * fixed infinite loop
 *
 * Revision 1.4  2020-06-16 17:56:58+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.3  2019-06-07 15:41:57+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:20:15+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.1  2019-04-18 08:38:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <setuserid.h>
#endif
#include "get_assign.h"
#include "common.h"
#include "sql_getpw.h"
#include "valias_select.h"
#include "valias_update.h"
#include "replacestr.h"
#include "MoveFile.h"
#include "sql_setpw.h"
#include "variables.h"
#include "dir_control.h"
#include "get_Mplexdir.h"
#include "open_big_dir.h"
#include "close_big_dir.h"
#include "next_big_dir.h"

#ifndef	lint
static char     sccsid[] = "$Id: vreorg.c,v 1.5 2020-10-14 00:20:59+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vreorg: fatal: "
#define WARN    "vreorg: warning: "

static char    *usage =
		"usage: vreorg -d domain_name [-r] [-R] user_list\n"
		"           -r Reset Dir Control for the entire domain\n"
		"           -R Do not Decrement Dir Control in the original filesystem"
		;

int
get_options(int argc, char **argv, char **domain, char **listfile, int *reset_dir,
	int *dec_dir, int *users_per_level)
{
	int             c;
	*listfile = *domain = 0;
	*reset_dir = 0;
	while ((c = getopt(argc, argv, "vrRd:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'd':
			*domain = optarg;
			break;
		case 'r':
			*reset_dir = 1;
			break;
		case 'l':
			scan_int(optarg, users_per_level);
			break;
		case 'R':
			*dec_dir = 0;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc)
		*listfile = argv[optind++];
	if (!*listfile) {
		strerr_warn1("must supply file containing user list", 0);
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vreorg: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	struct passwd  *pw;
	struct passwd   PwTmp;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], inbuf[512];
	char           *tmpstr, *domain, *listfile, *mplexdir, *fname;
#ifdef VALIAS
	char           *ptr;
#endif
	static stralloc OldDir = {0}, line = {0}, tmp = {0}, buf = {0};
	int             i, fd, count, reset_dir, dec_dir, users_per_level = 0, match;
	uid_t           uid, myuid;
	gid_t           gid;
	struct substdio ssin;

	dec_dir = 1;
	if (get_options(argc, argv, &domain, &listfile, &reset_dir, &dec_dir, &users_per_level))
		return (1);
	if (!get_assign(domain, 0, &uid, &gid)) {
		strerr_warn3("vreorg: ", domain, ": No such domain", 0);
		return (1);
	}
	if ((myuid = getuid()) != uid && setuser_privileges(uid, gid, "indimail")) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vreorg: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
		return (1);
	}
	/*- reset dir control external to this program */
	if (reset_dir) {
		out("vreorg", "Resetting directory layout status ");
		if (vdel_dir_control(domain))
			strerr_warn1("vreorg: vdel_dir_control failed", 0);
		out("vreorg", "done\n");
	}
	if ((fd = open_read(listfile)) == -1) {
		strerr_warn3("vreorg: open: ", listfile, ": ", &strerr_sys);
		return (1);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	out("vreorg", "Working on users ");
	flush("vreorg");
	for(count = 0;;count++) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vreorg: read: ", listfile, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (line.len == 0)
			break;
		if (!match)
			strerr_warn3("vadduser: ", listfile, ": incomplete line", 0);
		line.len--;
		line.s[line.len] = 0;
		/*- get old pw struct */
		if (!(pw = sql_getpw(line.s, domain))) {
			strerr_warn5("vreorg: ", line.s, "@", domain, ": sql_getpw failed", 0);
			continue;
		}
		if (!reset_dir && dec_dir && dec_dir_control(pw->pw_dir, line.s, domain, -1, -1))
			strerr_warn1("vreorg: dec_dir_control failed", 0);
		PwTmp = *pw;
		pw = &PwTmp;
		if (!stralloc_copys(&OldDir, pw->pw_dir) || !stralloc_0(&OldDir))
			die_nomem();
		OldDir.len--;
		/*- format new directory string */
		if (!(mplexdir = (char *) get_Mplexdir(line.s, domain, 0, uid, gid))) {
			strerr_warn1("vreorg: get_Mplexdir failed", 0);
			continue;
		}
		/*- get next dir */
		if (!(fname = open_big_dir(line.s, domain, mplexdir))) {
			strerr_warn1("vreorg: open_big_dir failed", 0);
			continue;
		}
		tmpstr = next_big_dir(uid, gid, users_per_level);
		if (close_big_dir(fname, domain, uid, gid)) {
			strerr_warn1("vreorg: close_big_dir failed", 0);
			continue;
		}
		if (*tmpstr) {
			if (!stralloc_copys(&tmp, mplexdir) ||
					!stralloc_append(&tmp, "/") ||
					!stralloc_cats(&tmp, tmpstr) ||
					!stralloc_append(&tmp, "/") ||
					!stralloc_cat(&tmp, &line) ||
					!stralloc_0(&tmp))
				die_nomem();
		} else {
			if (!stralloc_copys(&tmp, mplexdir) ||
					!stralloc_append(&tmp, "/") ||
					!stralloc_cat(&tmp, &line) ||
					!stralloc_0(&tmp))
				die_nomem();
		}
		if (!str_diffn(OldDir.s, tmp.s, OldDir.len + 1))
			continue;
		/*- Replace all occurence of OldDir with NewDir */
#ifdef VALIAS
		for(;;) {
			if (!(ptr = valias_select(line.s, domain)))
				break;
			buf.len = 0;
			if ((i = replacestr(ptr, OldDir.s, tmp.s, &buf)) == -1)
				die_nomem();
			if (!i)
				continue;
			if (valias_update(line.s, domain, ptr, buf.s))
				continue;
		}
#endif
		/*- move directory */
		if (!access(OldDir.s, F_OK) && MoveFile(OldDir.s, tmp.s)) {
			strerr_warn5("vreorg: MoveFile: ", OldDir.s, " =-> ", tmp.s, ": ", &strerr_sys);
			continue;
		}
		/*- update database */
		pw->pw_dir = tmp.s;
		if (sql_setpw(pw, domain) && !access(tmp.s, F_OK) && MoveFile(tmp.s, OldDir.s)) {
			strerr_warn5("vreorg: MoveFile: ", tmp.s, " =-> ", OldDir.s, ": ", &strerr_sys);
			continue;
		}
	}
	close(fd);
	strnum1[fmt_uint(strnum1, (unsigned int) count)] = 0;
	out("vreorg", "done ");
	out("vreorg", strnum1);
	out("vreorg", " users\n");
	flush("vreorg");
	return (0);
}
