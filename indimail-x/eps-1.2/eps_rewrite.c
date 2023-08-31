#if defined(MEM_DEBUG) || defined(COUNT_DEBUG)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "eps.h"

int
main(int argc, char *argv[])
{
	struct stat     st;
	struct line_t   ll;
	int             ret = 0, fd = 0, i = 0;
	struct header_t *h = NULL;
	struct eps_t   *eps = NULL;
	char           *l = NULL, mf = 0, *b = NULL, usebuf = 0;

#ifdef MEM_DEBUG
	mem_init();
#endif
#ifdef COUNT_DEBUG
	time_init();
#endif
	if (argc < 3) {
#ifdef MEM_DEBUG
		mem_kill();
#endif
		fprintf(stderr, "Usage: %s <usebuf> <files>\n", argv[0]);
		return 1;
	}
	usebuf = atoi(argv[1]);
	eps = NULL;
	for (i = 2; i < argc; i++) {
		if (usebuf) {
			if ((ret = stat(argv[i], &st)) == -1)
				continue;
			if (!S_ISREG(st.st_mode))
				continue;
		}
		mf = 0;
		if ((fd = open(argv[i], O_RDONLY)) == -1)
			continue;
		if (usebuf) {
			if (!(b = (char *) malloc(st.st_size + 1))) {
				close(fd);
				continue;
			}
			*(b + st.st_size + 1) = '\0';
			if ((ret = read(fd, b, st.st_size)) < st.st_size) {
				close(fd);
				continue;
			}
			ll.data = b;
			ll.bytes = st.st_size;
		}
#ifdef RESTART
		if (eps == NULL) {
			if (usebuf)
				eps = eps_begin(INTERFACE_BUFFER, &ll);
			else
				eps = eps_begin(INTERFACE_STREAM, &fd);
			if (!eps) {
#ifdef MEM_DEBUG
				mem_kill();
#endif
				return 1;
			}
		} else {
			if (usebuf)
				eps_restart(eps, &ll);
			else
				eps_restart(eps, &fd);
		}
#else
		if (usebuf)
			eps = eps_begin(INTERFACE_BUFFER, &ll);
		else
			eps = eps_begin(INTERFACE_STREAM, &fd);
		if (eps == NULL) {
#ifdef MEM_DEBUG
			mem_kill();
#endif
			return 1;
		}
#endif

#ifdef OUTPUT
		printf("%s:", argv[i]);
		if (usebuf)
			printf(" (%lu byte(s) in memory)", st.st_size);
		printf("\n");
#endif

		for (h = eps_next_header(eps); h; h = eps_next_header(eps)) {
#ifdef OUTPUT
			if ((h->name) && (h->data))
				printf("%s\n", h->orig);
#endif
			eps_header_free(eps);
		}
#ifdef OUTPUT
		printf("\n");
#endif
		for (l = eps_next_line(eps); l; l = eps_next_line(eps)) {
#ifdef OUTPUT
			printf("%s\n", l);
#endif
		}
		while ((!(eps->u->b->eof)) && (eps->content_type & CON_MULTI)) {
			mf = 0;
			if (!(ret = mime_init_stream(eps)))
				break;
			for (h = mime_next_header(eps); h; h = mime_next_header(eps)) {
#ifdef OUTPUT
				if ((h->name) && (h->data) && (*(h->name))) {
					if (!mf) {
						printf("--%s\n", eps->m->boundary);
						mf = 1;
					}
					printf("%s\n", h->orig);
				}
#endif
				header_kill(h);
			}
#ifdef OUTPUT
			if (mf)
				printf("\n");
#endif
			if (mf) {
				for (l = mime_next_line(eps); l; l = mime_next_line(eps)) {
#ifdef OUTPUT
					printf("%s\n", l);
#endif
				}
			}
#ifdef OUTPUT
			if (eps->m->depth == -1)
				//            printf("--%s--\n", eps->m->boundary);
				printf("%s\n", eps->m->orig ? eps->m->orig : "*** UH OH ***");
#endif
		}
		/*
		 * Clean up the remaining boundaries
		 */
		while (eps->content_type & CON_MULTI) {
			if (!(l = boundary_fetch(eps, eps->b->cdepth)))
				break;
#ifdef OUTPUT
			printf("--%s--\n", l);
#endif
			boundary_remove_last(eps);
		}
		close(fd);
#ifndef RESTART
		eps_end(eps);
#endif
		if (usebuf)
			free(b);
	}
#ifdef RESTART
	eps_end(eps);
#endif
#ifdef COUNT_DEBUG
	time_compare();
#endif
#ifdef MEM_DEBUG
	mem_kill();
#endif
	return 0;
}
