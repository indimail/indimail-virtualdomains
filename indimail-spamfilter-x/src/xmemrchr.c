/* (C) 2003 by David Relson, redistributable according to the terms
 * of the GNU General Public License, v2.
 */

#include "config.h"
#include <string.h>
#include "xmemrchr.h"

void *xmemrchr(void *v, byte b, size_t len) {
    byte *s = (byte *)v;
    byte *e = s + len;
    byte *a = NULL;
    while (s < e) {
	if (*s == b)
	    a = s;
	s += 1;
    }
    return a;
}
