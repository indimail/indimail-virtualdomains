/*
 * $Log: update_newu.c,v $
 * Revision 1.1  2019-04-18 08:33:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: update_newu.c,v 1.1 2019-04-18 08:33:45+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * Compile the users/assign file using qmail-newu program
 */
int
update_newu()
{
	int             pid;

	pid = vfork();
	if (pid == 0)
	{
		umask(022);
		execl(PREFIX"/sbin/qmail-newu", "qmail-newu", NULL);
		exit(127);
	} else
		wait(&pid);
	return (0);
}
