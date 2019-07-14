/*
 * $Log: copyPwdStruct.h,v $
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * copyPwdStruct.h
 *
 */
#ifndef COPYPWDSTRUCT_H
#define COPYPWDSTRUCT_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

struct passwd  *copyPwdStruct(struct passwd *);

#endif
