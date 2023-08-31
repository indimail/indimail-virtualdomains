#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "eps.h"

int
main(int argc, char *argv[])
{
	int             ret = 0, fd = 0;
	char            buf[500] = { 0 };
	struct line_t  *l = NULL;

	if (argc < 2)
		return 1;
	if ((fd = open(argv[1], O_RDONLY)) == -1)
		return 1;
	if (!(l = line_alloc())) {
		close(fd);
		return 1;
	}
	line_init(l, NULL, 5000);
	while (1) {
		memset((char *) buf, 0, 500);
		if ((ret = read(fd, buf, 500)) < 1)
			break;
		line_inject(l, buf, ret);
	}
	if (!(ret = base64_encode(1, l)))
		printf("Error encoding\n");
	line_kill(l);
	close(fd);
	return 0;
}
