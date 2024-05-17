/*
 * $Log: addaliasdomain.c,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2019-04-14 23:18:40+05:30  Cprogrammer
 * changed mode to 0640
 *
 * Revision 1.1  2019-04-14 21:02:45+05:30  Cprogrammer
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
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <open.h>
#endif
#include "get_assign.h"
#include "is_distributed_domain.h"
#include "open_master.h"
#include "sql_insertaliasdomain.h"
#include "add_domain_assign.h"
#include "add_control.h"
#include "update_file.h"

#ifndef	lint
static char     sccsid[] = "$Id: addaliasdomain.c,v 1.3 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("addaliasdomain: out of memory", 0);
	_exit(111);
}

int
addaliasdomain(const char *old_domain, const char *new_domain)
{
	static stralloc dirstr = {0}, tmpbuf = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	char           *ptr;
	int             i, domain_len, fdsourcedir;
	uid_t           uid;
	gid_t           gid;

	if (!new_domain || !*new_domain || *new_domain == '-' || !old_domain || !*old_domain) {
		strerr_warn1("addaliasdomain: Invalid Domain Name", 0);
		return (-1);
	}
	if ((fdsourcedir = open_read(".")) == -1)
		strerr_die1sys(111, "addaliasdomain: unable to open current directory: ");
	if (get_assign(new_domain, 0, 0, 0)) {
		strerr_warn3("addaliasdomain: domain ", new_domain, " exists", 0);
		return (-1);
	}
	if (!get_assign(old_domain, &dirstr, &uid, &gid)) {
		strerr_warn3("addaliasdomain: Domain ", old_domain, " does not exist", 0);
		return (-1);
	}
#ifdef CLUSTERED_SITE
	if ((i = is_distributed_domain(old_domain)) == -1) {
		strerr_warn2(old_domain, ": is_distributed_domain failed", 0);
		return (-1);
	} else
	if (i) {
		if (open_master()) {
			strerr_warn1("addaliasdomain: failed to open master db", 0);
			return (-1);
		}
		if (sql_insertaliasdomain(old_domain, new_domain))
			return (-1);
	}
#endif
	if (!stralloc_copy(&tmpbuf, &dirstr) || !stralloc_0(&tmpbuf))
		die_nomem();
	tmpbuf.len--;
	domain_len = str_len(old_domain);
	tmpbuf.len -= domain_len; /*- remove domain component to get base domain dir */
	tmpbuf.s[tmpbuf.len - 1] = 0;
	if (chdir(tmpbuf.s)) {
		strerr_warn3("addaliasdomain: chdir: ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	tmpbuf.s[tmpbuf.len - 1] = '/';
	if (symlink(old_domain, new_domain)) {
		strerr_warn5("addaliasdomain: symlink: ", new_domain, " -> ", old_domain, ": ", &strerr_sys);
		return (-1);
	}
	if (fchdir(fdsourcedir) == -1)
		strerr_die1sys(111, "addaliasdomain: unable to switch back to old working directory: ");
	if ((ptr = str_str(dirstr.s, "/domains/")))
		*ptr = 0;
	else {
		strerr_warn3("addaliasdomain: invalid domain dir [", dirstr.s, "]", 0);
		return (1);
	}
	if (add_domain_assign(new_domain, dirstr.s, uid, gid))
		return (-1);
	if (add_control(new_domain, 0))
		return (-1);
	*ptr = '/'; /*- restore directory stored */
	tmpbuf.len += domain_len;
	if (!stralloc_catb(&tmpbuf, "/.aliasdomains", 14) || !stralloc_0(&tmpbuf))
		die_nomem();
	if (update_file(tmpbuf.s, new_domain, 0640))
		return (-1);
	if (chown(tmpbuf.s, uid, gid)) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn7("addaliasdomain: chown/chmod ", tmpbuf.s, " (", strnum1, "/", strnum2, "/0600)", 0);
		return (-1);
	}
	return (0);
}
