/*
 * $Log: del_control.c,v $
 * Revision 1.2  2020-04-01 18:54:20+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:22:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <getEnvConfig.h>
#endif
#include "variables.h"
#include "remove_line.h"
#include "compile_morercpthosts.h"

#ifndef	lint
static char     sccsid[] = "$Id: del_control.c,v 1.2 2020-04-01 18:54:20+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("del_control: out of memory", 0);
	_exit(111);
}

/*
 * delete a domain from the control files
 */
int
del_control(char *domain)
{
	static stralloc filename = {0}, tmp = {0};
	char           *sysconfdir, *controldir;
	int             i, status = 0, relative;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
	if (relative) {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_catb(&filename, "/rcpthosts", 10) ||
				!stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_catb(&filename, "/rcpthosts", 10) ||
				!stralloc_0(&filename))
			die_nomem();
	}
	status = remove_line(domain, filename.s, 0, INDIMAIL_QMAIL_MODE);
	if (status < 1) { /*- if no lines found or if remove_line returned error */
		if (relative) {
			if (!stralloc_copys(&filename, sysconfdir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_cats(&filename, controldir) ||
					!stralloc_catb(&filename, "/morercpthosts", 14) ||
					!stralloc_0(&filename))
				die_nomem();
		} else {
			if (!stralloc_copys(&filename, controldir) ||
					!stralloc_catb(&filename, "/morercpthosts", 14) ||
					!stralloc_0(&filename))
				die_nomem();
		}
		/* at least one matching line found */
		if ((i = remove_line(domain, filename.s, 0, INDIMAIL_QMAIL_MODE)) > 0) {
			struct stat     statbuf;
			if (!stat(filename.s, &statbuf)) {
				if (statbuf.st_size == 0) {
					unlink(filename.s);
					filename.len--;
					if (!stralloc_catb(&filename, ".cdb", 4))
						die_nomem();
					else
					if (!stralloc_0(&filename))
						die_nomem();
					unlink(filename.s);
				} else
					compile_morercpthosts();
			}
		}
		if (i == -1)
			status = i;
	} 
	if (relative) {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_catb(&filename, "/etrnhosts", 10) ||
				!stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_catb(&filename, "/etrnhosts", 10) ||
				!stralloc_0(&filename))
			die_nomem();
	}
	if (!access(filename.s, F_OK) && remove_line(domain, filename.s, 0, INDIMAIL_QMAIL_MODE) == -1)
		status = -1;
	if (relative) {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_catb(&filename, "/chkrcptdomains", 15) ||
				!stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_catb(&filename, "/chkrcptdomains", 15) ||
				!stralloc_0(&filename))
			die_nomem();
	}
	if (!access(filename.s, F_OK) && remove_line(domain, filename.s, 0, INDIMAIL_QMAIL_MODE) == -1)
		status = -1;
	if (relative) {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_catb(&filename, "/virtualdomains", 15) ||
				!stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_catb(&filename, "/virtualdomains", 15) ||
				!stralloc_0(&filename))
			die_nomem();
	}
	if (use_etrn == 2) {
		if (!stralloc_copys(&tmp, domain) ||
				!stralloc_catb(&tmp, ":autoturn-", 10) ||
				!stralloc_0(&tmp))
			die_nomem();
	} else {
		if (!stralloc_copys(&tmp, domain) ||
				!stralloc_append(&tmp, ":") ||
				!stralloc_cats(&tmp, domain) ||
				!stralloc_0(&tmp))
			die_nomem();
	}
	if (remove_line(tmp.s, filename.s, 0, INDIMAIL_QMAIL_MODE) == -1)
		status = -1;
	return (status);
}
