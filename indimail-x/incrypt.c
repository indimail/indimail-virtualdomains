/*
 * $Log: incrypt.c,v $
 * Revision 1.5  2023-07-17 11:44:45+05:30  Cprogrammer
 * set hash method from hash_method control file in controldir
 *
 * Revision 1.4  2023-07-16 22:40:14+05:30  Cprogrammer
 * added YESCRYPT hash
 *
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
#include <scan.h>
#endif
#include "common.h"
#include "get_hashmethod.h"

#define FATAL   "incrypt: fatal: "
#define WARN    "incrypt: warning: "

static char    *usage = "usage: incrypt [-h hash] -S salt passphrase";

int
main(int argc, char **argv)
{
	char           *ptr, *passphrase = (char *) NULL, *hash = (char *) NULL;
	char            strnum[FMT_ULONG], salt[SALTSIZE + 5];
	int             c, i, r, verbose;

	*salt = 0;
	verbose = 0;
	while ((c = getopt(argc, argv, "vh:S:")) != opteof) {
		switch (c)
		{
		case 'h':
			if ((r = scan_int(optarg, &i)) != str_len(optarg))
				i = -1;
			if (!str_diffn(optarg, "DES", 3) || i == DES_HASH)
				strnum[fmt_int(strnum, DES_HASH)] = 0;
			else
			if (!str_diffn(optarg, "MD5", 3) || i == MD5_HASH)
				strnum[fmt_int(strnum, MD5_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-256", 7) || i == SHA256_HASH)
				strnum[fmt_int(strnum, SHA256_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-512", 7) || i == SHA512_HASH)
				strnum[fmt_int(strnum, SHA512_HASH)] = 0;
			else
			if (!str_diffn(optarg, "YESCRYPT", 8) || i == YESCRYPT_HASH)
				strnum[fmt_int(strnum, YESCRYPT_HASH)] = 0;
			else {
				strerr_die5x(100, FATAL, "wrong hash method ", optarg,
						". Supported HASH Methods: DES MD5 SHA-256 SHA-512 YESCRYPT\n", usage);
			}
			hash = optarg;
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
	if (!hash && !env_get("PASSWORD_HASH")) {
		if ((i = get_hashmethod((char *) 0)) == -1)
			strerr_die2sys(111, FATAL, "get_hashmethod: ");
		strnum[fmt_int(strnum, i)] = 0;
		if (!env_put2("PASSWORD_HASH", strnum))
			strerr_die1x(111, "out of memory");
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
