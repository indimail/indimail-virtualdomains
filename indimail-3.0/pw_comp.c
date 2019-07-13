/*
 * $Log: pw_comp.c,v $
 * Revision 1.1  2019-04-14 13:23:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <math.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <env.h>
#include <error.h>
#include <str.h>
#endif
#include "common.h"
#include "in_crypt.h"
#include "hmac.h"
#include "digest_md5.h"
#include "mkpasswd3.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: pw_comp.c,v 1.1 2019-04-14 13:23:55+05:30 Cprogrammer Exp mbhangui $";
#endif

static char     hextab[] = "0123456789abcdef";

int
pw_comp(unsigned char *testlogin, unsigned char *localpw, unsigned char *challenge,
	unsigned char *response, int auth_method)
{
	unsigned char   digest[21], digascii[41];
	static stralloc Crypted = {0};
	char           *crypt_pass, *e;
	int             i, len;

	if (!env_get("DISABLE_CRYPT") && (!challenge || (challenge && !*challenge))) {
		if (!(crypt_pass = in_crypt((char *) response, (char *) localpw))) {
			out("pw_comp", "454-CRYPT: ");
			out("pw_comp", error_str(errno));
			out("pw_comp", " (#4.3.0)\r\n");
			flush("pw_comp");
			_exit (111);
		}
		len = str_len(crypt_pass);
		i = str_diffn(crypt_pass, (char *) localpw, (size_t) len + 1);
		/*- non CRAM-MD5 aware app */
		if (i && env_get("TRIVIAL_PASSWORDS")) {
			mkpasswd3((char *) localpw, &Crypted);
			if (!(crypt_pass = in_crypt((char *) response, Crypted.s))) {
				out("pw_comp", "454-CRYPT: ");
				out("pw_comp", error_str(errno));
				out("pw_comp", " (#4.3.0)\r\n");
				flush("pw_comp");
				_exit (111);
			}
			len = str_len(crypt_pass);
			i = str_diffn(crypt_pass, Crypted.s, (size_t) len + 1);
		}
		return (i);
	}
	if ((!auth_method && !env_get("DISABLE_CRAM_MD5")) || auth_method == AUTH_CRAM_MD5) {
		hmac_md5(challenge, (int) str_len((char *) challenge), localpw,
			(int) str_len((char *) localpw), digest);
		digascii[32] = 0;
		for (i=0, e = (char *) digascii; i<16; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e=0;
		if (!(i = str_diff((char *) digascii, (char *) response)))
			return (i);
	}
	if ((!auth_method && !env_get("DISABLE_CRAM_SHA1")) || auth_method == AUTH_CRAM_SHA1) {
		hmac_sha1(challenge, (int) str_len((char *) challenge), localpw,
			(int) str_len((char *) localpw), digest);
		digascii[40] = 0;
		for (i=0, e = (char *) digascii; i<20; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e = 0;
		if (!(i = str_diff((char *) digascii, (char *) response)))
			return (i);
	}
	if ((!auth_method && !env_get("DISABLE_CRAM_SHA256")) || auth_method == AUTH_CRAM_SHA256) {
		hmac_sha256(challenge, (int) str_len((char *) challenge), localpw,
			(int) str_len((char *) localpw), digest);
		digascii[40] = 0;
		for (i=0, e = (char *) digascii; i<20; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e = 0;
		if (!(i = str_diff((char *) digascii, (char *) response)))
			return (i);
	}
	if ((!auth_method && !env_get("DISABLE_CRAM_RIPEMD")) || auth_method == AUTH_CRAM_RIPEMD) {
		hmac_ripemd(challenge, (int) str_len((char *) challenge), localpw,
			(int) str_len((char *) localpw), digest);
		digascii[40] = 0;
		for (i=0, e = (char *) digascii; i<20; i++) {
			*e = hextab[digest[i]/16]; ++e;
			*e = hextab[digest[i]%16]; ++e;
		} *e = 0;
		if (!(i = str_diff((char *) digascii, (char *) response)))
			return (i);
	}
	if ((!auth_method && !env_get("DISABLE_DIGEST_MD5")) || auth_method == AUTH_DIGEST_MD5) {
		if ((i = digest_md5((char *) response, testlogin, challenge, localpw)) == -1)
		{
			out("pw_comp", "454-DIGEST-MD5: ");
			out("pw_comp", error_str(errno));
			out("pw_comp", " (#4.3.0)\r\n");
			flush("pw_comp");
			_exit (111);
		} else
		if (!i)
			return (i);
	}
	return (1);
}
