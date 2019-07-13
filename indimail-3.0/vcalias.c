/*
 * $Log: vcalias.c,v $
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vcalias.c,v 1.3 2019-07-04 10:14:26+05:30 Cprogrammer Exp mbhangui $";
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
#include <fmt.h>
#include <getln.h>
#include <qprintf.h>
#endif
#include "get_assign.h"
#include "common.h"
#include "iopen.h"
#include "iclose.h"
#include "create_table.h"
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
	out("vcalias", "Looking in ");
	out("vcalias", Dir.s);
	out("vcalias", "\n");
	flush("vcalias");
	if (chdir(Dir.s)) {
		strerr_warn3("valias: chdir: ", Dir.s, ": ", &strerr_sys);
		return (-1);
	}
	/*- open the directory */
	if (!(entry = opendir(Dir.s))) {
		strerr_warn3("valias: opendir: ", Dir.s, ": ", &strerr_sys);
		return (-1);
	}
	/*- search for .qmail files */
	out("vcalias", "Converting from Filesystem to MYSQL for domain ");
	out("vcalias", Domain);
	out("vcalias", "\n");
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
		/*- printf("Converting %-20s", dp->d_name + 7); -*/
		AliasName= dp->d_name + 7;
		if ((fd = open_read(dp->d_name)) == -1) {
			strerr_warn3("vcalias: open: ", dp->d_name, ": ", &strerr_sys);
			return (-1);
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		for (err = 0, flag = 0;;flag++) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("vcalias: read: ", dp->d_name, ": ", &strerr_sys);
				close(fd);
			}
			if (!match && line.len == 0)
				break;
			line.len--;
			line.s[line.len] = 0;
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
						!stralloc_0(&tmp))
					die_nomem();
				qprintf(subfdoutsmall, "Converting ", "%s");
			} else
				qprintf(subfdoutsmall, "           ", "%s");
			qprintf(subfdoutsmall, Dir.s, "-%30s");
			qprintf(subfdoutsmall, " -> ", "%s");
			qprintf(subfdoutsmall, line.s, "%s");
			qprintf_flush(subfdoutsmall);
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
		close(fd);
		if (!err && unlink(dp->d_name))
			strerr_warn3("vcalias: unlink: ", dp->d_name, ": ", &strerr_sys);
	}
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
