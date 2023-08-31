#include "eps.h"

int
int_stream_init(struct eps_t *e, int *args)
{
	if (!(e->u = unroll_init(*args, MAX_LINE_LENGTH)))
		return 0;
	return 1;
}

void
int_stream_restart(struct eps_t *e, int *args)
{
	unroll_restart(e->u, *args);
}
