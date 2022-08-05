/*
 * $Log: gsasl_mkpasswd.c,v $
 * Revision 1.2  2022-08-05 23:39:15+05:30  Cprogrammer
 * compile gsasl code for libgsasl version >= 1.8.1
 *
 * Revision 1.1  2022-08-05 20:58:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_GSASL_H
#include <gsasl.h>
#endif
#include <str.h>
#include <sodium_random.h>
#include <base64.h>
#include <stralloc.h>
#include <alloc.h>
#include <fmt.h>
#include <byte.h>
#include "common.h"

#ifdef HAVE_GSASL
#define DEFAULT_SALT_SIZE 12

#define USAGE_ERR  1
#define MEMORY_ERR 2
#define SODIUM_ERR 3
#define GSASL_ERR  4
#define NO_ERR     0

#ifndef	lint
static char     sccsid[] = "$Id: gsasl_mkpasswd.c,v 1.2 2022-08-05 23:39:15+05:30 Cprogrammer Exp mbhangui $";
#endif

#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
int
gsasl_mkpasswd(int verbose, char *mechanism, int iteration_count, char *b64salt_arg, char *password, stralloc *result)
{
	char            salt_buf[DEFAULT_SALT_SIZE];
	char           *salt, *hexsaltedpassword, *b64salt;
	size_t          saltlen, b64saltlen, hashlen = 0;
	int             i, hash = 0, res;
	char            saltedpassword[GSASL_HASH_MAX_SIZE];
	char            clientkey[GSASL_HASH_MAX_SIZE], serverkey[GSASL_HASH_MAX_SIZE],
					storedkey[GSASL_HASH_MAX_SIZE], strnum[FMT_ULONG];
	static stralloc in = {0}, b64 = {0}, b64storedkey = {0}, b64serverkey = {0};

	if (str_diff(mechanism, "SCRAM-SHA-1") == 0) {
		hash = GSASL_HASH_SHA1;
		hashlen = GSASL_HASH_SHA1_SIZE;
	} else
	if (str_diff(mechanism, "SCRAM-SHA-256") == 0) {
		hash = GSASL_HASH_SHA256;
		hashlen = GSASL_HASH_SHA256_SIZE;
	} else
		return USAGE_ERR;
	if (iteration_count <= 0)
		return USAGE_ERR;
	if (b64salt_arg) {
		i = str_chr(b64salt_arg, ',');
		if (b64salt_arg[i])
			return USAGE_ERR;
		b64salt = b64salt_arg;
		if (b64decode((const unsigned char *) b64salt, b64saltlen = str_len(b64salt), &b64) == -1)
			return MEMORY_ERR;
		salt = b64.s;
		saltlen = b64.len;
	} else {
		salt = salt_buf;
		saltlen = sizeof (salt_buf);
		sodium_random(salt, saltlen);
		if (!stralloc_copyb(&in, salt, saltlen) ||
				b64encode(&in, &b64) == -1)
			return MEMORY_ERR;
		b64salt = b64.s;
		b64saltlen = b64.len;
	}
	if ((res = gsasl_scram_secrets_from_password(hash, password, iteration_count,
			salt, saltlen, saltedpassword, clientkey, serverkey, storedkey)) != GSASL_OK)
		return GSASL_ERR;
	if (!stralloc_copyb(&in, storedkey, hashlen) ||
			b64encode(&in, &b64storedkey) == -1 ||
			!stralloc_copyb(&in, serverkey, hashlen) ||
			b64encode(&in, &b64serverkey) == -1)
		return MEMORY_ERR;
	strnum[i = fmt_int(strnum, iteration_count)] = 0;
	/*-
	 * generate password of the form
	 * {SCRAM-SHA-256}iter_count,salt,stored_key,server_key[,salted_password]
	 */
	if (!stralloc_copyb(result, "{", 1) ||
			!stralloc_cats(result, mechanism) ||
			!stralloc_append(result, "}") ||
			!stralloc_catb(result, strnum, i) ||
			!stralloc_append(result, ",") ||
			!stralloc_catb(result, b64salt, b64saltlen) ||
			!stralloc_append(result, ",") ||
			!stralloc_catb(result, b64storedkey.s, b64storedkey.len) ||
			!stralloc_append(result, ",") ||
			!stralloc_catb(result, b64serverkey.s, b64serverkey.len) ||
			!stralloc_0(result))
		return MEMORY_ERR;
	if (verbose) {
		if (!(hexsaltedpassword = (char *) alloc(hashlen * 2 + 1)))
			return MEMORY_ERR;
		i = fmt_hexdump(hexsaltedpassword, saltedpassword, hashlen);
		hexsaltedpassword[i] = 0;
		out("gsasl", result->s);
		out("gsasl", ",");
		out("gsasl", hexsaltedpassword);
		out("gsasl", "\n");
		flush("gsasl");
	}
	return NO_ERR;
}
#endif
#endif
