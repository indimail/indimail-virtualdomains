/*****************************************************************************

NAME:
   bogoconfig.h -- prototypes and definitions for bogoconfig.c.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef BOGOCONFIG_H
#define BOGOCONFIG_H

#include "configfile.h"

/* Global variables */

extern const char *logtag;
extern const char *user_config_file;

extern rc_t query_config(void);
extern void process_parameters(int argc, char **argv, bool warn_on_error);

extern int process_arg(int option, const char *name, const char *arg, priority_t precedence, arg_pass_t pass);

#endif
