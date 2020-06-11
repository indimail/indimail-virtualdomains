#include "config.h"
#include "fetchmail.h"
#include "i18n.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* similar to realpath(file_name, NULL) - on systems that do not natively 
 * support the resolved file name to be passed as NULL, malloc() the buffer of 
 * PATH_MAX + 1 bytes, or abort if the system does not define PATH_MAX
 */
char *fm_realpath(const char *restrict file_name)
{
	char *rv;

	if (NULL == file_name) {
		errno = EINVAL;
		return NULL;
	}

	rv = realpath(file_name, NULL);
		/* FreeBSD, recent GNU libc, and all SUSv4
		 * compliant systems auto-allocate => done */
	if (NULL == rv && errno == EINVAL) {
		/* Implementation does not auto-allocate, but
		 * PATH_MAX is provided (Solaris 10, f.i.) */
#if HAVE_DECL_PATH_MAX
		char *b = (char *)xmalloc(PATH_MAX + 1);
		rv = realpath(file_name, b);
#else
		/* unsupported */
		report(stderr, GT_("Your operating system neither defines PATH_MAX nor will it accept realpath(f, NULL). Aborting.\n"));
		abort();
#endif
	}
	return rv;
}

#ifdef TEST
#include <string.h>
const char *program_name= __FILE__;

int main(int argc, char **argv)
{
	int i;
	int e = EXIT_SUCCESS;
	for (i = 1; i < argc; ++i) {
		char *p = fm_realpath(argv[i]);
		if (p) {
			printf("%s -> %s\n", argv[i], p);
			free(p);
		} else {
			fprintf(stderr, "%s: error: %s\n", argv[i], strerror(errno));
			e = EXIT_FAILURE;
		}
	}
	exit(e);
}
#endif
