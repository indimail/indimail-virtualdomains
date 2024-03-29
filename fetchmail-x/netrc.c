/*
 * netrc.c -- parse the .netrc file to get hosts, accounts, and passwords
 *
   Gordon Matzigkeit <gord@gnu.ai.mit.edu>, 1996
   Copyright assigned to Eric S. Raymond, October 2001.

   For license terms, see the file COPYING in this directory.

   Compile with -DSTANDALONE to test this module.
   (Makefile.am should have a rule so you can just type "make netrc")
*/

#include "config.h"
#include "fetchmail.h"

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "netrc.h"
#include "i18n.h"

#ifdef STANDALONE
/* Normally defined in xstrdup.c. */
# define xstrdup strdup

/* Normally defined in xmalloc.c */
# define xmalloc malloc
# define xrealloc realloc

const char *program_name = "netrc";
#endif

/** Maybe add NEWENTRY to the account information list, LIST.
 * NEWENTRY is set to a ready-to-use netrc_entry, in any event. */
static void
maybe_add_to_list (netrc_entry **newentry, netrc_entry **list)
{
    netrc_entry *a, *l;
    a = *newentry;
    l = *list;

    /* We need a login name in order to add the entry to the list. */
    if (a && ! a->login)
    {
	/* Free any allocated space. */
	if (a->host)
	    free (a->host);
	if (a->password)
	    free (a->password);
    }
    else
    {
	if (a)
	{
	    /* Add the current machine into our list. */
	    a->next = l;
	    l = a;
	}

	/* Allocate a new netrc_entry structure. */
	a = (netrc_entry *) xmalloc (sizeof (netrc_entry));
    }

    /* Zero the structure, so that it is ready to use. */
    memset (a, 0, sizeof(*a));

    /* Return the new pointers. */
    *newentry = a;
    *list = l;
    return;
}

netrc_entry *
parse_netrc (char *file)
{
    FILE *fp;
    char buf[POPBUFSIZE+1], *p, *tok;
    const char *premature_token;
    netrc_entry *current, *retval;
    int ln;
    int error_flag = 0;
    int have_default = 0;
    int filedes = -1;
    struct stat st;

    /* The latest token we've seen in the file. */
    enum
    {
	tok_nothing, tok_account, tok_login, tok_macdef, tok_machine, tok_password
    } last_token = tok_nothing;

    current = retval = NULL;
    memset(&st, 0, sizeof(st));

    fp = fopen(file, "r");
    if (!fp)
    {
	/* Just return NULL if we can't open the file. */
	if (ENOENT != errno) {
	    report(stderr, GT_("%s: cannot open file for reading: %s\n"), file, strerror(errno));
	}
	return NULL;
    }

    /* stat the file, but only check mode if it contains passwords */
    filedes = fileno(fp);
    if (-1 == filedes) {
	    report(stderr, GT_("%s: rejecting file, cannot get file descriptor number for fstat: %s\n"),
		   file, strerror(errno));
	    error_flag = 1;
    } else {
	    if (-1 == fstat(filedes, &st)) {
		    report(stderr, GT_("%s: rejecting file, cannot fstat(%d): %s\n"),
			   file, filedes, strerror(errno));
		    error_flag = 1;
	    }
    }

    /* Initialize the file data. */
    ln = 0;
    premature_token = NULL;

    /* While there are lines in the file... */
    while (fgets(buf, sizeof(buf) - 1, fp))
    {
	ln++;

	/* Strip trailing CRLF */
	for (p = buf + strlen(buf) - 1; (p >= buf) && isspace((unsigned char)*p); p--)
	    *p = '\0';

	/* Parse the line. */
	p = buf;

	/* If the line is empty... */
	if (!*p)
	{
	    if (last_token == tok_macdef)	/* end of macro */
		last_token = tok_nothing;
	    else
		continue;			/* otherwise ignore it */
	}

	/* If we are defining macros, then skip parsing the line. */
	while (*p && last_token != tok_macdef)
	{
	    char quote_char = 0;
	    char *pp;

	    /* Skip any whitespace. */
	    while (*p && isspace ((unsigned char)*p))
		p++;

	    /* Discard end-of-line comments. */
	    if (*p == '#')
		break;

	    tok = pp = p;

	    /* Find the end of the token. */
	    while (*p && (quote_char || !isspace ((unsigned char)*p)))
	    {
		if (quote_char)
		{
		    if (quote_char == *p)
		    {
			quote_char = 0;
			p ++;
		    }
		    else
		    {
			if ('\\' == *p) ++p;
			*pp++ = *p;
			if (*p) ++p;
		    }
		}
		else
		{
		    if (*p == '"' || *p == '\'')
			quote_char = *p;
		    else
		    {
			*pp = *p;
			pp ++;
		    }
		    p ++;
		}
	    }
	    /* Null-terminate the token, if it isn't already. */
	    if (*p)
		*p ++ = '\0';
	    *pp = 0;

	    switch (last_token)
	    {
	    case tok_login:
		if (current)
		    current->login = (char *) xstrdup (tok);
		else
		    premature_token = "login";
		break;

	    case tok_machine:
		/* Start a new machine entry. */
		maybe_add_to_list (&current, &retval);
		current->host = (char *) xstrdup (tok);
		break;

	    case tok_password:
		if (st.st_size && st.st_mode & 077) { /* numeric values sanctioned by SUSv4 in retrospect */
		    report(stderr, GT_("%s: rejecting file because it is group or other accessible (mode %#o) and contains passwords.\n"),
			   file, st.st_mode & 0777);
		    error_flag = 1;
		    st.st_size = 0; /* zero out file size so we only warn once */
		}
		if (current)
		    current->password = (char *) xstrdup (tok);
		else
		    premature_token = "password";
		break;

		/* We handle most of tok_macdef above. */
	    case tok_macdef:
		if (!current)
		    premature_token = "macdef";
		break;

		/* We don't handle the account keyword at all. */
	    case tok_account:
		if (!current)
		    premature_token = "account";
		break;

		/* We handle tok_nothing below this switch. */
	    case tok_nothing:
		break;
	    }

	    if (premature_token)
	    {
		report(stderr,
			 GT_("%s:%d: warning: found \"%s\" before any host names\n"),
			 file, ln, premature_token);
		premature_token = NULL;
	    }

	    if (last_token != tok_nothing)
		/* We got a value, so reset the token state. */
		last_token = tok_nothing;
	    else
	    {
		/* Fetch the next token. */
		if (!strcmp (tok, "default"))
		{
		    maybe_add_to_list (&current, &retval);
		    have_default = 1;
		}
		else if (!strcmp (tok, "login"))
		    last_token = tok_login;

		else if (!strcmp (tok, "user"))
		    last_token = tok_login;

		else if (!strcmp (tok, "macdef"))
		    last_token = tok_macdef;

		else if (!strcmp (tok, "machine")) {
		    last_token = tok_machine;
		    if (have_default) {
			    error_flag = 1;
			    report(stderr, GT_("%s:%d: machine entry not permitted after default, rejecting file.\n"),
			    file, ln);
		    }
		}
		else if (!strcmp (tok, "password"))
		    last_token = tok_password;

		else if (!strcmp (tok, "passwd"))
		    last_token = tok_password;

		else if (!strcmp (tok, "account"))
		    last_token = tok_account;

		else
		{
		    report(stderr, GT_("%s:%d: warning: unknown token \"%s\"\n"),
			     file, ln, tok);
		}
	    }
	}
    }

    if (ferror(fp)) {
	report(stderr, GT_("%s: error reading file (%s).\n"), file, strerror(errno));
	error_flag = 1;
	clearerr(fp);
    }

    if (fclose(fp)) {
	report(stderr, GT_("%s: error reading file (%s).\n"), file, strerror(errno));
	error_flag = 1;
    }

    /* Finalize the last machine entry we found. */
    maybe_add_to_list (&current, &retval);
    free (current);

    if (error_flag) {
	free_netrc(retval);
	return NULL;
    }

    /* Reverse the order of the list so that it appears in file order. */
    current = retval;
    retval = NULL;
    while (current)
    {
	netrc_entry *saved_reference;

	/* Change the direction of the pointers. */
	saved_reference = current->next;
	current->next = retval;

	/* Advance to the next node. */
	retval = current;
	current = saved_reference;
    }

    return retval;
}

