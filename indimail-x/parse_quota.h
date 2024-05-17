/*
 * $Log: parse_quota.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * parse_quota.h
 *
 */
#ifndef PARSE_QUOTA_H
#define PARSE_QUOTA_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

mdir_t          parse_quota(const char *, mdir_t *);

#endif
