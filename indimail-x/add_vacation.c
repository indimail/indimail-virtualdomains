/*
 * $Log: add_vacation.c,v $
 * Revision 1.2  2021-07-08 15:14:46+05:30  Cprogrammer
 * add missing error check
 *
 * Revision 1.1  2019-04-18 08:39:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifndef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#include <getln.h>
#include <substdio.h>
#endif
#include "parse_email.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "sql_getpw.h"
#include "r_mkdir.h"
#include "vdelfiles.h"
#include "common.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: add_vacation.c,v 1.2 2021-07-08 15:14:46+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("add_vacation: out of memory", 0);
	_exit(111);
}

int
add_vacation(char *email, char *fname)
{
	static stralloc tmpbuf = {0}, line = {0}, user = {0}, domain = {0};
	uid_t           uid;
	gid_t           gid;
	char           *real_domain;
	char            inbuf[512], outbuf[512];
	struct passwd  *pw;
	int             err, match, fd1, fd2;
	struct substdio ssin, ssout;

	parse_email(email, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s))) {
		strerr_warn3("add_vacation: ", domain.s, ": No such domain", 0);
		return (1);
	} else
	if (!get_assign(real_domain, 0, &uid, &gid)) {
		strerr_warn3("add_vacaton: ", real_domain, ": domain does not exist", 0);
		return (1);
	} else
	if (!(pw = sql_getpw(user.s, real_domain))) {
		strerr_warn4("vmoduser: no such user ", user.s, "@", real_domain, 0);
		return (1);
	}
	/*- Remove Vacation */
	if (fname && *fname && !str_diffn(fname, "-", 2)) {
		err = 0;
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (!access(tmpbuf.s, F_OK) && unlink(tmpbuf.s)) {
			strerr_warn3("add_vacation: ", tmpbuf.s, ": ", &strerr_sys);
			err = 1;
		}
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/.vacation.msg", 14) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (!access(tmpbuf.s, F_OK) && unlink(tmpbuf.s)) {
			strerr_warn3("add_vacation: ", tmpbuf.s, ": ", &strerr_sys);
			err = 1;
		}
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/.vacation.dir", 14) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if (!access(tmpbuf.s, F_OK) && vdelfiles(tmpbuf.s, user.s, 0))
			err = 1;
		return (err);
	}
	/*- Add vacation */
	if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
			!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd1 = open_trunc(tmpbuf.s)) == -1)
		strerr_die3sys(111, "add_vacation: open_trunc: ", tmpbuf.s, ": ");
	substdio_fdbuf(&ssout, write, fd1, outbuf, sizeof(outbuf));
	if (substdio_put(&ssout, "| ", 2) ||
			substdio_puts(&ssout, PREFIX) ||
			substdio_put(&ssout, "/bin/autoresponder -q ", 22) ||
			substdio_puts(&ssout, pw->pw_dir) ||
			substdio_put(&ssout, "/.vacation.msg ", 15) ||
			substdio_puts(&ssout, pw->pw_dir) ||
			substdio_put(&ssout, "/.vacation.dir\n", 15) ||
			substdio_puts(&ssout, pw->pw_dir) ||
			substdio_put(&ssout, "/Maildir/\n", 10)) {
		strerr_warn3("add_vacation: write", tmpbuf.s, ": ", &strerr_sys);
		close(fd1);
		return (1);
	}
	if (substdio_flush(&ssout)) {
		strerr_warn3("add_vacation: write error: ", tmpbuf.s, ": ", &strerr_sys);
		close(fd1);
		return (1);
	}
	close(fd1);
	if (chown(tmpbuf.s, uid, gid) || chmod(tmpbuf.s, INDIMAIL_QMAIL_MODE)) {
		strerr_warn3("add_vacation: chown/chmod", tmpbuf.s, ": ", &strerr_sys);
		unlink(tmpbuf.s);
		return (-1);
	}
	if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
			!stralloc_catb(&tmpbuf, "/.vacation.dir", 14) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if(r_mkdir(tmpbuf.s, INDIMAIL_DIR_MODE, uid, gid)) {
		strerr_warn3("add_vacation: r_mkdir", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	if (fname && *fname) {
		if (!str_diffn(fname, "+", 2)) {/*- Take text from STDIN */
			fd1 = 0;
			out("add_vacation", "Enter Text for Vacation Message. Type ^D (Ctrl-D) to end\n");
			flush("add_vacation");
		} else
		if ((fd1 = open_read(fname)) == -1){ /*- Take text from an existing file */
			strerr_warn3("add_vacaton: open_read: ", fname, ": ", &strerr_sys);
			if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
					!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			unlink(tmpbuf.s);
			return (1);
		}
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/.vacation.msg", 14) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if ((fd2 = open_trunc(tmpbuf.s)) == -1) {
			strerr_warn3("add_vacaton: open_trunc: ", tmpbuf.s, ": ", &strerr_sys);
			if (fd1)
				close(fd1);
			if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
					!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			unlink(tmpbuf.s);
			return (1);
		}
		if (chown(tmpbuf.s, uid, gid) || chmod(tmpbuf.s, INDIMAIL_QMAIL_MODE)) {
			strerr_warn3("add_vacation: chown/chmod", tmpbuf.s, ": ", &strerr_sys);
			if (fd1)
				close(fd1);
			close(fd2);
			unlink(tmpbuf.s);
			if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
					!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			unlink(tmpbuf.s);
			return (1);
		}
		substdio_fdbuf(&ssin, read, fd1, inbuf, sizeof(inbuf));
		substdio_fdbuf(&ssout, write, fd2, outbuf, sizeof(outbuf));
		for (;;) {
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("add_vacation: read: ", fd1 ? fname : "stdin", ": ", &strerr_sys);
				if (fd1)
					close(fd1);
				close(fd2);
				if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
						!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
				unlink(tmpbuf.s);
				return (1);
			}
			if (!line.len)
				break;
			if (substdio_put(&ssout, line.s, line.len)) {
				strerr_warn3("add_vacation: write error: ", tmpbuf.s, ": ", &strerr_sys);
				if (fd1)
					close(fd1);
				close(fd2);
				if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
						!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
				unlink(tmpbuf.s);
				return (1);
			}
		}
		if(substdio_flush(&ssout)) {
			strerr_warn3("add_vacation: write error: ", tmpbuf.s, ": ", &strerr_sys);
			if (fd1)
				close(fd1);
			close(fd2);
			if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
					!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			unlink(tmpbuf.s);
			return (1);
		}
		if (fd1)
			close(fd1);
		close(fd2);
		out("add_vacation", "Successfully wrote Vacation Message to ");
		out("add_vacation", tmpbuf.s);
		out("add_vaction", "\n");
	}
	return (0);
}
