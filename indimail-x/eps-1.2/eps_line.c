#ifdef DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "eps_line.h"
#include "eps_misc.h"

struct line_t  *
line_alloc(void)
{
	struct line_t  *l = NULL;

	if (!(l = (struct line_t *) mmalloc(sizeof(struct line_t), "line_alloc")))
		return NULL;
	memset((struct line_t *) l, 0, sizeof(struct line_t));
	return l;
}

/*
 * Set up the line buffer
 * Allow user to provide a pre-allocated buffer
 * for efficiency
 */
int
line_init(struct line_t *l, char *buffer, unsigned long size)
{
	unsigned long   s = 0;

	/*
	 * Pre-allocated buffer
	 */
	if (buffer) {
		if (buffer != l->data || l->size != size) {
			if (l->data)
				mfree(l->data);
			l->data = buffer;
			l->size = size;
		}
	} else {
		/*
		 * Allocate default space
		 */
		if (size == 0)
			s = DEFAULT_BUFFER_SIZE;
		else
			s = size;
		if (!(l->data = (char *) mmalloc(s + 1, "line_init")))
			return 0;
		l->size = s;
		*l->data = '\0';
	}
	return 1;
}

int
line_inject(struct line_t *l, char *data, unsigned long bytes)
{
	void           *p = NULL;
	unsigned long   addum = 0;

	/*
	 * Reallocate space
	 */
	if ((l->bytes + bytes) > l->size) {
		if (DEFAULT_BUFFER_ADD < ((l->bytes + bytes) - l->size))
			addum = (((l->bytes + bytes) - l->size) + DEFAULT_BUFFER_ADD + 1);
		else
			addum = DEFAULT_BUFFER_ADD;
#ifdef DEBUG
		fprintf(stderr, "line_inject: Reallocating: %d -> %d\n", l->size, l->size + addum + 1);
#endif
		if (!(p = realloc((char *) l->data, (l->size + addum + 1))))
			return 0;
		if (p != l->data)
			l->data = p;
		l->size += addum;
#ifdef DEBUG
		fprintf(stderr, "line_inject: Reallocated %p %lu -> %lu\n", l->data, (l->size - addum), l->size);
#endif
	}
#ifdef DEBUG
	fprintf(stderr, "line_inject: Injected %d [", bytes);
	line_print(data, bytes);
	fprintf(stderr, " -> [");
#endif
	memcpy((char *) (l->data + l->bytes), (char *) data, bytes);
	l->bytes += bytes;
	*(l->data + l->bytes) = '\0';
#ifdef DEBUG
	line_print(data, bytes);
	fprintf(stderr, "\n");
#endif
	return 1;
}

void
line_restart(struct line_t *l)
{
	if (l->bytes) {
		*(l->data) = '\0';
		l->bytes = 0;
	}
}

void
line_kill(struct line_t *l)
{
	if (l->data)
		mfree(l->data);
	mfree(l);
}

#ifdef DEBUG
void
line_print(char *b, unsigned long len)
{
	char           *p = NULL;

	for (p = b; (*p && p < (b + len)); p++)
		putc(*p, stderr);
}
#endif
