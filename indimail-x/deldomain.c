/*
 * $Id: deldomain.c,v 1.8 2025-05-13 19:59:14+05:30 Cprogrammer Exp mbhangui $
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
#include <getEnvConfig.h>
#endif
#include "variables.h"
#include "vlimits.h"
#include "open_master.h"
#include "is_alias_domain.h"
#include "sql_deldomain.h"
#include "sql_delaliasdomain.h"
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
static char     sccsid[] = "$Id: deldomain.c,v 1.8 2025-05-13 19:59:14+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("deldomain: out of memory", 0);
	_exit(111);
}

int
remove_alias_domain(const char *domain, const char *alias_domain_file)
{
	int             i;
	uid_t           uid;
	gid_t           gid;
	static stralloc Dir = {0};

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
	if (remove_line(domain, alias_domain_file, 0, 0640) == -1) {
		strerr_warn4("deldomain: Failed to remove alias domain ", domain, " from ", alias_domain_file, 0);
		return (-1);
	}
	if (!get_assign(domain, &Dir, &uid, &gid)) {
		strerr_warn3("deldomain: alias domain ", domain, " does not exist", 0);
		return (0);
	}
	if (del_domain_assign(domain, Dir.s, uid, gid))
		return (-1);
	del_control(domain);
	return 0;
}

int
deldomain(const char *domain)
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
	if (use_etrn) {
		if (!(ptr = autoturn_dir(domain))) {
			strerr_warn3("deldomain: autoturn domain ", domain, " does not exist", 0);
			return (-1);
		} else
		if (!(tmpstr = get_assign("autoturn", 0, &uid, &gid))) {
			strerr_warn3("deldomain: autoturn domain ", domain, " does not exist: No entry for autoturn in assign", 0);
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
		return (0);
	}
	if (!stralloc_copy(&tmpbuf, &Dir) ||
			!stralloc_catb(&tmpbuf, "/.base_path", 11) || !stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent) {
			strerr_warn3("deldomain: read: ", tmpbuf.s, ": ", &strerr_sys);
			return (-1);
		}
	} else {
		BasePath.len = 0;
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("deldomain: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		close(fd);
		if (!line.len) {
			strerr_warn3("deldomain: ", tmpbuf.s, ": incomplete line", 0);
			return (-1);
		}
		if (match)
			line.len--; /*- remove newline */
		if (!line.len) {
			strerr_warn3("deldomain: ", tmpbuf.s, ": incomplete line", 0);
			return (-1);
		} else {
			if (!stralloc_copy(&BasePath, &line) || !stralloc_0(&BasePath))
				die_nomem();
			BasePath.len--;
		}
	}
	if (!stralloc_copy(&tmpbuf, &Dir) ||
			!stralloc_catb(&tmpbuf, "/.aliasdomains", 14) || !stralloc_0(&tmpbuf))
		die_nomem();
	if ((is_alias = is_alias_domain(domain)) == 1) {
		if (remove_alias_domain(domain, tmpbuf.s))
			return -1;
	} else {
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno != error_noent)
				strerr_die3sys(111, "deldomain: ", tmpbuf.s, ": ");
		} else {
			if (verbose) {
				out("deldomain", "Removing domains aliased to ");
				out("deldomain", domain);
				out("deldomain", "\n");
				flush("deldomain");
			}
			substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
			for(;;) {
				if (getln(&ssin, &line, &match, '\n') == -1) {
					strerr_warn3("deldomain: read: ", tmpbuf.s, ": ", &strerr_sys);
					close(fd);
					break;
				}
				if (!line.len) {
					close(fd);
					break;
				}
				if (match) {
					line.len--;
					if (!line.len)
						continue;
					line.s[line.len] = 0; /*- remove newline */
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
				if (!str_diff(domain, line.s)) /*- invalid entry */
					continue;
				if (verbose)
					strerr_warn2("deldomain: removing alias domain ", ptr, 0);
				if (remove_alias_domain(ptr, tmpbuf.s))
					return -1;
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
			if (access(tmpbuf.s, F_OK) && errno == error_noent)
				continue;
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
	if (!access(Dir.s, F_OK)) {
		if (verbose) {
			out("deldomain", "Removing ");
			out("deldomain", Dir.s);
			out("deldomain", "\n");
			flush("deldomain");
		}
		if (vdelfiles(Dir.s, "", domain) != 0) {
			strerr_warn3("deldomain: Failed to remove dir ", Dir.s, ": ", &strerr_sys);
			return (-1);
		}
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
/*
 * $Log: deldomain.c,v $
 * Revision 1.8  2025-05-13 19:59:14+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.7  2024-05-17 16:24:31+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.6  2023-12-03 15:40:29+05:30  Cprogrammer
 * use same logic for ETRN, ATRN domains
 *
 * Revision 1.5  2023-03-25 14:32:08+05:30  Cprogrammer
 * multiple bug fixes
 *
 * Revision 1.4  2023-03-20 09:57:24+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.3  2020-04-01 18:54:26+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-07-02 09:48:07+05:30  Cprogrammer
 * return success while deleting if a domain is not found in assign file
 *
 * Revision 1.1  2019-04-18 08:16:56+05:30  Cprogrammer
 * Initial revision
 *
 */
