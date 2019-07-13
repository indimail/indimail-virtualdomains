/*
 * $Log: sslerator.c,v $
 * Revision 1.2  2019-06-07 16:07:19+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:37:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef lint
static char     sccsid[] = "$Id: sslerator.c,v 1.2 2019-06-07 16:07:19+05:30 mbhangui Exp mbhangui $";
#endif

#ifdef HAVE_SSL
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <strerr.h>
#include <fmt.h>
#include <env.h>
#include <strmsg.h>
#endif
#include "sockwrite.h"
#include "variables.h"

int             translate(SSL *, int, int, int, int, unsigned int);

static int      usessl = 0;
static char    *certfile;
SSL_CTX        *ctx = (SSL_CTX *) 0;

static int
ssl_write(SSL *ssl, char *buf, int len)
{
	int             w;

	while (len) {
		if ((w = SSL_write(ssl, buf, len)) == -1) {
			if (errno == EINTR)
				continue;
			return -1;	/*- note that some data may have been written */
		}
		if (w == 0);	/*- luser's fault */
		buf += w;
		len -= w;
	}
	return 0;
}

int
translate(SSL *ssl, int err_to_net, int out, int clearout, int clearerr, unsigned int iotimeout)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout;
	char            strnum[FMT_ULONG];
	int             flagexitasap, sslin, retval, n, r;
	char            tbuf[2048];

	timeout.tv_sec = iotimeout;
	timeout.tv_usec = 0;
	flagexitasap = 0;
	if (SSL_accept(ssl) <= 0) {
		strerr_warn1("sslerator: translate: unable to accept SSL connection", 0);
		while ((n = ERR_get_error()))
			strerr_warn2("sslerator: ", ERR_error_string(n, 0), 0);
		return (1);
	}
	if ((sslin = SSL_get_rfd(ssl)) == -1) {
		strerr_warn1("sslerator: translate: unable to set up SSL connection", 0);
		while ((n = ERR_get_error()))
			strerr_warn2("sslerator: ", ERR_error_string(n, 0), 0);
		return (1);
	}
	while (!flagexitasap) {
		FD_ZERO(&rfds);
		FD_SET(sslin, &rfds);
		FD_SET(clearout, &rfds);
		FD_SET(clearerr, &rfds);
		if ((retval = select(clearerr > sslin ? clearerr + 1 : sslin + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn1("sslerator: translate: ", &strerr_sys);
			return (1);
		} else
		if (!retval) {
			strnum[fmt_ulong(strnum, timeout.tv_sec)] = 0;
			strerr_warn3("sslerator: translate: timeout reached without input [", strnum, " sec]", 0);
			return (0);
		}
		if (FD_ISSET(sslin, &rfds)) {
			/*- data on sslin */
			if ((n = SSL_read(ssl, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn2("sslerator: translate: unable to read from network: ",
						ERR_error_string(SSL_get_error(ssl, n), 0), 0);
				flagexitasap = 1;
			} else
			if (n == 0)
				flagexitasap = 1;
			else
			if ((r = sockwrite(out, tbuf, n)) < 0) {
				strerr_warn1("sslerator: translate: unable to write to client: ", &strerr_sys);
				return (1);
			}
		}
		if (FD_ISSET(clearout, &rfds)) {
			/*- data on clearout */
			if ((n = read(clearout, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn1("sslerator: translate: unable to read from client: ", &strerr_sys);
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = ssl_write(ssl, tbuf, n)) < 0) {
				strerr_warn1("sslerator: translate: unable to write to network: ", &strerr_sys);
				return (1);
			}
		}
		if (FD_ISSET(clearerr, &rfds)) {
			/*- data on clearerr */
			if ((n = read(clearerr, tbuf, sizeof(tbuf))) < 0) {
				strerr_warn1("sslerator: translate: unable to read from client: ", &strerr_sys);
				return (1);
			} else
			if (n == 0)
				flagexitasap = 1;
			if ((r = (err_to_net ? ssl_write(ssl, tbuf, n) : sockwrite(2, tbuf, n))) < 0) {
				strerr_warn2("sslerator: translate: unable to write to ",
						err_to_net ? "network: " : "stderr: ", &strerr_sys);
				return (1);
			}
		}
	}
	return (0);
}

SSL_CTX *
load_certificate(char *certfile)
{
	SSL_CTX        *myctx = (SSL_CTX *) 0;
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
	char           *ptr;
#endif

    /* setup SSL context (load key and cert into ctx) */
	if (!(myctx = SSL_CTX_new(SSLv23_server_method()))) {
		strerr_warn2("SSL_CTX_new: unable to create SSL context: ",
			ERR_error_string(ERR_get_error(), 0), 0);
		return ((SSL_CTX *) 0);
	}
	/* set prefered ciphers */
#ifdef CRYPTO_POLICY_NON_COMPLIANCE
	ptr = env_get("SSL_CIPHER");
	if (ptr && !SSL_CTX_set_cipher_list(myctx, ptr)) {
		strer_warn3("SSL_CTX_set_cipher_list: unable to set cipher list: ", ptr,
			ERR_error_string(ERR_get_error(), 0), 0);
		SSL_CTX_free(myctx);
		return ((SSL_CTX *) 0);
	}
#endif
	if (SSL_CTX_use_certificate_chain_file(myctx, certfile)) {
		if (SSL_CTX_use_RSAPrivateKey_file(myctx, certfile, SSL_FILETYPE_PEM) != 1) {
			strerr_warn2("SSL_CTX_use_RSAPrivateKey: unable to load RSA private key: ",
				ERR_error_string(ERR_get_error(), 0), 0);
			SSL_CTX_free(myctx);
			return ((SSL_CTX *) 0);
		}
		if (SSL_CTX_use_certificate_file(myctx, certfile, SSL_FILETYPE_PEM) != 1) {
			strerr_warn2("SSL_CTX_use_certificate_file: unable to load certificate: ",
			ERR_error_string(ERR_get_error(), 0), 0);
			SSL_CTX_free(myctx);
			return ((SSL_CTX *) 0);
		}
	}
	return (myctx);
}

static void
SigChild(void)
{
	int             status;
	pid_t           pid;

	for (;(pid = waitpid(-1, &status, WNOHANG | WUNTRACED));) {
#ifdef ERESTART
		if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (pid == -1 && errno == EINTR)
#endif
			continue;
		break;
	} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
	(void) signal(SIGCHLD, (void (*)()) SigChild);
	return;
}

static int
get_options(int argc, char **argv, int *err_to_net, char ***pgargs)
{
	int             c;

	certfile = 0;
	*err_to_net = 0;
	while ((c = getopt(argc, argv, "vn:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'e':
			*err_to_net = 1;
			break;
		case 'n':
			certfile = optarg;
			break;
		default:
			strerr_warn1("usage: sslerator [-t][-n certfile][-v] prog [args]", 0);
			return (1);
		}
	}
	if (!certfile)
		certfile = env_get("TLS_CERTFILE");
	if (certfile && *certfile)
		usessl = 1;
	if (env_get("TCPREMOTEIP"))
		*err_to_net = 0;
	if (optind < argc)
		*pgargs = argv + optind;
	else {
		strerr_warn1("usage: sslerator [-t][-n certfile][-v] prog [args]", 0);
		return (1);
	}
	return (0);
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	BIO            *sbio;
	SSL            *ssl;
	char          **pgargs, *ptr;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             status, r, ret, n, err_to_net, pid, pi1[2], pi2[2], pi3[2];

	if (get_options(argc, argv, &err_to_net, &pgargs))
		return (1);
	if (usessl == 0) {
		execv(pgargs[0], pgargs);
		strerr_warn3("sslerator: execv: ", pgargs[0], ": ", &strerr_sys);
		return (1);
	}
	(void) signal(SIGCHLD, (void (*)()) SigChild);
	if (pipe(pi1) != 0 || pipe(pi2) != 0 || pipe(pi3) != 0) {
		strerr_warn1("sslerator: unable to create pipe: ", &strerr_sys);
		exit(1);
	}
	switch (pid = fork())
	{
		case -1:
			strerr_warn1("sslerator: fork: ", &strerr_sys);
			return (1);
		case 0:
			close(pi1[1]);
			close(pi2[0]);
			close(pi3[0]);
			if (dup2(pi1[0], 0) == -1 || dup2(pi2[1], 1) == -1 || dup2(pi3[1], 2) == -1) {
				strerr_warn1("sslerator: unable to set up descriptors: ", &strerr_sys);
				exit(1);
			}
			if (pi1[0] != 0)
				close(pi1[0]);
			if (pi2[1] != 1)
				close(pi2[1]);
			if (pi3[1] != 2)
				close(pi1[1]);
			/*
			 * signals are allready set in the parent
			 */
			putenv("SSLERATOR=1");
			execv(pgargs[0], pgargs);
			strerr_warn3("sslerator: execv: ", pgargs[0], ": ", &strerr_sys);
			close(0);
			close(1);
			close(2);
			_exit(1);
			break;
		default:
			break;
	} /*- switch (pid = fork()) */
	close(pi1[0]);
	close(pi2[1]);
	close(pi3[1]);
	if (access(certfile, F_OK)) {
		strerr_warn3("sslerator: missing certficate: ", certfile, ": ", &strerr_sys);
		return (1);
	}
	SSL_library_init();
	if (!(ctx = load_certificate(certfile)))
		return (1);
	if (!(ssl = SSL_new(ctx))) {
		long e;
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		while ((e = ERR_get_error()))
			strerr_warn3(strnum1, ": ", ERR_error_string(e, 0), 0);
		SSL_CTX_free(ctx);
		_exit(1);
	}
	SSL_CTX_free(ctx);
#ifndef CRYPTO_POLICY_NON_COMPLIANCE
	if (!(ptr = env_get("SSL_CIPHER")))
		ptr = "PROFILE=SYSTEM";
	if (!SSL_set_cipher_list(ssl, ptr)) {
		strerr_warn4("sslerator: unable to set ciphers: ", ptr,
				": ", ERR_error_string(ERR_get_error(), 0), 0);
		SSL_free(ssl);
		return (1);
	}
#endif
	if (!(sbio = BIO_new_socket(0, BIO_NOCLOSE))) {
		strnum1[fmt_ulong(strnum1, getpid())] = 0;
		strerr_warn2(strnum1, ": unable to set up BIO socket", 0);
		SSL_free(ssl);
		_exit(1);
	}
	SSL_set_bio(ssl, sbio, sbio); /*- cannot fail */
	if ((ptr = env_get("BANNER"))) {
		strmsg_out2(ptr, "\n");
	}
	n = translate(ssl, err_to_net, pi1[1], pi2[0], pi3[0], 3600);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	for (ret = -1;(r = waitpid(pid, &status, WNOHANG | WUNTRACED));) {
#ifdef ERESTART
		if (r == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (r == -1 && errno == EINTR)
#endif
			continue;
		if (WIFSTOPPED(status) || WIFSIGNALED(status)) {
			if (verbose) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_int(strnum2, WIFSTOPPED(status) ? WSTOPSIG(status) : WTERMSIG(status))] = 0;
				strerr_warn3(strnum1, ": killed by signal ", strnum2, 0);
			}
			ret = -1;
		} else
		if (WIFEXITED(status)) {
			ret = WEXITSTATUS(status);
			if (verbose) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_int(strnum2, ret)] = 0;
				strerr_warn3(strnum1, ": normal exit return status ", strnum2, 0);
			}
		}
		break;
	} /*- for (; pid = waitpid(-1, &status, WNOHANG | WUNTRACED);) -*/
	if (n)
		_exit(n);
	if (ret)
		_exit (ret);
	_exit (0);
}
#else
int
main(argc, argv)
	int             argc;
	char          **argv;
{
	strerr_warn1("SSL support not detected. HAVE_SSL not defined", 0);
	return (1);
}
#endif
