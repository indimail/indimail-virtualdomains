/*
 * $Id: log_sig.c,v 1.2 2025-01-22 15:58:22+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ALARM
#include <signal.h>
#endif

#include "defs.h"
#include "monitor.h"

extern unsigned int active;

void
sig_handle_abort(int sig)
{
	debug("\n[!] Caught signal %d, exiting ...\n", sig);
	exit(0);
}

void
sig_handle_timer(uint timeout)
{
	active = YES;
}

void
sig_init()
{
	signal(SIGALRM, (void (*)(int)) sig_handle_timer);
	signal(SIGINT, (void (*)(int)) sig_handle_abort);
	signal(SIGTERM, (void (*)(int)) sig_handle_abort);
}
/*
 * $Log: log_sig.c,v $
 * Revision 1.2  2025-01-22 15:58:22+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.1  2013-05-15 00:34:35+05:30  Cprogrammer
 * Initial revision
 *
 */
