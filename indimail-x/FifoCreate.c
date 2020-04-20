/*
 * $Log: FifoCreate.c,v $
 * Revision 1.2  2019-06-03 06:50:06+05:30  Cprogrammer
 * fixed permissions of inquery fifo
 *
 * Revision 1.1  2019-04-18 07:48:31+05:30  Cprogrammer
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <error.h>
#endif
#include "indimail.h"
#include "get_indimailuidgid.h"
#include "variables.h"
#include "r_mkdir.h"

#ifndef	lint
static char     sccsid[] = "$Id: FifoCreate.c,v 1.2 2019-06-03 06:50:06+05:30 Cprogrammer Exp mbhangui $";
#endif

int
FifoCreate(char *fifoname)
{
	char           *ptr;
	struct stat     statbuf;
	int             i, status;
	int             fperms = 0660;

	errno = 0;
	if (!(status = mkfifo(fifoname, fperms))) {
		if (!getuid() || !geteuid()) {
			if (indimailuid == -1 || indimailgid == -1)
				get_indimailuidgid(&indimailuid, &indimailgid);
			if (chown(fifoname, indimailuid, indimailgid))
				return (-1);
		}
		errno = 0;
		return (0);
	} else
	if (errno == EEXIST) {
		if (stat(fifoname, &statbuf))
			return(-1);
		if (S_ISFIFO(statbuf.st_mode)) {
			if (!getuid() || !geteuid()) {
				if (indimailuid == -1 || indimailgid == -1)
					get_indimailuidgid(&indimailuid, &indimailgid);
				if (chown(fifoname, indimailuid, indimailgid))
					return (-1);
			}
			return (0);
		}
		errno = EEXIST;
		return(-1);
	} else
	if (errno == ENOENT) {
		i = str_rchr(fifoname, '/');
		if (fifoname[i]) {
			ptr = fifoname + i;
			fifoname[i] = 0;
		} else
			ptr = 0;
		if (access(fifoname, F_OK)) {
			if (indimailuid == -1 || indimailgid == -1)
				get_indimailuidgid(&indimailuid, &indimailgid);
			if (r_mkdir(fifoname, 0770, indimailuid, indimailgid)) {
				if (ptr)
					*ptr = '/';
				return (-1);
			}
			if (ptr)
				*ptr = '/';
			if (mkfifo(fifoname, fperms))
				return (-1);
			else {
				if (!getuid() || !geteuid())
					if (chown(fifoname, indimailuid, indimailgid))
						return (-1);
				return(0);
			}
		}
		if (ptr)
			*ptr = '/';
		return (-1);
	}
	return (-1);
}
