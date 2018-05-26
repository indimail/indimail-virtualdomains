/*
 * $Log: ipalloc.c,v $
 * Revision 1.6  2018-05-26 12:41:01+05:30  Cprogrammer
 * fixed typo with #undef
 *
 * Revision 1.5  2004-10-22 20:25:58+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-11 13:54:24+05:30  Cprogrammer
 * prevent inclusion of ipalloc.h with prototypes
 *
 * Revision 1.3  2004-10-09 23:23:56+05:30  Cprogrammer
 * prevent inclusion of prototype from ipalloc.h
 *
 * Revision 1.2  2004-07-17 21:19:15+05:30  Cprogrammer
 * added RCS log
 *
 */
#define _ALLOC_
#include "alloc.h"
#undef _ALLOC_
#include "gen_allocdefs.h"
#include "ip.h"
#define _IPALLOC_
#include "ipalloc.h"
#undef _IPALLOC_

GEN_ALLOC_readyplus(ipalloc, struct ip_mx, ix, len, a, i, n, x, 10, ipalloc_readyplus)
GEN_ALLOC_append(ipalloc, struct ip_mx, ix, len, a, i, n, x, 10, ipalloc_readyplus, ipalloc_append)

void
getversion_ipalloc_c()
{
	static char    *x = "$Id: ipalloc.c,v 1.6 2018-05-26 12:41:01+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
