/*
 * $Log: incrypt.c,v $
 * Revision 1.3  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.2  2022-08-28 11:57:37+05:30  Cprogrammer
 * added option to specify salt and hash method
 *
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
#include <fmt.h>
#include <in_crypt.h>
#include <valid_password_chars.h>
#include <sgetopt.h>
#include <hashmethods.h>
#include <makesalt.h>
#include <env.h>
#endif
#include "common.h"

#define FATAL   "incrypt: fatal: "
#define WARN    "incrypt: warning: "

static char    *usage = "usage: incrypt [-h hash] -s salt passphrase";

int
main(int argc, char **argv)
{
	char           *ptr, *passphrase = (char *) NULL;
	char            strnum[FMT_ULONG], salt[SALTSIZE + 5];
	int             c, i, verbose;

	*salt = 0;
	verbose = 0;
	while ((c = getopt(argc, argv, "vh:S:")) != opteof) {
		switch (c)
		{
		case 'h':
			if (!str_diffn(optarg, "DES", 3))
				strnum[fmt_int(strnum, DES_HASH)] = 0;
			else
			if (!str_diffn(optarg, "MD5", 3))
				strnum[fmt_int(strnum, MD5_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-256", 7))
				strnum[fmt_int(strnum, SHA256_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-512", 7))
				strnum[fmt_int(strnum, SHA512_HASH)] = 0;
			else {
				strerr_warn3(WARN, optarg, ": wrong hash method. Supported [DES MD5 SHA-256 SHA-512]", 0);
				strerr_die2x(100, WARN, usage);
			}
			if (!env_put2("PASSWORD_HASH", strnum))
				strerr_die1x(111, "out of memory");
			break;
		case 'S':
			i = fmt_str(salt, optarg);
			salt[i] = 0;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_die2x(100, WARN, usage);
			break;
		}
	}
	if (!*salt) {
		if (makesalt(salt, SALTSIZE) == -1)
			strerr_die1sys(111, "failed to generate salt: ");
		if (verbose) {
			subprintfe(subfdout, "incrypt", "generated salt [%s]\n", salt);
			flush("incrypt");
		}
	} else {
		/* see crypt(5) for valid salt */
		if (salt[0] == '$' && !(salt[2] == '$' || salt[3] == '$' ||
				!str_diffn(salt, "$sha1", 5) ||
				!str_diffn(salt, "$2b$", 4) ||
				!str_diffn(salt, "$md5", 4)))
			strerr_die3x(100, FATAL, "invalid salt ", salt);
	}
	if (optind != (argc - 1))
		strerr_die2x(100, WARN, usage);
	passphrase = argv[optind++];
	if (!valid_password_chars(passphrase))
		strerr_die2x(100, FATAL, "invalid character used in passphrase");
	if (!(ptr = in_crypt(passphrase, salt)))
		strerr_die5x(100, FATAL, "failed to crypt passphrase ", passphrase, " with salt ", salt);
	subprintfe(subfdout, "incrypt", "\"%s\"\n", ptr);
	flush("incrypt");
	return(0);
}
