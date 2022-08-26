/*
 * $Log: incrypt.c,v $
 * Revision 1.1  2022-08-26 21:34:45+05:30  Cprogrammer
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
#include <str.h>
#include <in_crypt.h>
#include <valid_password_chars.h>
#endif

#define FATAL   "incrypt: fatal: "
#define WARN    "incrypt: warning: "

int
main(int argc, char **argv)
{
	char           *ptr, *passphrase, *salt;

	if (argc != 3)
		strerr_die2x(100, WARN, "usage: incrypt cleartxt salt");
	passphrase = argv[1];
	salt = argv[2];
	/* see crypt(5) for valid salt */
	if (salt[0] == '$' && !(salt[2] == '$' || salt[3] == '$' ||
				!str_diffn(salt, "$sha1", 5) ||
				!str_diffn(salt, "$2b$", 4) ||
				!str_diffn(salt, "$md5", 4)))
			strerr_die3x(100, FATAL, "invalid salt ", salt);
	if (!valid_password_chars(passphrase))
		strerr_die2x(100, FATAL, "invalid character used in passphrase");
	if (!(ptr = in_crypt(passphrase, salt)))
		strerr_die5x(100, FATAL, "failed to crypt passphrase ", passphrase, " with salt ", salt);
	if (substdio_put(subfdout, "\"", 1) ||
			substdio_puts(subfdout, ptr) ||
			substdio_put(subfdout, "\"\n", 2) ||
			substdio_flush(subfdout))
		strerr_die2sys(111, WARN, "unable to write to stdout");
	return(0);
}
