/** \file netrc.h -- declarations and documentation for netrc.c
 * For license terms, see the file COPYING in this directory.
 */

#ifndef _NETRC_H_
#define _NETRC_H_ 1

# undef __BEGIN_DECLS
# undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

/** netrc structure used to carry account information from the .netrc. */
typedef struct _netrc_entry {
  /** The exact host name given in the .netrc, NULL if default. */
  char *host;

  /** The login name of the user. */
  char *login;

  /** Password for the account (NULL, if none). */
  char *password;

  /** Pointer to the next entry in the list. */
  struct _netrc_entry *next;
} netrc_entry;

__BEGIN_DECLS
/** Parse FILE as a .netrc file (as described in ftp(1)), and return a
   singly-linked list of entries.
   Diagnostic messages are through report(), except for fopen() errors with ENOENT.
   \return NULL is returned if the file could not be
   parsed.  */
netrc_entry *parse_netrc (char *file);

/** Search the netrc database for an account password.
 * The DEFAULT entry is never matched, it is designed for anonftp auto-logins.
 * \return the \a netrc_entry from \a list corresponding to \a host and user \a account.
 * \return NULL is returned if no such entry exists.
 */
const netrc_entry *search_netrc (netrc_entry *list, char *host, char *account);

/** Free memory for the entire \a list netrc_entry database */
void free_netrc (netrc_entry *list);
__END_DECLS

#endif /* _NETRC_H_ */
