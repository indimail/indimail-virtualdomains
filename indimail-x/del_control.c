/*
 * $Log: del_control.c,v $
 * Revision 1.5  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.4  2023-12-03 16:10:48+05:30  Cprogrammer
 * use same logic for ETRN, ATRN domains
 *
 * Revision 1.3  2023-03-25 14:15:26+05:30  Cprogrammer
 * refactored code
 *
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
#include <error.h>
#include <str.h>
#endif
#include "variables.h"
#include "remove_line.h"
#include "compile_morercpthosts.h"

#ifndef	lint
static char     sccsid[] = "$Id: del_control.c,v 1.5 2024-05-17 16:25:48+05:30 mbhangui Exp mbhangui $";
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
del_control(const char *domain)
{
	static stralloc filename = {0}, tmp = {0};
	char           *sysconfdir, *controldir;
	int             i, status = 0, relative, len;
	char          **ptr;
	char           *fn[] = {"rcpthosts", "etrnhosts", "chkrcptdomains", "virtualdomains", 0};

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
	if (relative) {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_append(&filename, "/"))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_append(&filename, "/"))
			die_nomem();
	}
	len = filename.len;
	for (ptr = fn; *ptr; ptr++) {
		if (!stralloc_cats(&filename, *ptr) ||
				!stralloc_0(&filename))
			die_nomem();
		if (access(filename.s, F_OK)) {
			if (errno != error_noent)
				strerr_warn3("del_control: ", filename.s, ": ", &strerr_sys);
			filename.len = len; /*- restore original length */
			continue;
		}
		status = remove_line(domain, filename.s, 1, INDIMAIL_QMAIL_MODE);
		if (!str_diffn(*ptr, "rcpthosts", 10) && status < 1) { /*- remove from morercpthosts */
			/*- if no lines found or if remove_line returned error */
			if (!stralloc_catb(&filename, "morercpthosts\0", 14) )
				die_nomem();
			if (access(filename.s, F_OK)) {
				if (errno != error_noent)
					strerr_warn3("del_control: ", filename.s, ": ", &strerr_sys);
			} else {
				if ((i = remove_line(domain, filename.s, 0, INDIMAIL_QMAIL_MODE)) > 0) {
					/* at least one matching line found */
					struct stat     statbuf;
					if (!stat(filename.s, &statbuf)) {
						if (statbuf.st_size == 0) {
							unlink(filename.s);
							filename.len--;
							if (!stralloc_catb(&filename, ".cdb\0", 5))
								die_nomem();
							unlink(filename.s);
						} else
							compile_morercpthosts();
					}
				}
				if (i == -1)
					status = i;
			}
		}
		filename.len = len; /*- restore original length */
	}

	if (!stralloc_catb(&filename, "virtualdomains\0", 15) )
		die_nomem();
	if (use_etrn) {
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
	if (access(filename.s, F_OK)) {
		if (errno != error_noent)
			status = -1;
	} else
	if (remove_line_p(tmp.s, filename.s, 0, INDIMAIL_QMAIL_MODE) == -1)
		status = -1;
	return (status);
}
