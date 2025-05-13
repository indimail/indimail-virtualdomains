/*
 * $Id: vcalias.c,v 1.7 2025-05-13 20:37:06+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vcalias.c,v 1.7 2025-05-13 20:37:06+05:30 Cprogrammer Exp mbhangui $";
#endif

#if defined(VALIAS)
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <mysql.h>
#include <mysqld_error.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <open.h>
#include <substdio.h>
#include <subfd.h>
#include <str.h>
#include <getln.h>
#endif
#include "get_assign.h"
#include "iopen.h"
#include "iclose.h"
#include "create_table.h"
#include "common.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vcalias: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	DIR            *entry;
	struct dirent  *dp;
	char           *ptr, *Domain, *AliasName;
	char            inbuf[4096];
	uid_t           Uid;
	gid_t           Gid;
	int             i, fd, err, flag, match;
	static stralloc line = {0}, Dir = {0}, tmp = {0}, SqlBuf = {0};
	struct substdio ssin;

	/*- get the command line arguments */
	 if (argc != 2) {
		ptr = argv[0];
		i = str_rchr(ptr, '/');
		if (ptr[i])
			ptr = argv[0] + i + 1;
		strerr_warn3("vcalias: Usage: ", ptr, " domain", 0);
		return (1);
	 }
	 Domain = argv[1];
	/*- find the directory */
	if (!get_assign(Domain, &Dir, &Uid, &Gid)) {
		strerr_warn3("could not find domain ", Domain, " in indimail assign file", 0);
		return (-1);
	}
	subprintfe(subfdout, "vcalias", "Looking in %s\n", Dir.s);
	flush("vcalias");
	if (chdir(Dir.s)) {
		strerr_warn3("vcalias: chdir: ", Dir.s, ": ", &strerr_sys);
		return (-1);
	}
	/*- open the directory */
	if (!(entry = opendir(Dir.s))) {
		strerr_warn3("vcalias: opendir: ", Dir.s, ": ", &strerr_sys);
		return (-1);
	}
	/*- search for .qmail files */
	subprintfe(subfdout, "vcalias", "Converting from Filesystem to MYSQL for domain %s\n", Domain);
	flush("vcalias");
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	for (;;) {
		if (!(dp = readdir(entry)))
			break;
		else
		if (!str_diffn(dp->d_name, ".qmail-default", 15))
			continue;
		else
		if (str_diffn(dp->d_name, ".qmail-", 7))
			continue;
		subprintfe(subfdout, "vcalias", "Processing %-20s\n", dp->d_name);
		AliasName= dp->d_name + 7;
		if ((fd = open_read(dp->d_name)) == -1) {
			strerr_warn3("vcalias: open: ", dp->d_name, ": ", &strerr_sys);
			iclose();
			return (-1);
		}
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		for (err = 0, flag = 0;;flag++) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("vcalias: read: ", dp->d_name, ": ", &strerr_sys);
				close(fd);
				iclose();
				return (-1);
			}
			if (!line.len)
				break;
			if (match) {
				line.len--;
				if (!line.len) {
					strerr_warn3("vcalias", dp->d_name, ": incomplete line", 0);
					continue;
				}
				line.s[line.len] = 0;
			} else {
				if (!stralloc_0(&line))
					die_nomem();
				line.len--;
			}
			match = str_chr(line.s, '#');
			if (line.s[match])
				line.s[match] = 0;
			for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			if (!flag) {
				if (!stralloc_copys(&tmp, dp->d_name + 7) ||
						!stralloc_append(&tmp, "@") ||
						!stralloc_cats(&tmp, Domain) ||
						!stralloc_0(&tmp)) {
					iclose();
					die_nomem();
				}
				subprintfe(subfdout, "vcalias", "Converting ");
			} else
				subprintfe(subfdout, "vcalias", "           ");
			subprintfe(subfdout, "vcalias", "%-30s -> %s\n", Dir.s, line.s);
			flush("vcalias");
			/*- Convert ':' to '.' */
			for (ptr = AliasName;*ptr;ptr++) {
				if (*ptr == ':')
					*ptr = '.';
			}
			if (!stralloc_copyb(&SqlBuf, "insert low_priority into valias (alias, domain, valias_line) ", 61) ||
					!stralloc_catb(&SqlBuf, "values (\"", 9) ||
					!stralloc_cats(&SqlBuf, AliasName) ||
					!stralloc_catb(&SqlBuf, "\", \"", 4) ||
					!stralloc_cats(&SqlBuf, Domain) ||
					!stralloc_catb(&SqlBuf, "\", \"", 4) ||
					!stralloc_catb(&SqlBuf, "\")", 2) ||
					!stralloc_0(&SqlBuf))
				die_nomem();
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
					if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT)) {
						close(fd);
						iclose();
						return (1);
					}
					if (!mysql_query(&mysql[1], SqlBuf.s))
						continue;
				}
				strerr_warn4("vcalias: mysql_query: ", SqlBuf.s, ": ", (char *) in_mysql_error(&mysql[1]), 0);
				err = 1;
				break;
			}
		} /*- for (err = 0, flag = 0;;flag++) */
		subprintfe(subfdout, "vcalias", "\n");
		flush("vcalias");
		close(fd);
		if (!err && unlink(dp->d_name))
			strerr_warn3("vcalias: unlink: ", dp->d_name, ": ", &strerr_sys);
	} /*- for (;;) */
	iclose();
	return (0);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-mysql=y and --enable-valias=y", 0);
	return (0);
}
#endif
/*
 * $Log: vcalias.c,v $
 * Revision 1.7  2025-05-13 20:37:06+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.6  2023-03-20 10:33:27+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.5  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.4  2020-04-09 18:32:08+05:30  Cprogrammer
 * close MySQL on exit and return on read error
 *
 * Revision 1.3  2019-07-04 10:14:26+05:30  Cprogrammer
 * fixed incorrect initialization by replacing stralloc_cats() with stralloc_copys()
 *
 * Revision 1.2  2019-04-22 23:16:52+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-15 11:16:50+05:30  Cprogrammer
 * Initial revision
 *
 */
