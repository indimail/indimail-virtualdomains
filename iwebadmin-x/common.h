#ifndef COMMON_H
#define COMMON_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <substdio.h>

#define iweb_exit(ret) my_exit((ssdbg), (ret), __LINE__, __FILE__)
#define  INPUT_FAILURE          1 /*- invalid input */
#define  SETUP_FAILURE          2 /*- missing files/dirs */
#define  AUTH_FAILURE           3 /*- authentication error */
#define  SESSION_FAILURE        4 /*- error writing session */
#define  PASSWD_FAILURE         5 /*- error changing password */
#define  MEMORY_FAILURE         6 /*- oom */
#define  WRITE_FAILURE          7 /*- write error */
#define  READ_FAILURE           8 /*- read error */
#define  PERM_FAILURE           9 /*- permission */
#define  ASSIGN_FAILURE        10 /*- error reading assign */
#define  OUTPUT_FAILURE        11 /*- error writing output */
#define  MALFORMED_FAILURE     12 /*- Malformed input */
#define  LIMIT_FAILURE         13 /*- limit exceeded */
#define  ALIAS_FAILURE         14 /*- setting alias/forward failure */
#define  EXPIRE_FAILURE        15 /*- session expired */
#define  SYSTEM_FAILURE       111 /*- SYSTEM_FAILURE */


void            out(char *);
void            flush(void);
void            errout(char *);
void            errflush(void);
void            copy_status_mesg(char *);
void            set_status_mesg_size(int);
void            die_nomem();
void            my_exit(substdio *, int, int, char *);

extern substdio *ssdbg;

#endif /*- COMMON_H */
