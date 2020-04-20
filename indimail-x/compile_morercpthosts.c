/*
 * $Log: compile_morercpthosts.c,v $
 * Revision 1.1  2019-04-18 07:44:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: compile_morercpthosts.c,v 1.1 2019-04-18 07:44:38+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * compile the morercpthosts file using qmail-newmrh program
 */
int
compile_morercpthosts()
{
	int             pid;

	if ((pid = vfork()) == 0) {
		execl(PREFIX"/sbin/qmail-newmrh", "qmail-newmrh", NULL);
		exit(127);
	} else
		wait(&pid);
	return (0);
}
