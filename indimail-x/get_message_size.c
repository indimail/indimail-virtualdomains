/*
 * $Log: get_message_size.c,v $
 * Revision 1.1  2019-04-18 08:17:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#endif
#include "indimail.h"


/*-
 * Get the size of the email message
 * return the size
 */
mdir_t
get_message_size()
{
	struct stat     statbuf;

	if (fstat(0, &statbuf)) {
		strerr_warn1("deliver_mail: fstat: ", &strerr_sys);
		return (-1);
	}
	return (statbuf.st_size);
}
