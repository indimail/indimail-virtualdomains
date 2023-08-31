#ifdef DEBUG
#include <stdio.h>
#endif
#include <string.h>
#include <stdlib.h>
#include "eps_buffer.h"
#include "eps_line.h"
#include "eps_unroll.h"
#include "eps_misc.h"

struct unroll_t *
unroll_alloc(void)
{
	struct unroll_t *u = NULL;

	if (!(u = (struct unroll_t *) mmalloc(sizeof(struct unroll_t), "unroll_alloc")))
		return NULL;
	memset((struct unroll_t *) u, 0, sizeof(struct unroll_t));
	return u;
}

struct unroll_t *
unroll_init(int fd, unsigned long blen)
{
	int             ret = 0;
	struct unroll_t *u = NULL;

	if (!(u = unroll_alloc()))
		return NULL;
	u->b = buffer_init(fd, blen);
	if (u->b == NULL) {
		unroll_kill(u);
		return NULL;
	}
	u->l = line_alloc();
	if (u->l == NULL) {
		unroll_kill(u);
		return NULL;
	}
	if (!(ret = line_init(u->l, NULL, blen))) {
		unroll_kill(u);
		return NULL;
	}
	u->lb = line_alloc();
	if (u->lb == NULL) {
		unroll_kill(u);
		return NULL;
	}
	if (!(ret = line_init(u->lb, NULL, blen))) {
		unroll_kill(u);
		return NULL;
	}
	return u;
}

void
unroll_kill(struct unroll_t *u)
{
	if (!u)
		return;
	if (u->b)
		buffer_kill(u->b);
	if (u->l)
		line_kill(u->l);
	if (u->lb)
		line_kill(u->lb);
	mfree(u);
}

void
unroll_restart(struct unroll_t *u, int fd)
{
	u->eof = 0;
	line_restart(u->l);
	line_restart(u->lb);
	buffer_restart(u->b, fd);
}

char           *
unroll_next_line(struct unroll_t *u)
{
	int             ret = 0;
	char           *p = NULL;

	if (u->eof)
		return NULL;
	if (u->l->bytes)
		line_restart(u->l);
	/*
	 * Any saved data goes into the main buffer
	 */
	if (u->lb->bytes) {
		if (!(ret = line_inject(u->l, u->lb->data, u->lb->bytes))) {
			u->eof = 1;
			return NULL;
		}
		/*
		 * ..and the saving buffer gets restarted
		 */
		line_restart(u->lb);
	}
	while (1) {
		if (!(p = buffer_next_line(u->b))) {
			if (u->l->bytes) {
				u->eof = 1;
				return u->l->data;
			}
			return NULL;
		}
		/*
		 * Blank lines end the unroll
		 */
		if (!*p) {
			if (u->l->bytes) {
				u->eof = 1;
				return u->l->data;
			}
			return NULL;
		}
		/*
		 * Rolled lines always get appended
		 */
		if (*p == ' ' || *p == '\t') {
#ifdef DEBUG
			fprintf(stderr, "ROLLING: [%s]\n", (p + 1));
#endif
			if (!(ret = line_inject(u->l, (p + 1), (u->b->l->bytes - 1)))) {
				u->eof = 1;
				return NULL;
			}
			continue;
		}
		/*
		 * If we have data already in the line buffer,
		 * we need to back up the line we just read,
		 * and return the current data.
		 */
		if (u->l->bytes) {
#ifdef DEBUG
			fprintf(stderr, "SAVING: [%s]\n", p);
#endif
			if (!(ret = line_inject(u->lb, p, u->b->l->bytes))) {
				u->eof = 1;
				return NULL;
			}
			return u->l->data;
		}
		/*
		 * Otherwise just inject into the buffer and continue
		 */
#ifdef DEBUG
		fprintf(stderr, "INJECTING: [%s]\n", p);
#endif
		if (!(ret = line_inject(u->l, p, u->b->l->bytes))) {
			u->eof = 1;
			return NULL;
		}
	}
	return NULL;
}
