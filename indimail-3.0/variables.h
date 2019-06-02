/*
 * $Log: variables.h,v $
 * Revision 1.2  2019-04-15 21:58:49+05:30  Cprogrammer
 * added dir_control.h for vdir struct definition
 *
 * Revision 1.1  2019-04-13 23:39:28+05:30  Cprogrammer
 * variables.h
 *
 */
#ifndef VARIABLES_H
#define VARIABLES_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif
#include "indimail.h"
#include "dir_control.h"

extern int      create_flag;
extern int      site_size;
extern int      encrypt_flag;
extern int      fdm;
extern int      is_open;
extern int      delayed_insert;
extern stralloc mysql_host, cntrl_host;
extern char    *indi_port;
extern char    *cntrl_port;
extern int      isopen_cntrl;
extern int      isopen_vauthinit[];
extern int      OptimizeAddDomain;
extern int      userNotFound;
extern mdir_t   CurBytes, CurCount;
extern char    *default_table, *inactive_table, *cntrl_table;
extern char     dirlist[];
extern uid_t    indimailuid;
extern gid_t    indimailgid;
extern int      is_inactive;
extern int      is_overquota;
extern int      use_etrn;
extern int      use_vfilter;
extern int      verbose;
extern vdir_type vdir;
#endif
