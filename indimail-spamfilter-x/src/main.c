/*****************************************************************************

NAME:
   main.c -- a wrapper for bogomain

AUTHOR:
   Eric S. Raymond <esr@thyrsus.com>

******************************************************************************/

#include "common.h"

#include <stdlib.h>

#include "bogomain.h"
#include "sighandler.h"

const char *progname = "bogofilter";

/* Function Definitions */

int main(int argc, char **argv) /*@globals errno,stderr,stdout@*/
{
    ex_t exitcode;

    init_globals();

    signal_setup();		/* setup to catch signals */
    atexit(bf_exit);

    exitcode = bogomain(argc, argv);

    exit(exitcode);
}

/* End */
