/*
 * $Log: add_control.c,v $
 * Revision 1.1  2019-04-18 07:43:21+05:30  Cprogrammer
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
#include <getEnvConfig.h>
#endif
#include "count_rcpthosts.h"
#include "compile_morercpthosts.h"
#include "update_file.h"
#include "remove_line.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: add_control.c,v 1.1 2019-04-18 07:43:21+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("add_control: out of memory", 0);
	_exit(111);
}

int
add_control(char *domain, char *target)
{
	int             count, relative;
	static stralloc filename = {0}, tmpstr = {0};
	char           *sysconfdir, *controldir;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = (*controldir == '/' ? 0 : 1);
	if (relative)
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);

	/*
	 * If we have more than 50 domains in rcpthosts
	 * make a morercpthosts and compile it
	 */
	if ((count = count_rcpthosts()) == -1)
		return (-1);
	if (relative) {
		if (count >= 50) {
			if (!stralloc_copys(&filename, sysconfdir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_cats(&filename, controldir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_catb(&filename, "morercpthosts", 13) || !stralloc_0(&filename))
				die_nomem();
		} else {
			if (!stralloc_copys(&filename, sysconfdir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_cats(&filename, controldir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_catb(&filename, "rcpthosts", 9) || !stralloc_0(&filename))
				die_nomem();
		}
	} else {
		if (count >= 50) {
			if (!stralloc_copys(&filename, controldir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_catb(&filename, "morercpthosts", 13) || !stralloc_0(&filename))
				die_nomem();
		} else {
			if (!stralloc_copys(&filename, controldir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_catb(&filename, "rcpthosts", 9) || !stralloc_0(&filename))
				die_nomem();
		}
	}
	if (update_file(filename.s, domain, INDIMAIL_QMAIL_MODE))
		return (-1);
	if (count >= 50 && !OptimizeAddDomain && compile_morercpthosts()) {
		strerr_warn3("add_control: ", filename.s, ".cdb: failed to compile", 0);
		return (-1);
	}

	/*- Add to virtualdomains file and remove duplicates  and set mode */
	if (relative) {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_catb(&filename, "virtualdomains", 14) || !stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_catb(&filename, "virtualdomains", 14) || !stralloc_0(&filename))
			die_nomem();
	}
	if (target && *target) {
		if (!stralloc_copys(&tmpstr, domain) ||
				!stralloc_append(&tmpstr, ":") ||
				!stralloc_cats(&tmpstr, target) || !stralloc_0(&tmpstr))
			die_nomem();
	} else {
		if (!stralloc_copys(&tmpstr, domain) ||
				!stralloc_append(&tmpstr, ":") ||
				!stralloc_cats(&tmpstr, domain) || !stralloc_0(&tmpstr))
			die_nomem();
	}
	if (update_file(filename.s, tmpstr.s, INDIMAIL_QMAIL_MODE))
		return (-1);
	/*- make sure it's not in locals and set mode */
	if (relative)  {
		if (!stralloc_copys(&filename, sysconfdir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_cats(&filename, controldir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_catb(&filename, "locals", 6) || !stralloc_0(&filename))
			die_nomem();
	} else {
		if (!stralloc_copys(&filename, controldir) ||
				!stralloc_append(&filename, "/") ||
				!stralloc_catb(&filename, "locals", 6) || !stralloc_0(&filename))
			die_nomem();
	}
	if (remove_line(domain, filename.s, 0, INDIMAIL_QMAIL_MODE) == -1)
		return (-1);
	if (use_etrn) {
		/*- Add to etrndomains file and remove duplicates  and set mode */
		if (relative)  {
			if (!stralloc_copys(&filename, sysconfdir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_cats(&filename, controldir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_catb(&filename, "etrnhosts", 9) || !stralloc_0(&filename))
				die_nomem();
		} else {
			if (!stralloc_copys(&filename, controldir) ||
					!stralloc_append(&filename, "/") ||
					!stralloc_catb(&filename, "etrnhosts", 9) || !stralloc_0(&filename))
				die_nomem();
		}
		if (!stralloc_copys(&tmpstr, domain) || !stralloc_0(&tmpstr))
			die_nomem();
		if (update_file(filename.s, tmpstr.s, INDIMAIL_QMAIL_MODE))
			return (-1);
	}
	return (0);
}
