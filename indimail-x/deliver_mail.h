/*
 * $Log: deliver_mail.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * deliver_mail.h
 *
 */
#ifndef DELIVER_MAIL_H
#define DELIVER_MAIL_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

#ifdef USE_MAILDIRQUOTA
char           *read_quota(char *);
#endif
int             deliver_mail(char *, mdir_t, char *, uid_t, gid_t, char *, mdir_t *, mdir_t *);
mdir_t          get_message_size(void);

#endif
