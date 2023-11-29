/** \file report.c report function for noninteractive utilities
 *
 * For license terms, see the file COPYING in this directory.
 *
 * This code is distantly descended from the error.c module written by
 * David MacKenzie <djm@gnu.ai.mit.edu>.  It was redesigned and
 * rewritten by Dave Bodenstab, then redesigned again by ESR, then
 * bludgeoned into submission for SunOS 4.1.3 by Chris Cheyney
 * <cheyney@netcom.com>.  It works even when the return from
 * vprintf(3) is unreliable.
 */

/* make glibc expose vsyslog(3): */
#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include "config.h"
#include "fetchmail.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include "i18n.h"

#include <stdarg.h>

#define MALLOC(n)	xmalloc(n)	
#define REALLOC(n,s)	xrealloc(n,s)	

/* Used by report_build() and report_complete() to accumulate partial messages.
 */
static unsigned int partial_message_size = 0;
static unsigned int partial_message_size_used = 0;
static char *partial_message;
static int partial_suppress_tag = 0;
/* default size for the allocation of the report buffer */
const size_t defaultsize = 4096;

static unsigned unbuffered;
static unsigned int use_syslog;

#ifdef _LIBC
/* In the GNU C library, there is a predefined variable for this.  */

# define program_name program_invocation_name
# include <errno.h>

#endif	/* _LIBC */

/* Print the program name and error message MESSAGE, which is a printf-style
   format string with optional args. */
void report (FILE *errfp, const char *message, ...)
{
    va_list args;

    /* If a partially built message exists, print it now so it's not lost.  */
    if (partial_message_size_used != 0)
    {
	partial_message_size_used = 0;
	report (errfp, GT_("%s (log message incomplete)\n"), partial_message);
    }

    if (use_syslog)
    {
	int priority;

	va_start(args, message);
	priority = (errfp == stderr) ? LOG_ERR : LOG_INFO;

#ifdef HAVE_VSYSLOG
	vsyslog (priority, message, args);
#else
	{
	    va_list args_copy;
	    char *tmpbuf;
	    va_copy(args_copy, args);

	    int bufsiz = vsnprintf(NULL, 0, message, args);
	    if (bufsiz > 0) {
		    ++bufsiz;
		    tmpbuf = (char *)MALLOC(bufsiz);
		    vsnprintf(tmpbuf, bufsiz, message, args_copy);
		    syslog(priority, "%s", tmpbuf);
		    free(tmpbuf);
	    }
	    va_end(args_copy);
	}
#endif

	va_end(args);
    }
    else /* i. e. not using syslog */
    {
	if ( *message == '\n' )
	{
	    fputc( '\n', errfp );
	    ++message;
	}
	if (!partial_suppress_tag) {
		time_t timer;
		char buffer[64];
		struct tm* tm_info;
		timer = time(NULL);
		tm_info = localtime(&timer);
		strftime (buffer, sizeof(buffer), "%b %d %H:%M:%S", tm_info);
		fprintf (errfp, "%s %s: ", buffer, program_name);
	}
	partial_suppress_tag = 0;

	va_start (args, message);
	vfprintf (errfp, message, args);
	va_end (args);
	fflush (errfp);
    }
}

/**
 * Configure the report module. The output is set according to
 * \a mode.
 */
void report_init(int mode /** 0: regular output, 1: unbuffered output, -1: syslog */)
{
    switch(mode)
    {
    case 0:			/* errfp, buffered */
    default:
	unbuffered = FALSE;
	use_syslog = FALSE;
	break;

    case 1:			/* errfp, unbuffered */
	unbuffered = TRUE;
	use_syslog = FALSE;
	break;

    case -1:			/* syslogd */
	unbuffered = FALSE;
	use_syslog = TRUE;
	break;
    }
}

