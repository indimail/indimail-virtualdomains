/*
 * $Log: add_domain_assign.c,v $
 * Revision 1.3  2021-07-08 11:30:50+05:30  Cprogrammer
 * removed QMAILDIR setting through env variable
 *
 * Revision 1.2  2020-04-01 18:52:28+05:30  Cprogrammer
 * moved getEnvConfig to libqmail
 *
 * Revision 1.1  2019-04-14 18:34:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#include <open.h>
#include <getEnvConfig.h>
#endif
#include "get_assign.h"
#include "get_indimailuidgid.h"
#include "update_newu.h"
#include "update_file.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: add_domain_assign.c,v 1.3 2021-07-08 11:30:50+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("add_domain_assign: out of memory", 0);
	_exit(111);
}

/*-
 * Add a domain to all the control files 
 * And signal qmail
 */
int
add_domain_assign(char *domain, char *domain_base_dir, uid_t uid, gid_t gid)
{
	static stralloc filename = {0}, tmpstr = {0};
	int             fd;
	char           *assigndir;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             i, j;

	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	/*- stat assign file, if it's not there create one */
	if (!stralloc_copys(&filename, assigndir) ||
			!stralloc_catb(&filename, "/assign", 7) ||
			!stralloc_0(&filename))
		die_nomem();
	if (access(filename.s, F_OK)) {
		/*- put a . on one line by itself */
		if ((fd = open_trunc(filename.s)) == -1) {
			strerr_warn3("add_domain_assign: open: ", filename.s, ": ", &strerr_sys);
			return (-1);
		}
		if (write(fd, ".\n", 2) != 2) {
			strerr_warn3("add_domain_assign: write: ", filename.s, ": ", &strerr_sys);
			return (-1);
		}
		if (close(fd) == -1) {
			strerr_warn3("add_domain_assign: write: ", filename.s, ": ", &strerr_sys);
			return (-1);
		}
	}
	if (use_etrn == 2  && !get_assign("autoturn", 0, 0, 0)) {
		get_indimailuidgid(&indimailuid, &indimailgid);
		strnum1[i = fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[j = fmt_ulong(strnum2, indimailgid)] = 0;
		if (!stralloc_copyb(&tmpstr, "+autoturn-:indimail:", 20) ||
				!stralloc_catb(&tmpstr, strnum1, i) ||
				!stralloc_append(&tmpstr, ":") ||
				!stralloc_catb(&tmpstr, strnum2, j) ||
				!stralloc_append(&tmpstr, ":") ||
				!stralloc_cats(&tmpstr, QMAILDIR) ||
				!stralloc_catb(&tmpstr, "/autoturn:-::", 13) ||
				!stralloc_0(&tmpstr))
			die_nomem();
		/*- update the file and add the above line and remove duplicates */
		if (update_file(filename.s, tmpstr.s, INDIMAIL_QMAIL_MODE))
			return (-1);
		if (!OptimizeAddDomain)
			update_newu();
	} else {
		strnum1[i = fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[j = fmt_ulong(strnum2, indimailgid)] = 0;
		if (!stralloc_copyb(&tmpstr, "+", 1) ||
				!stralloc_cats(&tmpstr, domain) ||
				!stralloc_catb(&tmpstr, "-:", 2) ||
				!stralloc_cats(&tmpstr, domain) ||
				!stralloc_append(&tmpstr, ":") ||
				!stralloc_catb(&tmpstr, strnum1, i) ||
				!stralloc_append(&tmpstr, ":") ||
				!stralloc_catb(&tmpstr, strnum2, j) ||
				!stralloc_append(&tmpstr, ":") ||
				!stralloc_cats(&tmpstr, domain_base_dir) ||
				!stralloc_catb(&tmpstr, use_etrn ? "/" : "/domains/", use_etrn ? 1 : 9) ||
				!stralloc_cats(&tmpstr, domain) ||
				!stralloc_catb(&tmpstr, ":-::", 4) ||
				!stralloc_0(&tmpstr))
			die_nomem();
		if (update_file(filename.s, tmpstr.s, INDIMAIL_QMAIL_MODE))
			return (-1);
		/*- compile the assign file */
		if (!OptimizeAddDomain)
			update_newu();
	}
	return(0);
}
