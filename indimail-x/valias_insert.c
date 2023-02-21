/*
 * $Log: valias_insert.c,v $
 * Revision 1.1  2019-04-15 12:01:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <error.h>
#endif
#include "iopen.h"
#include "create_table.h"
#include "get_real_domain.h"
#include "findhost.h"
#include "addusercntrl.h"
#include "get_local_hostid.h"
#include "is_distributed_domain.h"
#include "islocalif.h"
#include "variables.h"
#include "common.h"

#ifdef VALIAS
#include <mysql.h>
#include <mysqld_error.h>

#ifndef	lint
static char     sccsid[] = "$Id: valias_insert.c,v 1.1 2019-04-15 12:01:14+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("valias_insert: out of memory", 0);
	_exit(111);
}

int
valias_insert(char *alias, char *domain, char *alias_line, int ignore)
{
	int             err, i;
	static stralloc SqlBuf = {0};
	char           *real_domain;
#ifdef CLUSTERED_SITE
	char           *mailstore, *ptr;
	static stralloc emailid = {0};
#endif

	if (!domain || !*domain)
		return (1);
	if (!alias_line || !*alias_line)
		return (1);
	if (!(real_domain = get_real_domain(domain)))
		real_domain = domain;
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1) {
		strerr_warn2(real_domain, ": is_distributed_domain failed", 0);
		return (1);
	} else
	if (err) {
		if (!stralloc_copys(&emailid, alias) ||
				!stralloc_append(&emailid, "@") ||
				!stralloc_cats(&emailid, real_domain) || !stralloc_0(&emailid))
			die_nomem();
		if (!(mailstore = findhost(emailid.s, 1))) {
			/*
			 * Get IP-Address of the Local machine
			 */
			if (!(ptr = get_local_hostid())) {
				strerr_warn3("valias_insert: could not get local hostid: ", emailid.s, ": ", &strerr_sys);
				return (-1);
			}
			if (addusercntrl(alias, real_domain, ptr, "alias", 0)) {
				strerr_warn1("valias_insert: Could not insert into central database", 0);
				return (1);
			}
		}
		i = str_chr(alias_line, '@');
		if (alias_line[i] && (mailstore = findhost(alias_line, 1)) != (char *) 0) {
			ptr = mailstore;
			i = str_rchr(mailstore, ':');
			if (mailstore[i]) {
				mailstore[i] = 0;
				for (; *mailstore && *mailstore != ':'; mailstore++);
				if (*mailstore == ':')
					mailstore++;
				else {
					mailstore[i] = ':';
					strerr_warn3("valias_insert: findhost: invalid route spec [", ptr, "]", 0);
					return (1);
				}
			} else {
				strerr_warn3("valias_insert: findhost: invalid route spec [", mailstore, "]", 0);
				return (1);
			}
			if (!ignore && !islocalif (mailstore)) {
				strerr_warn6(alias_line, "@", real_domain, " not local (mailstore ", mailstore, ")", 0);
				return (1);
			}
		}
	}
#endif
	if ((err = iopen((char *) 0)) != 0)
		return (err);
	while (*alias_line == ' ' && *alias_line != 0)
		++alias_line;
	if (!stralloc_copyb(&SqlBuf, "insert low_priority into valias ( alias, domain, valias_line ) values ( \"", 73) ||
			!stralloc_cats(&SqlBuf, alias) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, real_domain) ||
			!stralloc_catb(&SqlBuf, "\", \"", 4) ||
			!stralloc_cats(&SqlBuf, alias_line) ||
			!stralloc_catb(&SqlBuf, "\")", 2) ||
			!stralloc_0(&SqlBuf))
		die_nomem();
	if (mysql_query(&mysql[1], SqlBuf.s)) {
		if (in_mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[1], SqlBuf.s)) {
				strerr_warn4("valias_insert: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
				return (-1);
			}
		} else {
			strerr_warn4("valias_insert: mysql_query [", SqlBuf.s, "]: ", (char *) in_mysql_error(&mysql[1]), 0);
			return (-1);
		}
	}
	if ((err = in_mysql_affected_rows(&mysql[1])) == -1) {
		strerr_warn2("valias_insert: mysql_affected_rows: ", (char *) in_mysql_error(&mysql[1]), 0);
		return (-1);
	}
	if (!verbose)
		return (0);
	if (err) {
		out("valias_insert", "Added alias ");
		out("valias_insert", alias);
		out("valias_insert", "@");
		out("valias_insert", real_domain);
		out("valias_insert", "\n");
		flush("valias_insert");
	} else {
		strerr_warn5("valias_insert: ", "No alias ", alias, "@", real_domain, 0);
	}
	return (err ? 0 : 1);
}
#endif
