/*-
 * $Log: parseAddress.c,v $
 * Revision 1.4  2023-08-31 22:36:43+05:30  Cprogrammer
 * handle address without domain component
 *
 * Revision 1.3  2023-08-06 15:35:09+05:30  Cprogrammer
 * BUGFIX - Fixed logic to parse addresses
 *
 * Revision 1.2  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.1  2019-04-18 08:31:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: parseAddress.c,v 1.4 2023-08-31 22:36:43+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <strerr.h>
#include <stralloc.h>
#include <subfd.h>
#endif
#include "storeHeader.h"
#include "common.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("parseAddress: out of memory", 0);
	_exit(111);
}

void
parseAddress(struct header_t *h, stralloc *addr_buf)
{
	struct group_t *g = (struct group_t *) 0;
	struct address_t *a = (struct address_t *) 0;

	addr_buf->len = 0;
	if (!(g = (struct group_t *) address_evaluate((char *) h->data))) {
		strerr_warn3("parseAddress: ", (char *) h->name, ": no valid addresses", 0);
		return;
	}
	if (verbose) {
		if (g->group)
			subprintfe(subfdout, "parseAddress", "%s (of group %s)\n", (char *) h->name, g->group);
		else
			subprintfe(subfdout, "parseAddress", "%s\n", (char *) h->name);
		flush("parseAddress");
	}
	for (a = g->members; a->next; a = a->next) {
		if (a->next->user && a->next->domain && *(a->next->domain)) {
			if (verbose) {
				if (a->next->name)
					subprintfe(subfdout, "parseAddress", "  (%s) { [%s] @ [%s] }\n",
							a->next->name ? a->next->name : "",
							a->next->user ? a->next->user : "N/A",
							a->next->domain ? a->next->domain : "N/A");
				else
					subprintfe(subfdout, "parseAddress", "  { [%s] @ [%s] }\n",
							a->next->user ? a->next->user : "N/A",
							a->next->domain ? a->next->domain : "N/A");
				flush("parseAddress");
			}
			if (!stralloc_copys(addr_buf, a->next->user) ||
					!stralloc_0(addr_buf))
				die_nomem();
			addr_buf->len--;
			if (!stralloc_append(addr_buf, "@") ||
					!stralloc_cats(addr_buf, a->next->domain) ||
					!stralloc_0(addr_buf))
				die_nomem();
			addr_buf->len--;
			if (a->next->next) {
				if (!stralloc_append(addr_buf, ",") || !stralloc_0(addr_buf))
					die_nomem();
				addr_buf->len--;
			}
		} else /*- no domain */
		if (a->next->user) {
			if (a->next->name)
				subprintfe(subfdout, "parseAddress", "  (%s) { [%s] @ [null] }\n",
						a->next->name ? a->next->name : "",
						a->next->user ? a->next->user : "N/A");
			else
				subprintfe(subfdout, "parseAddress", "  { [%s] @ [null] }\n",
						a->next->user ? a->next->user : "N/A");
			flush("parseAddress");
			if (!stralloc_copys(addr_buf, a->next->user) ||
					!stralloc_0(addr_buf))
				die_nomem();
			addr_buf->len--;
			if (a->next->next) {
				if (!stralloc_append(addr_buf, ",") || !stralloc_0(addr_buf))
					die_nomem();
				addr_buf->len--;
			}
		}
	}
	address_kill(g);
}
#endif
