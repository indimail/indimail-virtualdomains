/*
 * $Log: auth_admin.c,v $
 * Revision 1.7  2022-05-10 20:00:04+05:30  Cprogrammer
 * corrected misleading error message string
 *
 * Revision 1.6  2021-03-09 19:57:59+05:30  Cprogrammer
 * use functions from tls.c
 *
 * Revision 1.5  2021-03-04 11:54:48+05:30  Cprogrammer
 * added options to match host with common name
 * added option to specify cafile
 *
 * Revision 1.4  2021-03-03 14:12:38+05:30  Cprogrammer
 * added cafile argument to tls_init()
 *
 * Revision 1.3  2021-03-03 14:01:12+05:30  Cprogrammer
 * fixed data type for len
 *
 * Revision 1.2  2020-04-01 18:52:46+05:30  Cprogrammer
 * moved getEnvConfig to libqmail
 *
 * Revision 1.1  2019-04-18 08:39:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SSL
#include <sys/types.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <scan.h>
#include <str.h>
#include <env.h>
#include <error.h>
#include <getEnvConfig.h>
#endif
#include "tcpopen.h"
#ifdef HAVE_SSL
#include "tls.h"
#endif

#ifndef lint
static char     sccsid[] = "$Id: auth_admin.c,v 1.7 2022-05-10 20:00:04+05:30 Cprogrammer Exp mbhangui $";
#endif

int
auth_admin(char *admin_user, char *admin_pass, char *admin_host,
	char *admin_port, char *clientcert, char *cafile, int match_cn)
{
	int             sfd, port, admin_timeout;
	ssize_t         len;
	SSL            *ssl;
	SSL_CTX        *ctx;
	char           *ciphers;
	char            inbuf[512];

	scan_uint(admin_port, (unsigned int *) &port);
	if ((sfd = tcpopen(admin_host, 0, port)) == -1) {
		strerr_warn5("tcpopen: ", admin_host, ":", admin_port, ": ", &strerr_sys);
		return (-1);
	}
	getEnvConfigInt(&admin_timeout, "ADMIN_TIMEOUT", 120);
#ifdef HAVE_SSL
	if (clientcert) {
		if (!(ciphers = env_get("TLS_CIPHER_LIST")))
			ciphers = "PROFILE=SYSTEM";
		if (!(ctx = tls_init(clientcert, cafile, ciphers, client)))
			return(-1);
		if (!(ssl = tls_session(ctx, sfd, ciphers)))
			return(-1);
		SSL_CTX_free(ctx);
		ctx = NULL;
		if (tls_connect(ssl, match_cn ? admin_host : 0) == -1)
			return(-1);
	}
#endif
	if ((len = saferead(sfd, inbuf, sizeof(inbuf) - 1, admin_timeout)) == -1 || !len) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		return (-1);
	}
	inbuf[len] = 0;
	if (!str_str(inbuf, "Login: ")) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	}
	len = str_len(admin_user);
	if (safewrite(sfd, admin_user, len, admin_timeout) != len || safewrite(sfd, "\n", 1, admin_timeout) != 1) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	} else
	if ((len = saferead(sfd, inbuf, sizeof(inbuf) - 1, admin_timeout)) == -1 || !len) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	}
	inbuf[len] = 0;
	if (!str_str(inbuf, "Password: ")) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	}
	len = str_len(admin_pass);
	if (safewrite(sfd, admin_pass, len, admin_timeout) != len || safewrite(sfd, "\n", 1, admin_timeout) != 1) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	} else
	if ((len = saferead(sfd, inbuf, sizeof(inbuf) - 1, admin_timeout)) == -1 || !len) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		errno = EPROTO;
		return (-1);
	} else
	if (!str_str(inbuf, "OK")) {
		close(sfd);
#ifdef HAVE_SSL
		ssl_free();
#endif
		if (write(1, inbuf, len) == -1)
			strerr_warn1("unable to write to stdout", &strerr_sys);
		errno = EPROTO;
		return (-1);
	}
	return (sfd);
}
