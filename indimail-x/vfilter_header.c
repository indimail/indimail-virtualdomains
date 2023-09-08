/*
 * $Log: vfilter_header.c,v $
 * Revision 1.4  2023-09-05 21:50:44+05:30  Cprogrammer
 * added headerNumber function to convert textual header name to number
 *
 * Revision 1.3  2023-03-20 10:36:29+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.2  2020-04-01 18:58:46+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:33:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_header.c,v 1.4 2023-09-05 21:50:44+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <error.h>
#include <alloc.h>
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#include <open.h>
#include <getln.h>
#include <substdio.h>
#include <getEnvConfig.h>
#endif
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_header: out of memory", 0);
	_exit(111);
}

int
headerNumber(char **hlist, char *headerName)
{
	int             i, len;

	len = str_len(headerName);
	for (i = 0; hlist[i]; i++)
		if (!str_diffn(headerName, hlist[i], len + 1))
			return i;
	return -1;
}

char          **
headerList()
{
	static stralloc line = {0}, filename = {0};
	int             count, len, fd, match;
	char           *ptr, *sysconfdir;
	char          **hptr;
	struct substdio ssin;
	char            inbuf[4096];

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	if (!stralloc_copys(&filename, sysconfdir) ||
			!stralloc_catb(&filename, "/headerlist", 11) ||
			!stralloc_0(&filename))
		die_nomem();
	if ((fd = open_read(filename.s)) == -1) {
		strerr_warn3("vfilter_header: ", filename.s, ": ", &strerr_sys);
		return ((char **) 0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (count = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vfilter_header: read: ", filename.s, ": ", &strerr_sys);
			close(fd);
			return ((char **) 0);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		count++;
	}
	if (!(hptr = (char **) alloc(sizeof(char *) * (count + 1)))) {
		strerr_warn1("headerList: alloc: ", &strerr_sys);
		close(fd);
		return ((char **) 0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if (lseek(fd, 0, SEEK_SET) == -1) {
		strerr_warn3("vfilter_header: lseek: ", filename.s, ": ", &strerr_sys);
		close(fd);
		return ((char **) 0);
	}
	ssin.p = 0; /*- reset position to beginning of file */
	ssin.n = sizeof(inbuf);
	for (count = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vfilter_header: read: ", filename.s, ": ", &strerr_sys);
			close(fd);
			return ((char **) 0);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		len = ptr == line.s ? line.len + 1 : str_len(ptr) + 1;
		if (!(hptr[count] = (char *) alloc(sizeof(char) * len))) {
			strerr_warn1("headerList: alloc: ", &strerr_sys);
			close(fd);
			return ((char **) 0);
		}
		str_copyb(hptr[count++], ptr, len);
	}
	close(fd);
	hptr[count++] = 0;
	return (hptr);
}
#endif
