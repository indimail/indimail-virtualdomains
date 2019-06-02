/*
 * $Id: alias.h,v 1.1 2010-04-26 12:08:19+05:30 Cprogrammer Exp mbhangui $
 */
#ifndef ALIAS_H_H
#define ALIAS_H_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

void            adddotqmail();
void            adddotqmailnow();
int             adddotqmail_shared(char *, stralloc *, int);
void            deldotqmail();
void            deldotqmailnow();
char           *dotqmail_alias_command(char *);
void            moddotqmail();
void            moddotqmailnow();
int             show_aliases(void);
void            show_dotqmail_lines(char *, char *, time_t);
void            show_dotqmail_file(char *);

#endif
