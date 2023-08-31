#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h>
#include <stdlib.h>
#include "eps.h"

int
atom_begin(struct header_t *h)
{
	if (!(h->atoms = (struct atom_t *) mmalloc(sizeof(struct atom_t), "atom_begin")))
		return 0;
	h->atoms->next = NULL;
	h->atail = h->atoms;
	return 1;
}

struct atom_t  *
atom_new(struct header_t *h)
{
	struct atom_t  *a = NULL;

	if ((a = (struct atom_t *) mmalloc(sizeof(struct atom_t), "atom_new"))) {
		memset((struct atom_t *) a, 0, sizeof(struct atom_t));
		a->next = NULL;
		h->atail->next = a;
		h->atail = a;
	}
	return a;
}

int
atom_kill(struct atom_t *al)
{
	struct atom_t  *a = NULL, *oa = NULL;

	if (!al)
		return 0;
	a = al;
	while (a->next) {
		oa = a->next;
		a->next = a->next->next;
		if (oa->name)
			mfree(oa->name);
		if (oa->data)
			mfree(oa->data);
		mfree(oa);
	}
	mfree(al);
	return 1;
}

struct header_t *
header_alloc(void)
{
	struct header_t *h = NULL;

	if (!(h = (struct header_t *) mmalloc(sizeof(struct header_t), "header_alloc")))
		return NULL;
	memset((struct header_t *) h, 0, sizeof(struct header_t));
	return h;
}

struct header_t *
header_parse(unsigned char *line)
{
	struct header_t *hdr = NULL;
	unsigned char  *data = NULL, *nlc = NULL, *orig = NULL;

	if (!(hdr = header_alloc()))
		return NULL;
	if (!(orig = mstrdup(line)))
		return hdr;
	if (!(nlc = rfc2822_remove_comments(line))) {
		mfree(orig);
		return hdr;
	}
	data = rfc2822_next_token(nlc, ':', (unsigned char *) " ");
	if ((!data || !*data) || (data && *data == ' ')) {
		mfree(orig);
		mfree(nlc);
		return hdr;
	}
	*data++ = '\0';
	if (!*data || !*nlc) {
		mfree(orig);
		mfree(nlc);
		return hdr;
	}
	hdr->orig = orig;
	hdr->name = mstrdup(nlc);
	while (*data == ' ')
		data++;
	hdr->data = mstrdup(data);
	hdr->atoms = header_fetch_atoms(hdr, data);
	mfree(nlc);
	return hdr;
}

/*
 * Return an atom linked list if the header
 * is structured.  Otherwise, return NULL.
 * 
 * FORMAT: atom; atom_name=atom_data; etc
 */
struct atom_t  *
header_fetch_atoms(struct header_t *hh, unsigned char *data)
{
	unsigned char  *h = NULL, *t = NULL, *p = NULL;

	if (!data)
		return NULL;
	else
	if (!*data)
		return NULL;
#ifdef DEBUG
	fprintf(stderr, "HEADER FETCH ATOMS: BEGIN\n<-- %s\n", data);
#endif
	atom_begin(hh);
	for (p = t = data; ((h = rfc2822_next_token(p, ';', NULL)) != NULL);) {
#ifdef DEBUG
		fprintf(stderr, "HEADER FETCH ATOMS: Looping on atom token\n");
#endif
		if (*h) {
			/*
			 * Remove WSP after atom
			 */
			if (rfc2822_is_wsp(*(h - 1))) {
				h--;
				for (; rfc2822_is_wsp(*h); h--);
				++h;
				*h = '\0';
			}
			*h++ = '\0';
			p = h;
		} else {
			/*
			 * Remove WSP at the end of the line
			 * set h to NULL so we know we're done.
			 */
			if (rfc2822_is_wsp(*(h - 1))) {
				--h;
				for (; rfc2822_is_wsp(*h); h--);
				h++;
				*h = '\0';
			}
			h = NULL;
		}
		/*
		 * Skip any WSP before atom
		 */
		for (; rfc2822_is_wsp(*t); t++);
		if (*t)
			header_parse_atom(hh, t);
#ifdef DEBUG
		else
			fprintf(stderr, "HEADER FETCH ATOMS: Atom is blank\n");
#endif
		if (!h)
			break;
		t = p;
	}
	return hh->atoms;
}

