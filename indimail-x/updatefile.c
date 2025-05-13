/*
 * $Id: updatefile.c,v 1.3 2025-05-13 20:03:42+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <subfd.h>
#include <substdio.h>
#include <getln.h>
#include <scan.h>
#include <open.h>
#endif
#include "variables.h"
#include "remove_line.h"
#include "update_file.h"

#ifndef	lint
static char     sccsid[] = "$Id: updatefile.c,v 1.3 2025-05-13 20:03:42+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "updatefile: fatal: "
#define WARN    "udpatefile: warning: "

static char    *usage =
		"USAGE: updatefile [-s] [-a] [-u updateline] [-d deleteline] -m mode filename\n"
		"-s Display File\n"
		"-u updateline - Update File with updateline\n"
		"-d deleteline - Delete line deleteline (first match)\n"
		"-a Delete all matching Lines\n"
		"-m mode       - Set permission on file"
		;

static int
get_options(int argc, char **argv, char **filename, char **updateLine, char **deleteLine, unsigned int *mode, int *display, int *all)
{
	int             c;

	*all = 0;*filename = *updateLine = *deleteLine = 0;
	*mode = 0644;
	*display = 0;
	while ((c = getopt(argc, argv, "vsau:d:m:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'a':
			*all = 1;
			break;
		case 's':
			*display = 1;
			break;
		case 'u':
			*updateLine = optarg;
			break;
		case 'd':
			*deleteLine = optarg;
			break;
		case 'm':
			scan_8long(optarg, (unsigned long *) mode);
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc)
		*filename = argv[optind];
	if (!*filename || !*mode) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

int
main(int argc, char **argv)
{
	unsigned int    mode;
	int             display, all, fd, match;
	char          *filename, *updateline, *deleteline;
	static stralloc line = {0};
	char            inbuf[4096];
	struct substdio ssin;

	filename = updateline = deleteline = (char *) 0;
	if (get_options(argc, argv, &filename, &updateline, &deleteline, &mode, &display, &all))
		return (1);
	if (deleteline && remove_line(deleteline, filename, all == 1 ? 0 : 1, mode) == -1)
		return (1);
	if (updateline && update_file(filename, updateline, mode))
		return (1);
	if (display) {
		if ((fd = open_read(filename)) == -1) {
			strerr_die3sys(111, "updatefile: open: ", filename, ": ");
			return (0);
		}
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		for(;;) {
			if (getln(&ssin, &line, &match, '\n') == -1)
				strerr_die3sys(111, "udpatefile: read: ", filename, ": ");
			if (line.len == 0)
				break;
			if (substdio_put(subfdoutsmall, line.s, line.len))
				strerr_die1sys(111, "udpatefile: unable to write to stdout: ");
		}
		if (substdio_flush(subfdoutsmall))
			strerr_die1sys(111, "udpatefile: unable to write to stdout: ");
		close(fd);
	}
	return (0);
}
/*
 * $Log: updatefile.c,v $
 * Revision 1.3  2025-05-13 20:03:42+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.2  2019-06-07 16:10:05+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:33:41+05:30  Cprogrammer
 * Initial revision
 *
 */
