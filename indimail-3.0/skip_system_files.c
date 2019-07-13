/*
 * $Log: skip_system_files.c,v $
 * Revision 1.1  2019-04-13 20:19:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: skip_system_files.c,v 1.1 2019-04-13 20:19:47+05:30 Cprogrammer Exp mbhangui $";
#endif

int
skip_system_files(char *filename)
{
	char           *system_files[] = {
		".Trash",
		".current_size",
		"domain",
		"QuotaWarn",
		"vfilter",
		"folder.dateformat",
		"noprefilt",
		"nopostfilt",
		"BulkMail",
		"deliveryCount", 
		"maildirfolder",
		"maildirsize",
		"core",
		"sqwebmail",
		"courier",
		"shared-maildirs",
		"shared-timestamp",
		"shared-folders",
		0,
	};
	char          **ptr;
	int             len;

	for (ptr = system_files; ptr && *ptr; ptr++) {
		len = str_len(*ptr);
		if (!str_diffn(filename, *ptr, len + 1))
			return (1);
	}
	return (0);
}
