/*
 * $Id: xmalloc.c,v 1.4 2025-01-22 15:52:05+05:30 Cprogrammer Exp mbhangui $
 *
 * (C) Copyright 1993,1994 by Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Carnegie
 * Mellon University not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  Carnegie Mellon University makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char           *
xmalloc(int size)
{
	char           *ret;

	if ((ret = malloc((unsigned) size)))
		return ret;

	fprintf(stderr, "Memory exhausted\n");
	exit(1);
}


char           *
xrealloc(char *ptr, int size)
{
	char           *ret;

	/*
	 * xrealloc (NULL, size) behaves like xmalloc (size), as in ANSI C 
	 */
	if ((ret = !ptr ? malloc((unsigned) size) : realloc(ptr, (unsigned) size)))
		return ret;

	fprintf(stderr, "Memory exhausted\n");
	exit(1);
}

char           *
strsave(char *str)
{
	char           *p = xmalloc(strlen(str) + 1);
	strcpy(p, str);
	return p;
}
/*-
 * $Log: xmalloc.c,v $
 * Revision 1.4  2025-01-22 15:52:05+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.3  2008-05-21 18:45:11+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2004-01-06 14:56:00+05:30  Manny
 * fixed compilation warnings
 *
 * Revision 1.1  2004-01-06 12:44:23+05:30  Manny
 * Initial revision
 *
 */
