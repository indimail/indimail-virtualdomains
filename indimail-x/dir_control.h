/*
 * $Log: dir_control.h,v $
 * Revision 1.3  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2019-04-15 21:56:05+05:30  Cprogrammer
 * added vdir struct definition
 *
 * Revision 1.1  2019-04-13 23:39:26+05:30  Cprogrammer
 * dir_control.h
 *
 */
#ifndef DIR_CONTROL_H
#define DIR_CONTROL_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

/*- Definitions for Bigdir */
#define MAX_DIR_LEVELS          3
#define MAX_USERS_PER_LEVEL     100
#define MAX_DIR_NAME            300
#define MAX_DIR_LIST            62

typedef struct
{
	int             level_cur;
	int             level_max;
	int             level_start[MAX_DIR_LEVELS];
	int             level_end[MAX_DIR_LEVELS];
	int             level_mod[MAX_DIR_LEVELS];
	/*- current spot in dir list */
	int             level_index[MAX_DIR_LEVELS];
	unsigned long   cur_users;
	char            the_dir[MAX_DIR_NAME];
} vdir_type;
extern vdir_type vdir;
extern char    *rfc_ids[];


void            init_dir_control(vdir_type *);
char           *inc_dir(vdir_type *, int);
int             vcreate_dir_control(const char *, const char *);
int             vdel_dir_control(const char *);
int             inc_dir_control(vdir_type *, int);
int             dec_dir_control(const char *, const char *, const char *, uid_t, gid_t);
int             vread_dir_control(const char *, vdir_type *, const char *);
int             vwrite_dir_control(const char *, vdir_type *, const char *, uid_t, gid_t);
int             dec_dir_control(const char *, const char *, const char *, uid_t, gid_t);

#endif
