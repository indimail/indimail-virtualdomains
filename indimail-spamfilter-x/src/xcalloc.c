/*
* NAME:
*    xcalloc.c -- front-end to standard heap manipulation routines, with error checking.
*
* AUTHOR:
*    Gyepi Sam <gyepi@praxis-sw.com>
*
*/

#include "config.h"

#include "xmalloc.h"

void
*xcalloc(size_t nmemb, size_t size){
   void *ptr;
   ptr = bf_calloc(nmemb, size);
   if (ptr == NULL && (nmemb == 0 || size == 0))
       ptr = bf_calloc(1, 1);
   if (ptr == NULL)
       xmem_error("xcalloc");
   return ptr;
}