/*
 * Parse just one peice of atom data.
 * We are not handed the trailing semi-colons
 * 
 * FORMATS:
 * data
 * name = ["]data["]
 * 
 * Returns 1 on success, 0 on 'failure'
 */
int
header_parse_atom(struct header_t *hh, unsigned char *data)
{
	struct atom_t  *a = NULL;
	unsigned char  *h = NULL, *t = NULL, *p = NULL;

	if (!data || !*data)
		return 1;
#ifdef DEBUG
	fprintf(stderr, "HEADER PARSE ATOM: BEGIN\n<-- %s\n", data);
#endif
	/*
	 * What's the point in allocating before we
	 * even know if we have data.  Let's not
	 * allocate space for empty atoms.
	 * 
	 * Not sure if this breaks RFCs or not :)
	 * <vol@inter7.com>
	 * 
	 * a = atom_new();
	 */

	a = NULL;
	h = t = data;
	h = rfc2822_next_token(data, '=', NULL);
	/*
	 * No equal sign.
	 */
	if (!h || !*h) {
#ifdef DEBUG
		fprintf(stderr, "HEADER PARSE ATOM: No variable definition; just data\n");
#endif

		if (!*data) {
#ifdef DEBUG
			fprintf(stderr, "HEADER PARSE ATOM: Blank atom, no allocations made\n");
#endif
			return 1;
		}
		p = rfc2822_convert_literals(data);
		if (*p) {
			a = atom_new(hh);
			a->data = mstrdup(p);
		}
#ifdef DEBUG
		else
			fprintf(stderr, "HEADER PARSE ATOM: Blank atom, no allocations made\n");
#endif
		mfree(p);
		return 1;
	}
	*h++ = '\0';
	if (!*t) {
#ifdef DEBUG
		fprintf(stderr, "HEADER PARSE ATOM: Blank atom, no allocations made\n");
#endif
		return 1;
	}
	p = rfc2822_convert_literals(t);
	if (*p) {
		a = atom_new(hh);
		a->name = mstrdup(p);
		mfree(p);
		p = rfc2822_convert_literals(h);
		if (!*p) {
#ifdef DEBUG
			fprintf(stderr, "HEADER PARSE ATOM: Blank atom, no allocations made\n");
#endif
			mfree(p);
			return 1;
		}
		a->data = mstrdup(p);
		mfree(p);
	} else {
#ifdef DEBUG
		fprintf(stderr, "HEADER PARSE ATOM: Blank atom, no allocations made\n");
#endif
		mfree(p);
		return 1;
	}
#ifdef DEBUG
	if (a) {
		fprintf(stderr, "HEADER PARSE ATOM: New atom\n");
		if (a->name)
			fprintf(stderr, "  %s=%s\n", a->name, a->data);
		else
			fprintf(stderr, "  %s\n", a->data);
	} else
		fprintf(stderr, "HEADER PARSE ATOM: Blank atom, no allocations made\n");
#endif
	return 1;
}

void
header_kill(struct header_t *h)
{
	if (!h)
		return;
	if (h->name)
		mfree(h->name);
	if (h->data)
		mfree(h->data);
	if (h->orig)
		mfree(h->orig);
	if (h->atoms)
		atom_kill(h->atoms);
	mfree(h);
}

#ifdef DEBUG
/*
 * Debugging.  Show header data.
 */
void
header_show(struct header_t *h)
{
	struct atom_t  *a = NULL;

	if (!h)
		return;
	if (h->name == NULL || h->data == NULL)
		return;
	fprintf(stderr, "HEADER:\n" \
		"  Name: [%s]\n" \
		"  Original data: [%s]\n", h->name, h->data);
	if (h->atoms) {
		fprintf(stderr, "  ATOMS:\n");
		for (a = h->atoms; a->next; a = a->next) {
			if (a->next->name)
				fprintf(stderr, "    [%s] = [%s]\n", a->next->name, a->next->data);
			else
				fprintf(stderr, "    [%s]\n", a->next->data);
		}
	}
}
#endif

/*
 * Return an atom's data by atom name from
 * a header structure
 */
unsigned char  *
header_fetch_atom(struct header_t *h, unsigned char *name)
{
	struct atom_t  *a = NULL;

	if (!h->atoms)
		return NULL;
	for (a = h->atoms; a->next; a = a->next) {
		if (a->next->name) {
			if (!strcasecmp((const char *) a->next->name, (const char *) name))
				return a->next->data;
		}
	}
	return NULL;
}
