/**
 * \file socket.h -- declarations for socket library functions
 *
 * For license terms, see the file COPYING in this directory.
 */

#ifndef SOCKET__
#define SOCKET__

struct addrinfo;

#include "config.h"
#include <sys/socket.h>
#include <netdb.h>

/** Create a new client socket; returns -1 on error */
int SockOpen(const char *host, const char *service, const char *plugin, struct addrinfo **);

/** 
Get a string terminated by an '\n' (matches interface of fgets).
Pass it a valid socket, a buffer for the string, and
the length of the buffer (including the trailing \0)
returns length of buffer on success, -1 on failure. 
*/
int SockRead(int sock, char *buf, int len);

/**
 * Peek at the next socket character without actually reading it.
 */
int SockPeek(int sock);

/**
Write a chunk of bytes to the socket (matches interface of fwrite).
Returns number of bytes successfully written.
*/
int SockWrite(int sock, const char *buf, int size);

/* from /usr/include/sys/cdefs.h */
#if !defined __GNUC__
# define __attribute__(xyz)    /* Ignore. */
#endif

/**
Send formatted output to the socket (matches interface of fprintf).
Returns number of bytes successfully written.
*/
int SockPrintf(int sock, const char *format, ...)
    __attribute__ ((format (printf, 2, 3)))
    ;
 
/**
Close a socket previously opened by SockOpen.  This allows for some
additional clean-up if necessary.
*/
int SockClose(int sock);

/**
 \todo document this
*/
int UnixOpen(const char *path);

#ifdef SSL_ENABLE
int SSLOpen(int sock, char *mycert, char *mykey, const char *myproto, int certck, char *cacertfile, char *cacertpath,
    char *fingerprint, char *servercname, char *label, char **remotename);
#endif /* SSL_ENABLE */

#endif /* SOCKET__ */
