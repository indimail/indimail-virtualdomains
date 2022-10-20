/*
 * $Log: fappend.c,v $
 * Revision 1.2  2022-10-20 11:57:33+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.1  2019-04-18 08:35:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <fmt.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: fappend.c,v 1.2 2022-10-20 11:57:33+05:30 Cprogrammer Exp mbhangui $";
#endif

/* function to append a file to another file */
int
fappend(char *readfile, char *appndfile, char *mode, mode_t perm,
		uid_t uid, gid_t gid)
{
	char            buffer[2048], strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             fin, fout;
#ifdef SUN41
	int             num;
#else
	unsigned        num;
#endif

#if defined(CYGWIN) || defined(WindowsNT)
	if ((fin = open(readfile, O_RDONLY|O_BINARY, 0)) == -1)
#else
	if ((fin = open(readfile, O_RDONLY, 0)) == -1)
#endif
		return (-1);
	if (*mode == 'w') {
#if defined(CYGWIN) || defined(WindowsNT)
		if ((fout = open(appndfile, O_CREAT|O_TRUNC|O_BINARY|O_WRONLY, perm)) == -1)
			return (-1);
#else
		if ((fout = open(appndfile, O_CREAT|O_TRUNC|O_WRONLY, perm)) == -1)
			return (-1);
#endif
	} else {
#if defined(CYGWIN) || defined(WindowsNT)
	if ((fout = open(appndfile, O_CREAT|O_APPEND|O_BINARY|O_WRONLY, perm)) == -1)
#else
	if ((fout = open(appndfile, O_CREAT|O_APPEND|O_WRONLY, perm)) == -1)
#endif
		return (-1);
	}
	if (chown(appndfile, uid, gid)) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn7("fappend: chown: ", appndfile, " (", strnum1, " ", strnum2, "): ", &strerr_sys);
	}
	while ((num = read(fin, buffer, sizeof(buffer))) >= 1)
		if (write(fout, buffer, num) != num) {
			(void) close(fin);
			(void) close(fout);
			return (-1);
		}
	(void) close(fin);
	(void) close(fout);
	return (0);
}
