/*
 * $Log: crypt.c,v $
 * Revision 1.1  2019-04-14 13:23:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#else
char           *crypt(const char *, const char *);
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <substdio.h>
#include <subfd.h>
#endif

int
main(int argc, char **argv)
{
	if (argc != 3) {
		strerr_warn1("Usage: crypt <key> <salt>", 0);
		_exit(100);
	}
	if (substdio_put(subfdout, "\"", 1) ||
			substdio_puts(subfdout, crypt(*(argv + 1), *(argv + 2))) ||
			substdio_put(subfdout, "\"\n", 2) ||
			substdio_flush(subfdout))
	{
		strerr_warn1("crypt: unable to write to stdout", 0);
		_exit(111);
	}
	return(0);
}