const netrc_entry *
search_netrc (netrc_entry *list, char *host, char *login)
{
    /* Look for the HOST in LIST. */
    while (list)
    {
	if (list->host && !strcmp(list->host, host))
	    if (list->login && !strcmp(list->login, login))
		/* We found a matching entry. */
		break;

	list = list->next;
    }

    /* Return the matching entry, or NULL. */
    return list;
}

void
free_netrc(netrc_entry *a) {
    while(a) {
	netrc_entry *n = a->next;
	if (a->password != NULL) {
		memset(a->password, 0x55, strlen(a->password));
		free(a->password);
	}
	xfree(a->login);
	xfree(a->host);
	xfree(a);
	a = n;
    }
}

#ifdef STANDALONE
int main (int argc, char **argv)
{
    struct stat sb;
    char *file, *host, *login;
    netrc_entry *head;
    const netrc_entry *a;

    program_name = argv[0];
    file = argv[1];
    host = argv[2];
    login = argv[3];

    switch (argc) {
	case 2:
	case 4:
	    break;
	default:
	    fprintf(stderr, "Usage: %s <file> [<host> <login>]\n", argv[0]);
	    exit(EXIT_FAILURE);
    }

    report_init(1);

    if (stat(file, &sb))
    {
	fprintf(stderr, "PRE-CHECK for %s: cannot stat %s: %s\n", argv[0], file,
		 strerror (errno));
    }

    head = parse_netrc(file);
    if (!head)
    {
	fprintf(stderr, "%s: read error or no entries found in %s\n", argv[0], file);
	exit (1);
    }

    if (host && login)
    {
	int status;
	status = EXIT_SUCCESS;

	printf("Host: %s, Login: %s\n", host, login);
	
	a = search_netrc (head, host, login);
	if (a)
	{
	    /* Print out the password (if any). */
	    if (a->password)
	    {
		printf("Password: %s\n", a->password);
	    }
	} else
	    status = EXIT_FAILURE;
	fputc ('\n', stdout);

	exit (status);
    }

    /* Print out the entire contents of the netrc. */
    for (a = head; a; a = a->next)
    {
	/* Print the host name. */
	if (a->host)
	    fputs (a->host, stdout);
	else
	    fputs ("DEFAULT", stdout);

	fputc ('|', stdout);

	/* Print the login name. */
	fputs (a->login, stdout);

	if (a->password)
	{
	    /* Print the password, if there is any. */
	    fputc ('|', stdout);
	    fputs (a->password, stdout);
	}

	fputs ("|\n", stdout);
    }

    free_netrc(head);

    exit (0);
}
#endif /* STANDALONE */
