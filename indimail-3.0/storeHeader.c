/*-
 * $Log: storeHeader.c,v $
 * Revision 1.1  2019-04-18 08:36:27+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: storeHeader.c,v 1.1 2019-04-18 08:36:27+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <case.h>
#include <str.h>
#include <strerr.h>
#endif
#include "eps.h"
#include "storeHeader.h"
#include "parseAddress.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("storeHeader: out of memory", 0);
	_exit(111);
}

int
storeHeader(struct header ***Hptr, struct header_t *h)
{
	struct header **ptr, **hptr;
	static int      count;
	static stralloc tmp = {0};
	int             i, found, idx, len;

	if (!*h->data)
		return 0;
	if (!Hptr)
		hptr = (struct header **) 0;
	else
		hptr = *Hptr;
	if (!hptr) {
		if (!((hptr = (struct header **) alloc(sizeof(struct header *) * 2))))
			die_nomem();
		else
			*Hptr = hptr;
		if (!(hptr[0] = (struct header *) alloc(sizeof(struct header))))
			die_nomem();
		else
		if (!(hptr[0]->name = (char *) alloc((len = str_len((char *) h->name)) + 1)))
			die_nomem();
		else
			str_copyb(hptr[0]->name, (char *) h->name, len + 1);
		if (!(hptr[0]->data = (char **) alloc(sizeof(char *) * 2)))
			die_nomem();
		for (i = 0; i_headers[i]; i++) {
			if (!case_diffs(i_headers[i], (char *) h->name)) {
				parseAddress(h, &tmp);
				break;
			}
		}
		if (i_headers[i])
			len = tmp.len + 1;
		else
			len = str_len((char *) h->data) + 1;
		if (!(hptr[0]->data[0] = (char *) alloc(len)))
			die_nomem();
		str_copyb(hptr[0]->data[0], i_headers[i] ? tmp.s : (char *) h->data, len);
		hptr[0]->data[1] = (char *) 0;
		hptr[0]->data_items = 1;
		hptr[1] = (struct header *) 0;
		count = 1;
		return(0);
	} 
	for (ptr = hptr,found = idx = 0;idx < count;idx++) {
		if (!case_diffs(ptr[idx]->name, (char *) h->name)) {
			found = 1;
			break;
		} 
	}
	if (found) {
		if (!alloc_re((char *) &(ptr[idx]->data), sizeof(char *) * (ptr[idx]->data_items + 1), sizeof(char *) * (ptr[idx]->data_items + 2)))
			die_nomem();
		for (i = 0; i_headers[i]; i++) {
			if (!case_diffs(i_headers[i], (char *) h->name)) {
				parseAddress(h, &tmp);
				break;
			}
		}
		if (i_headers[i])
			len = tmp.len + 1;
		else
			len = str_len((char *) h->data) + 1;
		if (!(ptr[idx]->data[ptr[idx]->data_items] = (char *) alloc(len)))
			die_nomem();
		str_copyb(ptr[idx]->data[ptr[idx]->data_items++], i_headers[i] ? tmp.s : (char *) h->data, len);
		ptr[idx]->data[ptr[idx]->data_items] = (char *) 0;
	} else {
		if (!alloc_re((char *) &hptr, sizeof(struct header *) * (count + 1), sizeof(struct header *) * (count + 2)))
			die_nomem();
		else
			*Hptr = hptr;
		if (!(hptr[count] = (struct header *) alloc(sizeof(struct header))))
			die_nomem();
		else
		if (!(hptr[count]->name = (char *) alloc((len = str_len((char *) h->name)) + 1)))
			die_nomem();
		else
			str_copyb(hptr[count]->name, (char *) h->name, len + 1);
		if (!(hptr[count]->data = (char **) alloc(sizeof(char *) * 2)))
			die_nomem();
		for (i = 0; i_headers[i]; i++) {
			if (!case_diffs(i_headers[i], (char *) h->name)) {
				parseAddress(h, &tmp);
				break;
			}
		}
		if (i_headers[i])
			len = tmp.len + 1;
		else
			len = str_len((char *) h->data) + 1;
		if (!(hptr[count]->data[0] = (char *) alloc(len)))
			die_nomem();
		str_copyb(hptr[count]->data[0], i_headers[i] ? tmp.s : (char *) h->data, len);
		hptr[count]->data[1] = (char *) 0;
		hptr[count++]->data_items = 1;
		hptr[count] = (struct header *) 0;
	}
	return(0);
}
#endif
