/*
 * $Log: deldomain.c,v $
 * Revision 1.1  2019-04-18 08:16:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "indimail.h"
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <error.h>
#include <strerr.h>
#include <substdio.h>
#include <open.h>
#include <getln.h>
#endif
#include "variables.h"
#include "vlimits.h"
#include "lowerit.h"
#include "open_master.h"
#include "is_alias_domain.h"
#include "sql_deldomain.h"
#include "sql_delaliasdomain.h"
#include "getEnvConfig.h"
#include "get_real_domain.h"
#include "dir_control.h"
#include "del_control.h"
#include "get_assign.h"
#include "vdelfiles.h"
#include "remove_line.h"
#include "vfilter_delete.h"
#include "del_domain_assign.h"
#include "autoturn_dir.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: deldomain.c,v 1.1 2019-04-18 08:16:56+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("deldomain: out of memory", 0);
	_exit(111);
}

int
deldomain(char *domain)
{
	char            inbuf[512];
	static stralloc Dir = {0}, tmpbuf = {0}, BasePath = {0}, line = {0};
	char           *ptr, *tmpstr, *base_path;
	int             is_alias, i, fd, match;
	struct substdio ssin;
	uid_t           uid;
	gid_t           gid;
	char           *FileSystems[] = {
		"A2E",
		"F2K",
		"L2P",
		"Q2S",
		"T2Zsym",
	};

	if (!domain || !*domain) {
		strerr_warn1("deldomain: domain name cannot be null", 0);
		return (-1);
	}
	lowerit(domain);
	if (use_etrn == 2) {
		if (!(ptr = autoturn_dir(domain))) {
			strerr_warn3("deldomain: domain ", domain, " does not exist", 0);
			return (-1);
		} else
		if (!(tmpstr = get_assign("autoturn", 0, &uid, &gid))) {
			strerr_warn3("deldomain: domain ", domain, " does not exist: No entry for autoturn in assign", 0);
			return (-1);
		}
		if (!stralloc_copys(&tmpbuf, tmpstr) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, ptr) || !stralloc_0(&tmpbuf))
			die_nomem();
		if (vdelfiles(tmpbuf.s, 0, ptr) != 0) {
			strerr_warn3("deldomain: Failed to remove dir ", tmpbuf.s, ": ", &strerr_sys);
			return (-1);
		}
		if (!stralloc_copys(&tmpbuf, tmpstr) ||
				!stralloc_catb(&tmpbuf, "/.qmail-", 8) ||
				!stralloc_cats(&tmpbuf, ptr) ||
				!stralloc_catb(&tmpbuf, "-default", 8) || !stralloc_0(&tmpbuf))
			die_nomem();
		i = str_rchr(tmpbuf.s, '/');
		if (tmpbuf.s[i]) {
			ptr = tmpbuf.s + i + 1;
			if (*ptr == '.' && *(ptr + 1) == 'q')
				ptr++;
			for(; *ptr; ptr++) {
				if (*ptr == '.')
					*ptr = ':';
			}
			if (!access(tmpbuf.s, F_OK) && unlink(tmpbuf.s)) {
				strerr_warn3("deldomain: unlink: ", tmpbuf.s, ": ", &strerr_sys);
				return (-1);
			}
		}
		del_control(domain);
		return (0);
	}
	if (!get_assign(domain, &Dir, &uid, &gid)) {
		strerr_warn3("deldomain: domain ", domain, " does not exist", 0);
		return (-1);
	}
	if (!stralloc_copy(&tmpbuf, &Dir) ||
			!stralloc_catb(&tmpbuf, "/.base_path", 11) || !stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1 && errno != error_noent) {
		strerr_warn3("deldomain: read: ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	BasePath.len = 0;
	if (fd > -1) {
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("deldomain: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (line.len == 0)
			strerr_warn3("deldomain", tmpbuf.s, "incomplete line", 0);
		else
		if (match) {
			line.len--; /*- remove newline */
			if (line.len == 0)
				strerr_warn3("deldomain", tmpbuf.s, "incomplete line", 0);
			else {
				if (!stralloc_copy(&BasePath, &line) || !stralloc_0(&BasePath))
					die_nomem();
				BasePath.len--;
			}
		}
		close(fd);
	}
	if (!stralloc_copy(&tmpbuf, &Dir) ||
			!stralloc_catb(&tmpbuf, "/.aliasdomains", 14) || !stralloc_0(&tmpbuf))
		die_nomem();
	if ((is_alias = is_alias_domain(domain)) == 1) {
		if (verbose) {
			out("deldomain", "Removing alias domain ");
			out("deldomain", domain);
			out("deldomain", "\n");
			flush("deldomain");
		}
#ifdef CLUSTERED_SITE
		i = open_master();
		if (i && i != 2) {
			strerr_warn1("deldomain: Failed to open master db", 0);
			return (-1);
		}
		if (!i && get_real_domain(domain) && sql_delaliasdomain(domain)) {
			strerr_warn3("deldomain: Failed to remove alias domain ", domain, " from aliasdomain table", 0);
			return (-1);
		}
#endif
		if (remove_line(domain, tmpbuf.s, 0, 0640) == -1) {
			strerr_warn4("deldomain: Failed to remove alias domain ", domain, " from ", tmpbuf.s, 0);
			return (-1);
		}
	} else {
		if ((fd = open_read(tmpbuf.s)) == -1 && errno != error_noent)
			strerr_die3sys(111, "deldomain: ", tmpbuf.s, ": ");
		if (fd > -1) {
			if (verbose)
				strerr_warn2("deldomain: Removing domains aliased to ", domain, 0);
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			for(;;) {
				if (getln(&ssin, &line, &match, '\n') == -1) {
					strerr_warn3("deldomain: read: ", tmpbuf.s, ": ", &strerr_sys);
					break;
				}
				if (!match && line.len == 0)
					break;
				if (match) {
					line.len--;
					line.s[line.len] = 0; /*- remove newline */
				}
				match = str_chr(line.s, '#');
				if (line.s[match])
					line.s[match] = 0;
				for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
				if (!*ptr)
					continue;
				strerr_warn2("deldomain: removing alias domain ", ptr, 0);
				if (deldomain(ptr)) {
					strerr_warn2("deldomain: error removing alias domain ", ptr, 0);
					close(fd);
					return (-1);
				}
			}
			close(fd);
		}
	}
	if (is_alias != 1) {
		if (vdel_dir_control(domain)) {
			strerr_warn2("deldomain: vdel_dir_control: Failed to remove dir_control for ", domain, 0);
			return (-1);
		}
#ifdef ENABLE_DOMAIN_LIMITS
		vdel_limits(domain);
#endif
	}
	if (!stralloc_copyb(&tmpbuf, "prefilt@", 8) || !stralloc_cats(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
		die_nomem();
	vfilter_delete(tmpbuf.s, -1);
	if (!stralloc_copyb(&tmpbuf, "postfilt@", 9) || !stralloc_cats(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
		die_nomem();
	vfilter_delete(tmpbuf.s, -1);

	/*- delete the assign file line */
	if (del_domain_assign(domain, Dir.s, uid, gid))
		return (-1);

	/* delete the Mail File systems */
	if (is_alias != 1) {
		if (BasePath.len)
			base_path = BasePath.s;
		else
			getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
		for (i = 0; i < 5; i++) {
			if (!stralloc_copys(&tmpbuf, base_path) ||
					!stralloc_append(&tmpbuf, "/") ||
					!stralloc_cats(&tmpbuf, FileSystems[i]) ||
					!stralloc_append(&tmpbuf, "/") ||
					!stralloc_cats(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
				die_nomem();
			if (verbose) {
				out("deldomain", "Removing ");
				out("deldomain", tmpbuf.s);
				out("deldomain", "\n");
				flush("deldomain");
			}
			if (vdelfiles(tmpbuf.s, "", domain)) {
				strerr_warn3("deldomain: Failed to remove dir ", tmpbuf.s, ": ", &strerr_sys);
				continue;
			}
		}
	}
	/*- Delete /var/indimail/domains/domain_name */
	if (vdelfiles(Dir.s, "", domain) != 0) {
		strerr_warn3("deldomain: Failed to remove dir ", Dir.s, ": ", &strerr_sys);
		return (-1);
	}

	/*-
	 * call the auth module to delete the domain from the authentication
	 * database
	 */
	if (is_alias != 1 && sql_deldomain(domain))
		return (-1);
	/*- delete the email domain from the qmail control files */
	if (del_control(domain) == -1)
		return (-1);
	return (0);
}
