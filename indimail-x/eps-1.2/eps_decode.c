#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "eps.h"

int
main(int argc, char *argv[])
{
	int             ret = 0;
	struct base64_t b;
	FILE           *stream = NULL;
	unsigned long   len = 0;
	unsigned char   buf[500] = { 0 };
	struct line_t  *ld = NULL;

	if (argc < 2)
		return 1;
	if (!(stream = fopen(argv[1], "r")))
		return 1;
	base64_init(&b);
	if (!(ld = line_alloc())) {
		fclose(stream);
		return 1;
	}
	while (!(feof(stream))) {
		memset(buf, 0, 500);
		fgets(buf, 500, stream);
		if (!(ret = base64_decode(&b, ld, buf))) {
			printf("Error decoding\n");
			return 1;
		}
	}
	fclose(stream);
	for (len = 0; len < ld->bytes; len++)
		putchar(*(ld->data + len));
	line_kill(ld);
	return 0;
}
