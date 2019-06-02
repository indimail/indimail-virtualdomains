/*
 * $Log: indimail_compat.h,v $
 * Revision 1.1  2019-05-29 01:06:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef INDIMAIL_COMPATH_H
#define INDIMAIL_COMPATH_H
#include <indimail_config.h>
#include <indimail.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef ENABLE_DOMAIN_LIMITS
#define ENABLE_DOMAIN_LIMITS
#endif
#ifndef VALIAS
#define VALIAS
#endif
#ifndef QUERY_CACHE
#define QUERY_CACHE
#endif
#ifndef USE_MAILDIRQUOTA
#define USE_MAILDIRQUOTA
#endif

#include <iopen.h>
#include <iclose.h>
#include <lowerit.h>
#include <sql_getpw.h>
#include <sql_setpw.h>
#include <vlimits.h>
#include <r_mkdir.h>
#include <ipasswd.h>
#include <sql_getall.h>
#include <setuserquota.h>
#include <check_quota.h>
#include <iadduser.h>
#include <vdelfiles.h>
#include <valias_select.h>
#include <valias_insert.h>
#include <valias_delete.h>
#include <valias_update.h>

#ifndef INDIMAIL_UMASK
#define INDIMAIL_UMASK          0077
#endif
#ifndef INDIMAILDIR
#define INDIMAILDIR "/var/indimail"
#endif
#ifndef QMAILDIR
#define QMAILDIR "/var/indimail"
#endif

int             vauth_open(char *);
struct passwd  *vauth_getpw(char *, char *);
char           *vget_real_domain(char *);
char           *vshow_atrn_map(char **, char **);
void            vclose();
char           *vget_assign(char *, char *, int, uid_t *, gid_t *);
int             vpasswd(char *, char *, char *, int);
char           *get_assign(char *, stralloc *, uid_t *, gid_t *);
#ifdef QUERY_CACHE
void            get_assign_cache(char);
#endif
long            count_table(char *, char *);

#endif
