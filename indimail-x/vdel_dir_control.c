/*
 * $Log: vdel_dir_control.c,v $
 * Revision 1.2  2023-03-20 10:33:37+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.1  2019-04-15 12:40:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <open.h>
#include <error.h>
#include <getln.h>
#include <substdio.h>
#include <strerr.h>
#endif
#include "iopen.h"
#include "get_assign.h"
#include "remove_line.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdel_dir_control.c,v 1.2 2023-03-20 10:33:37+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("vdel_dir_control: out of memory", 0);
	_exit(111);
}

int
vdel_dir_control(char *domain)
{
	int             err, fd, match;
	char           *ptr;
	static stralloc SqlBuf = {0}, tmpbuf = {0}, line = {0};
	struct substdio ssin;
	char            inbuf[512];

	if (!(ptr = get_assign(domain, 0, 0, 0))) {
		strerr_warn2(domain, ": No such domain", 0);
		return (-1);
	}
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	if (!stralloc_copys(&tmpbuf, ptr) ||
			!stralloc_catb(&tmpbuf, "/.filesystems", 13) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent) {
			strerr_warn3("vdel_dir_control: ", tmpbuf.s, ": ", &strerr_sys);
			return (-1);
		}
		return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for(;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vdel_dir_control: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (!line.len)
			break;
		if (line.len < 4) {
			strerr_warn3("vdel_dir_control: ", tmpbuf.s, ": incomplete line", 0);
			close(fd);
			return (-1);
		}
		if (match)
			line.len--;
		if (!stralloc_catb(&SqlBuf, "delete low_priority from dir_control", 36) ||
				!stralloc_cat(&SqlBuf, &line) ||
				!stralloc_catb(&SqlBuf, " where domain = \"", 17) ||
				!stralloc_cats(&SqlBuf, domain) ||
				!stralloc_append(&SqlBuf, "\"") ||
				!stralloc_0(&SqlBuf))
			die_nomem();
		if (mysql_query(&mysql[1], SqlBuf.s)) {
			line.s[line.len] = 0; /*- remove newline */
			strerr_warn6("vdel_dir_control: dir_control", line.s, ": ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
			continue;
		}
		remove_line(line.s, tmpbuf.s, 0, 0640);
	}
	close(fd);
	return (0);
}
