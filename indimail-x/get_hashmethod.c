/*
 * $Id: get_hashmethod.c,v 1.1 2023-07-17 11:26:56+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <error.h>
#include <str.h>
#include <getEnvConfig.h>
#include <hashmethods.h>
#include <stralloc.h>
#include <strerr.h>
#include <getln.h>
#include <substdio.h>
#include <scan.h>
#include <open.h>
#endif
#include "get_real_domain.h"
#include "get_assign.h"
#include "get_hashmethod.h"

static stralloc fn = { 0 };

static char *
get_default_fn()
{
	char           *sysconfdir, *controldir;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&fn, controldir) ||
				!stralloc_catb(&fn, "/hash_method", 12)) {
			strerr_warn1("out of memory", 0);
			return ((char *) NULL);
		}
	} else {
		if (!stralloc_copys(&fn, sysconfdir) ||
				!stralloc_catb(&fn, "/", 1) ||
				!stralloc_cats(&fn, controldir) ||
				!stralloc_catb(&fn, "/hash_method", 12)) {
			strerr_warn1("out of memory", 0);
			return ((char *) NULL);
		}
	}
	if (!stralloc_0(&fn)) {
		strerr_warn1("out of memory", 0);
		return ((char *) NULL);
	}
	return fn.s;
}

int
get_hashmethod(const char *domain)
{
	struct substdio ssin;
	static stralloc line = {0};
	char            inbuf[4096];
	char           *ptr;
	const char     *real_domain;
	int             match, fd, i, r;
	static int      hash_m = -1;

	if (hash_m != -1)
		return hash_m;
	if (domain) {
		if (!(real_domain = get_real_domain(domain))) {
			strerr_warn2(domain, ": domain does not exist", 0);
			return -1;
		}
		if (!(ptr = get_assign(real_domain, 0, 0, 0))) {
			strerr_warn2(real_domain, ": domain does not exist", 0);
			return -1;
		}
		if (!stralloc_copys(&fn, ptr) ||
				!stralloc_catb(&fn, "/hash_method", 12) ||
				!stralloc_0(&fn)) {
			strerr_warn1("out of memory", 0);
			return -1;
		}
		if (access(fn.s, F_OK) && errno == error_noent) {
			ptr = get_default_fn();
			if (access(ptr, F_OK)) {
				if (errno == error_noent)
					return (hash_m = PASSWORD_HASH);
				else
					return -1;
			}
		}
	} else {
		ptr = get_default_fn();
		if (access(ptr, F_OK)) {
			if (errno == error_noent)
				return (hash_m = PASSWORD_HASH);
			else
				return -1;
		}
	}
	if ((fd = open_read(fn.s)) == -1) {
		strerr_warn3("get_hashmethod: ", fn.s, ": ", &strerr_sys);
		return -1;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("get_hashmethod: read: ", fn.s, ": ", &strerr_sys);
			close(fd);
			return -1;
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0; /*- remove newline */
		} else {
			if (!stralloc_0(&line)) {
				strerr_warn1("out of memory", 0);
				return -1;
			}
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		close(fd);
		if ((r = scan_int(ptr, &i)) != str_len(ptr))
			i = -1;
		if (!str_diffn(ptr, "DES", 4) || i == DES_HASH)
			return (hash_m = DES_HASH);
		else
		if (!str_diffn(ptr, "MD5", 4) || i == MD5_HASH)
			return (hash_m = MD5_HASH);
		else
		if (!str_diffn(ptr, "SHA-256", 8) || i == SHA256_HASH)
			return (hash_m = SHA256_HASH);
		else
		if (!str_diffn(ptr, "SHA-512", 8) || i == SHA512_HASH)
			return (hash_m = SHA512_HASH);
		else
		if (!str_diffn(ptr, "YESCRYPT", 9) || i == YESCRYPT_HASH)
			return (hash_m = YESCRYPT_HASH);
		else
			return -1;
	}
	close(fd);
	return -1;
}

char *
print_hashmethod(int method)
{
	switch (method)
	{
	case DES_HASH:
		return "DES";
	case MD5_HASH:
		return "MD5";
	case SHA256_HASH:
		return "SHA-256";
	case SHA512_HASH:
		return "SHA-512";
	case YESCRYPT_HASH:
		return "YESCRYPT";
	default:
		return print_hashmethod(PASSWORD_HASH);
	}
}

/*
 * $Log: get_hashmethod.c,v $
 * Revision 1.1  2023-07-17 11:26:56+05:30  Cprogrammer
 * Initial revision
 *
 */