static void rep_ensuresize(size_t increment) {
    if (partial_message_size == 0)
    {
	/* initialization */
	partial_message_size_used = 0;
	/* avoid too many small allocations initially */
	if (increment < defaultsize) increment = defaultsize;
	partial_message_size = increment;
	partial_message = (char *)MALLOC (partial_message_size);
    }
    else /* already have buffer -> resize if too little room */
    {
	if (increment < defaultsize) increment = defaultsize;
	if (partial_message_size - partial_message_size_used < increment)
	{
	    partial_message_size += increment;
	    partial_message = (char *)REALLOC (partial_message, partial_message_size);
	}
    }
}

/* Build an report message by appending MESSAGE, which is a printf-style
   format string with optional args, to the existing report message (which may
   be empty.)  The completed report message is finally printed (and reset to
   empty) by calling report_complete().
   If an intervening call to report() occurs when a partially constructed
   message exists, then, in an attempt to keep the messages in their proper
   sequence, the partial message will be printed as-is (with a trailing 
   newline) before report() prints its message. */


/* VARARGS */
static int report_vgetsize(const char *message, va_list args)
{
    return vsnprintf(NULL, 0, message, args);
}

/* note that report_vbuild assumes that the buffer was already allocated. */
/* VARARGS */
static int report_vbuild(const char *message, va_list args)
{
    int n;

    n = vsnprintf (partial_message + partial_message_size_used,
		   partial_message_size - partial_message_size_used,
		   message, args);

    /* output error, f. i. EILSEQ */
    if (n < 0)
	    return -1;

    if (n > 0)
    {
	partial_message_size_used += n;
    }

    return n;
}

void report_build (FILE *errfp, const char *message, ...)
{
    int n;
    va_list args;

/* the logic is to first calculate the size,
 * then reallocate, then fill the buffer
 */

    va_start(args, message);
    n = report_vgetsize(message, args);
    va_end(args);

    rep_ensuresize(n + 1);

    va_start(args, message);
    (void)report_vbuild(message, args);
    va_end(args);

    if (unbuffered && partial_message_size_used != 0)
    {
	partial_message_size_used = 0;
	fputs(partial_message, errfp);
    }
}

void report_flush(FILE *errfp)
{
    if (partial_message_size_used != 0)
    {
	partial_message_size_used = 0;
	report(errfp, "%s", partial_message);
	partial_suppress_tag = 1;
    }
}

/* Complete a report message by appending MESSAGE, which is a printf-style
   format string with optional args, to the existing report message (which may
   be empty.)  The completed report message is then printed (and reset to
   empty.) */
void report_complete (FILE *errfp, const char *message, ...)
{
    int n;
    va_list args;

    va_start(args, message);
    n = report_vgetsize(message, args);
    va_end(args);

    rep_ensuresize(n + 1);

    va_start(args, message);
    (void)report_vbuild(message, args);
    va_end(args);

    /* Finally... print it.  */
    partial_message_size_used = 0;

    if (unbuffered)
    {
	fputs(partial_message, errfp);
	fflush (errfp);
    }
    else
	report(errfp, "%s", partial_message);
}

/* Sometimes we want to have at most one error per line.  This
   variable controls whether this mode is selected or not.  */
static int error_one_per_line;

/* If errnum is nonzero, print its corresponding system error message. */
void report_at_line (FILE *errfp, int errnum, const char *file_name,
	       unsigned int line_number, const char *message, ...)
{
    va_list args;

    if (error_one_per_line)
    {
	static const char *old_file_name;
	static unsigned int old_line_number;

	if (old_line_number == line_number &&
	    (file_name == old_file_name || (old_file_name != NULL && 0 == strcmp (old_file_name, file_name))))
	    /* Simply return and print nothing.  */
	    return;

	old_file_name = file_name;
	old_line_number = line_number;
    }

    fflush (errfp);
    if ( *message == '\n' )
    {
	fputc( '\n', errfp );
	++message;
    }
    fprintf (errfp, "%s:", program_name);

    if (file_name != NULL)
	fprintf (errfp, "%s:%u: ", file_name, line_number);

    va_start(args, message);
    vfprintf (errfp, message, args);
    va_end (args);

    if (errnum)
	fprintf (errfp, ": %s", strerror (errnum));
    putc ('\n', errfp);
    fflush (errfp);
}
