/*
 * $Log: parse_email.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * parse_email.h
 *
 */
#ifndef PARSE_EMAIL_H
#define PARSE_EMAIL_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

int             parse_email(const char *, stralloc *, stralloc *);

#endif
