/*
 * For license terms, see the file COPYING in this directory.
 */

/***********************************************************************
  module:       getpass.c
  project:      fetchmail
  programmer:   Carl Harris, ceharris@mal.com
  description: 	getpass() replacement which allows for long passwords.
                This version hacked by Wilfred Teiken, allowing the
                password to be piped to fetchmail.
 
 ***********************************************************************/

#include "config.h"
#include "fetchmail.h"

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "i18n.h"

#define INPUT_BUF_SIZE	PASSWORDLEN

#include <termios.h>

static int ttyfd;

static struct termios termb;
static tcflag_t flags;

static void save_tty_state(void);
static void disable_tty_echo(void);
static void restore_tty_state(void);
static void sigint_handler(int);

char *fm_getpassword(char *prompt)
{
    char *p;
    int c;
    FILE *fi;
    static char pbuf[INPUT_BUF_SIZE];
    SIGHANDLERTYPE sig = 0;	/* initialization pacifies -Wall */

    int istty = isatty(0);

    /* get the file descriptor for the actual input device if it's a tty */
    if (istty)
    {
	if ((fi = fdopen(open("/dev/tty", 2), "r")) == NULL)
	    fi = stdin;
	else
	    setbuf(fi, (char *)NULL);
    }
    else
	fi = stdin;

    /* store descriptor for the tty */
    ttyfd = fileno(fi);

    if (istty)
    {
	/* preserve tty state before turning off echo */
	save_tty_state();

	/* now that we have the current tty state, we can catch SIGINT and  
	   exit gracefully */
	sig = set_signal_handler(SIGINT, sigint_handler);

	/* turn off echo on the tty */
	disable_tty_echo();

	/* display the prompt and get the input string */
	fprintf(stderr, "%s", prompt);
    }

    for (p = pbuf; (c = getc(fi))!='\n' && c!=EOF;)
    {
	if (p < &pbuf[INPUT_BUF_SIZE - 1])
	    *p++ = c;
    }
    *p = '\0';

    /* write a newline so cursor won't appear to hang */
    if (fi != stdin)
	fprintf(stderr, "\n");

    if (istty)
    {
	/* restore previous state of the tty */
	restore_tty_state();

	/* restore previous state of SIGINT */
	set_signal_handler(SIGINT, sig);
    }
    if (fi != stdin)
	fclose(fi);	/* not checking should be safe, file mode was "r" */

    return(pbuf);
}

static void save_tty_state (void)
{
    tcgetattr(ttyfd, &termb);
    flags = termb.c_lflag;
}

static void disable_tty_echo(void) 
{
    /* turn off echo on the tty */
    termb.c_lflag &= ~ECHO;
    tcsetattr(ttyfd, TCSAFLUSH, &termb);
}

static void restore_tty_state(void)
{
    /* restore previous tty echo state */
    termb.c_lflag = flags;
    tcsetattr(ttyfd, TCSAFLUSH, &termb);
}

static void sigint_handler(int signum)
{
    (void)signum;
    restore_tty_state();
    report(stderr, GT_("\nCaught SIGINT... bailing out.\n"));
    exit(1);
}

/* getpass.c ends here */
