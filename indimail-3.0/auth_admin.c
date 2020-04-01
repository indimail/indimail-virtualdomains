/*
 * $Log: auth_admin.c,v $
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
#include <error.h>
#include <getEnvConfig.h>
#endif
#include "tcpopen.h"
#ifdef HAVE_SSL
#include "tls.h"
#endif

#ifndef lint
static char     sccsid[] = "$Id: auth_admin.c,v 1.1 2019-04-18 08:39:48+05:30 Cprogrammer Exp mbhangui $";
#endif

int
auth_admin(char *admin_user, char *admin_pass, char *admin_host, char *admin_port, char *clientcert)
{
	int             sfd, len, port, admin_timeout;
	char            inbuf[512];

	scan_uint(admin_port, (unsigned int *) &port);
	if ((sfd = tcpopen(admin_host, 0, port)) == -1) {
		strerr_warn5("tcpopen: ", admin_host, ":", admin_port, ": ", &strerr_sys);
		return (-1);
	}
	getEnvConfigInt(&admin_timeout, "ADMIN_TIMEOUT", 120);
#ifdef HAVE_SSL
	if (clientcert && tls_init(sfd, clientcert))
		return (-1);
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
			strerr_warn1("tcpopen: unable to write to stdout", &strerr_sys);
		errno = EPROTO;
		return (-1);
	}
	return (sfd);
}
