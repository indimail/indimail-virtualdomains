/*
 * $Log: vlimits.h,v $
 * Revision 1.5  2024-05-28 19:35:50+05:30  Cprogrammer
 * added limit_type member to vlimits sructure
 *
 * Revision 1.4  2024-05-27 08:05:16+05:30  Cprogrammer
 * changed data type of variables to long
 *
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2019-04-15 22:00:32+05:30  Cprogrammer
 * added vlimit struct definition
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * vlimits.h
 *
 */
#ifndef VLIMITS_H
#define VLIMITS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

#ifdef ENABLE_DOMAIN_LIMITS
/* permissions for non-postmaster admins */
#define VLIMIT_DISABLE_CREATE 0x01
#define VLIMIT_DISABLE_MODIFY 0x02
#define VLIMIT_DISABLE_DELETE 0x04

#define VLIMIT_DISABLE_ALL (VLIMIT_DISABLE_CREATE|VLIMIT_DISABLE_MODIFY|VLIMIT_DISABLE_DELETE)
#define VLIMIT_DISABLE_BITS 3

struct vlimits {
      /* max service limits */
      long      domain_expiry;
      long      passwd_expiry;
      long      maxpopaccounts;
      long      maxaliases;
      long      maxforwards;
      long      maxautoresponders;
      long      maxmailinglists;

      /* quota & message count limits */
      umdir_t   diskquota;			/*- domain quota */
      umdir_t   maxmsgcount;		/*- domain quota total message count */
      umdir_t   defaultquota;		/*- user quota total message size */
      umdir_t   defaultmaxmsgcount;	/* -user quota total message count */

      /* the following are 0 (false) or 1 (true) */
      short     disable_pop;
      short     disable_imap;
      short     disable_dialup;
      short     disable_passwordchanging;
      short     disable_webmail;
      short     disable_relay;
      short     disable_smtp;

      /* the following permissions are for non-postmaster admins */
      short     perm_account;
      short     perm_alias;
      short     perm_forward;
      short     perm_autoresponder;
      short     perm_maillist;
      short     perm_maillist_users;
      short     perm_maillist_moderators;
      short     perm_quota;
      short     perm_defaultquota;
	  short     limit_type;
};

int             vget_limits(const char *, struct vlimits *);
int             vdel_limits(const char *);
int             vset_limits(const char *, struct vlimits *);
int             vlimits_get_flag_mask(struct vlimits *);

#endif
#endif
