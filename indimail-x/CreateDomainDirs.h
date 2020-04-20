/*
 * $Log: CreateDomainDirs.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * CreateDomainDirs.h
 *
 */
#ifndef CREATEDOMAINDIRS_H
#define CREATEDOMAINDIRS_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int             CreateDomainDirs(char *, uid_t, gid_t);

#endif
