/*
* NAME:
*    xmalloc.c -- front-end to standard heap manipulation routines, with error checking.
*
* AUTHOR:
*    Gyepi Sam <gyepi@praxis-sw.com>
*
*/

#include "config.h"

#include "xmalloc.h"

void *
xmalloc(size_t size){
    void *ptr;
    ptr = bf_malloc(size);
    if (ptr == NULL && size == 0)
	ptr = bf_malloc(1);
    if (ptr == NULL)
	xmem_error("xmalloc"); 
    return ptr;
}

void
xfree(void *ptr){
    if (ptr)
	bf_free(ptr);
}
