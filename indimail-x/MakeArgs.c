/*
 * $Log: MakeArgs.c,v $
 * Revision 1.1  2019-04-18 08:30:15+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <str.h>
#include <alloc.h>
#include <stralloc.h>
#include <env.h>
#endif

#define isEscape(ch) ((ch) == '"' || (ch) == '\'')

/*
 * function to expand a string into command line
 * arguments. To free memory allocated by this
 * function the following should be done
 *
 * free(argv); free(argv[0]);
 *
 */
char          **
MakeArgs(char *cmmd)
{
	char           *ptr, *marker;
	char          **argv;
	int             argc, idx;
	static stralloc sptr = { 0 };

	for (ptr = cmmd;*ptr && isspace((int) *ptr);ptr++);
	idx = str_len(ptr);
	if (!stralloc_copys(&sptr, ptr))
		return ((char **) 0);
	if (!stralloc_0(&sptr))
		return ((char **) 0);
	/*-
	 * Get the number of arguments by counting
	 * white spaces. Allow escape via the double
	 * quotes character at the first word
	 */
	for (argc = 0, ptr = sptr.s;*ptr;) {
		for (;*ptr && isspace((int) *ptr);ptr++);
		if (!*ptr)
			break;
		argc++;
		marker = ptr;
		/*- Move till you hit the next white space */
		for (;*ptr && !isspace((int) *ptr);ptr++) {
			/*-
			 * 1. If escape char is encounted skip till you
			 *    hit the terminating escape char
			 * 2. If terminating escape char is missing, come
			 *    back to the start escape char
			 */
			if (ptr == marker && isEscape(*ptr)) {
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
			}
		} /*- for(;*ptr && !isspace((int) *ptr);ptr++) */
	} /*- for (argc = 0, ptr = sptr.s;*ptr;) */
	/*
	 * Allocate memory to store the arguments
	 * Do not bother extra bytes occupied by
	 * white space characters.
	 */
	if (!(argv = (char **) alloc((argc + 1) * sizeof(char *))))
		return ((char **) 0);
	for (idx = 0, ptr = sptr.s;*ptr;) {
		for (;*ptr && isspace((int) *ptr);ptr++)
			*ptr = 0;
		if (!*ptr)
			break;
		if (*ptr == '$')
			argv[idx++] = env_get(ptr + 1);
		else
			argv[idx++] = ptr;
		marker = ptr;
		for (;*ptr && !isspace((int) *ptr);ptr++) {
			if (ptr == marker && isEscape(*ptr)) {
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
				else { /*- Remove the quotes */
					argv[idx - 1] += 1;
					*ptr = 0;
				}
			}
		}
	} /*- for (idx = 0, ptr = sptr.s;*ptr;) */
	argv[idx++] = (char *) 0;
	return (argv);
}

void
FreeMakeArgs(char **argv)
{
	alloc_free((char *) argv);
	return;
}
