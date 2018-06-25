/*
 * $Log: add_control.c,v $
 * Revision 2.7  2017-03-13 13:33:39+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.6  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.5  2009-01-15 08:54:36+05:30  Cprogrammer
 * change for once_only flag in remove_line
 *
 * Revision 2.4  2008-08-02 09:05:13+05:30  Cprogrammer
 * new function error_stack
 *
 * Revision 2.3  2005-12-29 22:38:39+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-05-17 13:59:56+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.1  2002-08-25 22:48:30+05:30  Cprogrammer
 * *** empty log message ***
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: add_control.c,v 2.7 2017-03-13 13:33:39+05:30 Cprogrammer Stab mbhangui $";
#endif

int
add_control(char *domain, char *target)
{
	int             count, relative;
	char            filename[MAX_BUFF], tmpstr[MAX_BUFF];
	char           *sysconfdir, *controldir;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = (*controldir == '/' ? 0 : 1);

	/*
	 * If we have more than 50 domains in rcpthosts
	 * make a morercpthosts and compile it
	 */
	if (relative) {
		if ((count = count_rcpthosts()) >= 50)
			snprintf(filename, MAX_BUFF, "%s/%s/morercpthosts", sysconfdir, controldir);
		else
			snprintf(filename, MAX_BUFF, "%s/%s/rcpthosts", sysconfdir, controldir);
	} else {
		if ((count = count_rcpthosts()) >= 50)
			snprintf(filename, MAX_BUFF, "%s/morercpthosts", controldir);
		else
			snprintf(filename, MAX_BUFF, "%s/rcpthosts", controldir);
	}
	if(update_file(filename, domain, INDIMAIL_QMAIL_MODE))
		return (-1);
	if (count >= 50 && !OptimizeAddDomain && compile_morercpthosts())
	{
		error_stack(stderr, "%s.cdb: failed to compile\n", filename);
		return (-1);
	}
	/*
	 * Add to virtualdomains file and remove duplicates  and set mode 
	 */
	if (relative) 
		snprintf(filename, MAX_BUFF, "%s/%s/virtualdomains", sysconfdir, controldir);
	else
		snprintf(filename, MAX_BUFF, "%s/virtualdomains", controldir);
	if(target && *target)
		snprintf(tmpstr, MAX_BUFF, "%s:%s", domain, target);
	else
		snprintf(tmpstr, MAX_BUFF, "%s:%s", domain, domain);
	if(update_file(filename, tmpstr, INDIMAIL_QMAIL_MODE))
		return (-1);
	/*
	 * make sure it's not in locals and set mode 
	 */
	if (relative) 
		snprintf(filename, MAX_BUFF, "%s/%s/locals", sysconfdir, controldir);
	else
		snprintf(filename, MAX_BUFF, "%s/locals", controldir);
	if(remove_line(domain, filename, 0, INDIMAIL_QMAIL_MODE) == -1)
		return (-1);
	if(use_etrn)
	{
		/*
		 * Add to etrndomains file and remove duplicates  and set mode 
		 */
		if (relative) 
			snprintf(filename, MAX_BUFF, "%s/%s/etrnhosts", sysconfdir, controldir);
		else
			snprintf(filename, MAX_BUFF, "%s/etrnhosts", controldir);
		snprintf(tmpstr, MAX_BUFF, "%s", domain);
		if(update_file(filename, tmpstr, INDIMAIL_QMAIL_MODE))
			return (-1);
	}
	return (0);
}

void
getversion_add_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}