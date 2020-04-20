/*
 * $Log: mysql_query.c,v $
 * Revision 1.2  2019-05-28 17:41:45+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-14 18:34:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <str.h>
#endif
#include "replacestr.h"
#ifdef mysql_query
#undef mysql_query
#endif
#include <mysql.h>
#include <mysqld_error.h>
#include "load_mysql.h"

#ifndef	lint
static char     sccsid[] = "$Id: mysql_query.c,v 1.2 2019-05-28 17:41:45+05:30 Cprogrammer Exp mbhangui $";
#endif

static int      _es_opt;

int
_mysql_Query(MYSQL *mysql, char *query_str)
{
	int             i, ret, len, end;
	char           *ptr;
	static stralloc buf = {0}, rbuf = {0};

	ptr = env_get("ENABLE_MYSQL_ESCAPE");
	if (!ptr || _es_opt)
		return (in_mysql_query(mysql, query_str));
	len = str_len(query_str);
	i = (len * 2) + 1;
	if (!stralloc_ready(&buf, i))
		return (-1);
	end = in_mysql_real_escape_string(mysql, buf.s, query_str, len);
	buf.s[end] = 0;
	ret = in_mysql_query(mysql, buf.s);
	rbuf.len = 0;
	if (in_mysql_errno(mysql) == ER_PARSE_ERROR) { /*- NO_BACKSLASH_ESCAPES stupidity */
		if ((i = replacestr(buf.s, "\\\"", "'", &rbuf)) == -1)
			return (-1);
		ret = in_mysql_query(mysql, i == 0 ? buf.s : rbuf.s);
	}
	return (ret);
}

int
disable_mysql_escape(int opt)
{
	int             orig_opt;

	orig_opt = _es_opt;
	_es_opt = opt;
	return (orig_opt);
}
