/*
 * $Log: tls.h,v $
 * Revision 1.6  2021-03-10 19:46:32+05:30  Cprogrammer
 * added proto for set_essential_fd()
 *
 * Revision 1.5  2021-03-09 19:59:12+05:30  Cprogrammer
 * refactored tls code
 *
 * Revision 1.4  2021-03-04 11:56:25+05:30  Cprogrammer
 * added host argument to match host with common name
 *
 * Revision 1.3  2021-03-03 14:14:06+05:30  Cprogrammer
 * added cafile argument to tls_init()
 *
 * Revision 1.2  2021-03-03 14:00:54+05:30  Cprogrammer
 * fixed data types
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * tls.h
 *
 */
#ifndef TLS_H
#define TLS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#ifdef HAVE_SSL
#include <openssl/ssl.h>
#endif

enum tlsmode  {none = 0, client = 1, server = 2};
enum starttls {smtp, pop3, unknown};

ssize_t         saferead(int, char *, size_t, long);
ssize_t         safewrite(int, char *, size_t, long);
ssize_t         allwrite(int, char *, size_t);
#ifdef HAVE_SSL
SSL_CTX        *tls_init(char *, char *, char *, enum tlsmode);
SSL            *tls_session(SSL_CTX *, int, char *);
int             tls_connect(SSL *, char *);
int             tls_accept(SSL *);
void            ssl_free();
int             translate(int, int, int, unsigned int);
ssize_t         allwritessl(SSL *ssl, char *buf, size_t len);
ssize_t         ssl_timeoutio(int (*fun) (), long, int, int, SSL *, char *, size_t);
ssize_t         ssl_timeoutread(long, int, int, SSL *, char *, size_t);
ssize_t         ssl_timeoutwrite(long, int, int, SSL *, char *, size_t);
int             ssl_timeoutrehandshake(long, int, int, SSL *);
const char     *myssl_error_str();
void            set_essential_fd(int);
#endif

#endif
