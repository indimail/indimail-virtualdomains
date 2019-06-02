/*
 * $Log: sockwrite.c,v $
 * Revision 1.1  2019-04-18 08:37:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: sockwrite.c,v 1.1 2019-04-18 08:37:52+05:30 Cprogrammer Exp mbhangui $";
#endif

ssize_t
sockwrite(int fd, char *wbuf, int len)
{
	char           *ptr;
	int             rembytes, wbytes;

	for (ptr = wbuf, rembytes = len; rembytes;) {
		for (;;) {
			errno = 0;
			if ((wbytes = write(fd, ptr, rembytes)) == -1) {
#ifdef ERESTART
				if (errno == EINTR || errno == ERESTART)
#else
				if (errno == EINTR)
#endif
					continue;
				return (-1);
			} else
			if (!wbytes)
				return(0);
			break;
		}
		ptr += wbytes;
		rembytes -= wbytes;
	}
	return (len);
}
