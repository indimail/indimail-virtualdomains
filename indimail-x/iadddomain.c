/*
 * $Id: iadddomain.c,v 1.5 2023-12-13 00:34:13+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <substdio.h>
#include <str.h>
#include <fmt.h>
#include <env.h>
#include <strerr.h>
#include <error.h>
#endif
#include "add_control.h"
#include "del_control.h"
#include "vmake_maildir.h"
#include "r_mkdir.h"
#include "update_file.h"
#include "get_assign.h"
#include "variables.h"
#include "sql_adddomain.h"
#include "add_domain_assign.h"
#include "del_domain_assign.h"
#include "CreateDomainDirs.h"
#include "vdelfiles.h"

#ifndef	lint
static char     sccsid[] = "$Id: iadddomain.c,v 1.5 2023-12-13 00:34:13+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("adddomain: out of memory", 0);
	_exit(111);
}

int
iadddomain(char *domain, char *ipaddr, char *dir, uid_t uid, gid_t gid, int chk_rcpt)
{
	char           *ptr, *controldir;
	struct substdio ssout;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], tmp[2], outbuf[256];
	static stralloc tmpbuf = {0};
	int             i, fd;
	mode_t          omask;

	tmp[1] = 0;
	if (!domain || !*domain || *domain == '-') {
		strerr_warn1("adddomain: invalid domain name", 0);
		return (-1);
	}
	if (str_len(domain) > MAX_PW_DOMAIN) {
		strerr_warn3("adddomain: ", domain, ": name to long", 0);
		return (-1);
	}
	if (!dir || !*dir) {
		strerr_warn1("adddomain: directory cannot be null", 0);
		return (-1);
	}
	/*- check invalid email domain characters */
	for (ptr = domain, i = 0; *ptr; ptr++, i++) {
		if ((*ptr == '-') || (*ptr == '.'))
			continue;
		if (i > MAX_PW_DOMAIN) {
			strerr_warn3("adddomain: ", domain, ": name to long", 0);
			return (-1);
		}
		if (!isalnum((int) *ptr)) {
			tmp[0] = *ptr;
			tmp[1] = 0;
			strerr_warn4("adddomain: invalid char ", "'", tmp, "'", 0);
			return (-1);
		}
		if (isupper((int) *ptr))
			*ptr = tolower(*ptr);
	}
	if ((*(ptr - 1)) == '-') {
		strerr_warn1("adddomain: Last component cannot be '-'", 0);
		return (-1);
	}
	if (get_assign(domain, NULL, NULL, NULL)) {
		strerr_warn3("adddomain: domain ", domain, " exists", 0);
		return (-1);
	}
	if (use_etrn == 1) {
		if (!stralloc_copys(&tmpbuf, dir) || !stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
			die_nomem();
		if (vmake_maildir(tmpbuf.s, uid, gid, domain) == -1) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn7("adddomain: vmake_maildir(", tmpbuf.s, ", ", strnum1, ", ", strnum2, "): ", &strerr_sys);
			return (-1);
		}
	} else
	if (use_etrn == 2) {
		if (!stralloc_copys(&tmpbuf, dir) || !stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, ipaddr) || !stralloc_0(&tmpbuf))
			die_nomem();
		if (vmake_maildir(tmpbuf.s, uid, gid, domain) == -1) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn7("adddomain: vmake_maildir(", tmpbuf.s, ", ", strnum1, ", ", strnum2, "): ", &strerr_sys);
			return (-1);
		}
	} else
	if (!use_etrn) {
		if (!stralloc_copys(&tmpbuf, dir) || !stralloc_catb(&tmpbuf, "/domains/", 9) ||
				!stralloc_cats(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
			die_nomem();
		if (r_mkdir(tmpbuf.s, INDIMAIL_DIR_MODE, uid, gid)) {
			if (errno != EEXIST) {
				strnum1[fmt_ulong(strnum1, uid)] = 0;
				strnum2[fmt_ulong(strnum2, gid)] = 0;
				strerr_warn7("adddomain: r_mkdir(", tmpbuf.s, ", ", strnum1, ", ", strnum2, "): ", &strerr_sys);
				return (-1);
			}
		}
		if (!stralloc_copys(&tmpbuf, dir) || !stralloc_catb(&tmpbuf, "/domains/", 9) ||
				!stralloc_cats(&tmpbuf, domain) || !stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, (ptr = env_get("BULK_MAILDIR")) ? ptr : BULK_MAILDIR) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (r_mkdir(tmpbuf.s, INDIMAIL_DIR_MODE, uid, gid)) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn7("adddomain: r_mkdir(", tmpbuf.s, ", ", strnum1, ", ", strnum2, "): ", &strerr_sys);
			return (-1);
		}
	}
	if (use_etrn == 1) {
		if (!stralloc_copys(&tmpbuf, dir) ||
				!stralloc_catb(&tmpbuf, "/.qmail-", 8) ||
				!stralloc_cats(&tmpbuf, domain) ||
				!stralloc_catb(&tmpbuf, "-default", 8) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		i = str_len(dir);
		for (ptr = tmpbuf.s + i + 8; *ptr; ptr++) {
			if (*ptr == '.')
				*ptr = ':';
		}
	} else
	if (use_etrn == 2) {
		if (!stralloc_copys(&tmpbuf, dir) ||
				!stralloc_catb(&tmpbuf, "/.qmail-", 8) ||
				!stralloc_cats(&tmpbuf, ipaddr) ||
				!stralloc_catb(&tmpbuf, "-default", 8) || !stralloc_0(&tmpbuf))
			die_nomem();
		i = str_len(dir);
		for (ptr = tmpbuf.s + i + 8; *ptr; ptr++) {
			if (*ptr == '.')
				*ptr = ':';
		}
	} else {
		if (!stralloc_copys(&tmpbuf, dir) ||
				!stralloc_catb(&tmpbuf, "/domains/", 9) ||
				!stralloc_cats(&tmpbuf, domain) ||
				!stralloc_catb(&tmpbuf, "/.qmail-default", 15) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	omask = umask(0007);
	if ((fd = open(tmpbuf.s, O_CREAT|O_EXCL|O_WRONLY, 0660)) == -1) {
		strerr_warn3("adddomain: open: ", tmpbuf.s, ": ", &strerr_sys);
		umask(omask);
		return (-1);
	}
	umask(omask);
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if (use_etrn == 1) { /*- etrn, atrn */
		if (substdio_puts(&ssout, dir) ||
				substdio_put(&ssout, "/", 1) ||
				substdio_puts(&ssout, domain) ||
				substdio_put(&ssout, "/Maildir/\n", 10)) {
			strerr_warn3("adddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
			return (-1);
		}
	} else
	if (use_etrn == 2) { /*- autoturn */
		if (substdio_put(&ssout, "./", 2) ||
				substdio_puts(&ssout, ipaddr) ||
				substdio_put(&ssout, "/Maildir/\n", 10)) {
			strerr_warn3("adddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
			return (-1);
		}
	} else {
#ifdef VFILTER
		if (use_vfilter) {
			if (substdio_put(&ssout, "| ", 2) ||
					substdio_puts(&ssout, PREFIX) ||
					substdio_put(&ssout, "/sbin/vfilter '' bounce-no-mailbox\n", 35)) {
				strerr_warn3("adddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
				return (-1);
			}
		} else {
			if (substdio_put(&ssout, "| ", 2) ||
					substdio_puts(&ssout, PREFIX) ||
					substdio_put(&ssout, "/sbin/vdelivermail '' bounce-no-mailbox\n", 40)) {
				strerr_warn3("adddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
				return (-1);
			}
		}
#else
		if (substdio_put(&ssout, "| ", 2) ||
				substdio_puts(&ssout, PREFIX) ||
				substdio_put(&ssout, "/sbin/vdelivermail '' bounce-no-mailbox\n", 40)) {
			strerr_warn3("adddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
			return (-1);
		}
#endif
	}
	if (substdio_flush(&ssout)) {
		strerr_warn3("adddomain: write error: ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	close(fd);
	if (chown(tmpbuf.s, uid, gid)) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn7("adddomain: chown(", tmpbuf.s, ", ", strnum1, ", ", strnum2, "): ", &strerr_sys);
		return (-1);
	}
	if (add_domain_assign(use_etrn == 0 ? domain : NULL, dir, uid, gid))
		return (-1);
	if (!use_etrn) {
		if (add_control(domain, domain))
			return (-1);
	} else
	if (use_etrn) {
		if (!stralloc_copyb(&tmpbuf, "autoturn-", 9) ||
				!stralloc_cats(&tmpbuf, use_etrn == 1 ? domain : ipaddr) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (add_control(domain, tmpbuf.s))
			return (-1);
	}
	if (!use_etrn) {
		if (sql_adddomain(domain)) {
			if (!stralloc_copys(&tmpbuf, dir) ||
					!stralloc_catb(&tmpbuf, "/domains/", 9) ||
					!stralloc_cats(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
				die_nomem();
			del_domain_assign(domain, tmpbuf.s, uid, gid);
			del_control(domain);
			vdelfiles(tmpbuf.s, 0, 0);
			return (-1);
		}
		if (chk_rcpt) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = CONTROLDIR;
			if (!stralloc_copys(&tmpbuf, controldir) ||
					!stralloc_catb(&tmpbuf, "/chkrcptdomains", 15) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			if (update_file(tmpbuf.s, domain, INDIMAIL_QMAIL_MODE)) {
				if (!stralloc_copys(&tmpbuf, dir) ||
						!stralloc_catb(&tmpbuf, "/domains/", 9) ||
						!stralloc_cats(&tmpbuf, domain) || !stralloc_0(&tmpbuf))
					die_nomem();
				del_domain_assign(domain, tmpbuf.s, uid, gid);
				del_control(domain);
				vdelfiles(tmpbuf.s, 0, 0);
				return (-1);
			}
		}
		CreateDomainDirs(domain, uid, gid);
	}
	return (0);
}

/*
 * $Log: iadddomain.c,v $
 * Revision 1.5  2023-12-13 00:34:13+05:30  Cprogrammer
 * call add_domain_assign for etrn/atrn domains
 *
 * Revision 1.4  2023-12-03 15:41:05+05:30  Cprogrammer
 * use same logic for ETRN, ATRN domains
 *
 * Revision 1.3  2023-11-26 19:52:49+05:30  Cprogrammer
 * fix .qmail-default for etrn, atrn domains
 *
 * Revision 1.2  2020-10-19 12:46:12+05:30  Cprogrammer
 * use /var/indomain/domains for domain/bulk_mail
 *
 * Revision 1.1  2019-04-14 18:31:49+05:30  Cprogrammer
 * Initial revision
 *
 */
