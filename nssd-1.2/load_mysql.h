/*
 * $Log: load_mysql.h,v $
 * Revision 1.2  2020-04-30 19:50:29+05:30  Cprogrammer
 * define prototypes as extern to fix compilation error
 *
 * Revision 1.1  2019-06-13 21:21:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef LOAD_MYSQL_H
#define LOAD_MYSQL_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <mysql.h>

typedef unsigned int i_uint;
typedef unsigned long i_ulong;
typedef const char i_char;
#ifndef HAVE_BOOL
typedef char bool;
#endif
#ifdef LIBMARIADB
typedef struct st_mysql_res res;
#else
typedef struct MYSQL_RES res;
#endif

#ifdef DLOPEN_LIBMYSQLCLIENT
extern MYSQL   *mysql_Init(MYSQL *);
extern MYSQL   *(*in_mysql_init) (MYSQL *);
extern MYSQL   *(*in_mysql_real_connect) (MYSQL *, const char *, const char *, const char *, const char *, unsigned int, const char *, unsigned long);
extern i_char  *(*in_mysql_error) (MYSQL *);
extern i_uint   (*in_mysql_errno) (MYSQL *);
extern int      (*in_mysql_next_result) (MYSQL *);
extern void     (*in_mysql_close) (MYSQL *);
extern int      (*in_mysql_options) (MYSQL *, enum mysql_option, const void *);
#if MYSQL_VERSION_ID >= 50703 && !defined(MARIADB_BASE_VERSION)
extern int      (*in_mysql_get_option) (MYSQL *, enum mysql_option, void *);
#endif
extern int      (*in_mysql_query) (MYSQL *, const char *);
extern res     *(*in_mysql_store_result) (MYSQL *);
extern char   **(*in_mysql_fetch_row) (MYSQL_RES *);
extern unsigned long  *(*in_mysql_fetch_lengths) (MYSQL_RES *);
extern my_ulonglong (*in_mysql_num_rows) (MYSQL_RES *);
extern unsigned int (*in_mysql_num_fields) (MYSQL_RES *res);
extern my_ulonglong (*in_mysql_affected_rows) (MYSQL *);
extern void     (*in_mysql_free_result) (MYSQL_RES *);
extern char    *(*in_mysql_stat) (MYSQL *);
extern int      (*in_mysql_ping) (MYSQL *);
extern i_ulong  (*in_mysql_real_escape_string) (MYSQL *, char *, const char *, unsigned long);
extern i_uint   (*in_mysql_get_proto_info) (MYSQL *);
extern int      (*in_mysql_select_db) (MYSQL *, const char *);
extern i_char  *(*in_mysql_get_host_info) (MYSQL *);
extern bool     (*in_mysql_ssl_set) (MYSQL *, const char *, const char *, const char *, const char *, const char *);
extern i_char  *(*in_mysql_get_ssl_cipher) (MYSQL *);
extern void     (*in_mysql_data_seek) (MYSQL_RES *, my_ulonglong);
extern i_char  *(*in_mysql_get_server_info) (MYSQL *);
extern i_char  *(*in_mysql_get_client_info) (void);
extern void    *loadLibrary(void **, char *, int *, char **);
extern void    *getlibObject(char *, void **, char *, char **);
extern void     closeLibrary(void **);
extern int      initMySQLlibrary(char **);
#else /*- DLOPEN_LIBMYSQLCLIENT */
extern MYSQL   *mysql_Init(MYSQL *);
extern MYSQL   *in_mysql_init(MYSQL *);
extern MYSQL   *in_mysql_real_connect(MYSQL *, const char *, const char *, const char *, const char *, unsigned int, const char *, unsigned long);
extern i_char  *in_mysql_error(MYSQL *);
extern i_uint   in_mysql_errno(MYSQL *mysql);
extern int      in_mysql_next_result(MYSQL *);
extern void     in_mysql_close(MYSQL *);
extern int      in_mysql_options(MYSQL *, enum mysql_option, const void *);
#if MYSQL_VERSION_ID >= 50703 && !defined(MARIADB_BASE_VERSION)
extern int      in_mysql_get_option(MYSQL *, enum mysql_option, void *);
#endif
extern int      in_mysql_query(MYSQL *, const char *);
extern res     *in_mysql_store_result(MYSQL *);
extern char   **in_mysql_fetch_row(MYSQL_RES *);
extern unsigned long  *in_mysql_fetch_lengths(MYSQL_RES *);
extern my_ulonglong in_mysql_num_rows(MYSQL_RES *);
extern unsigned int in_mysql_num_fields(MYSQL_RES *);
extern my_ulonglong in_mysql_affected_rows(MYSQL *);
extern void     in_mysql_free_result(MYSQL_RES *);
extern char    *in_mysql_stat(MYSQL *);
extern int      in_mysql_ping(MYSQL *);
extern i_ulong  in_mysql_real_escape_string(MYSQL *, char *, const char *, unsigned long);
extern i_uint   in_mysql_get_proto_info(MYSQL *);
extern int      in_mysql_select_db(MYSQL *, const char *);
extern i_char  *in_mysql_get_host_info(MYSQL *);
extern bool     in_mysql_ssl_set(MYSQL *, const char *, const char *, const char *, const char *, const char *);
extern i_char  *in_mysql_get_ssl_cipher(MYSQL *);
extern void     in_mysql_data_seek(MYSQL_RES *, my_ulonglong);
extern i_char  *in_mysql_get_server_info(MYSQL *);
extern i_char  *in_mysql_get_client_info(void);
#endif /*- #ifdef DLOPEN_LIBMYSQLCLIENT */
#endif /*- #ifndef LOAD_MYSQL_H */
