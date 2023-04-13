/*
 * $Log: showbytes.c,v $
 * Revision 1.4  2021-04-05 21:58:15+05:30  Cprogrammer
 * fixed compilation errors
 *
 * Revision 1.3  2020-06-21 12:49:33+05:30  Cprogrammer
 * quench rpmlint
 *
 * Revision 1.2  2013-05-15 00:16:21+05:30  Cprogrammer
 * fixed warnings
 *
 * Revision 1.1  2013-03-15 09:30:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <open.h>

#ifndef	lint
static char     rcsid[] = "$Id: showbytes.c,v 1.4 2021-04-05 21:58:15+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL "showbytes: fatal: "
#define WARN  "showbytes: warn: "

int
main(int argc, char **argv)
{
	size_t          bytes;
	int             fd;

	if (argc != 2)
		strerr_die1x(100, "usage: showbytes statusfile");
	if ((fd = open_read(argv[1])) == -1)
		strerr_die3sys(111, FATAL, argv[1], ": ");
	if (lseek(fd, sizeof(pid_t), SEEK_SET) == -1)
		strerr_die4sys(111, FATAL, "unable to rewind ", argv[1], ": ");
	if (read(fd, &bytes, sizeof(bytes)) == -1)
		strerr_die4sys(111, FATAL, "read: ", argv[1], ": ");
	close(fd);
	subprintf(subfdout, "%d\n", (int) bytes);
	_exit(substdio_flush(subfdout));
}

#ifndef	lint
void
getversion_showbytes_c()
{
	char           *x;

	x = rcsid;
	x++;
}
#endif
