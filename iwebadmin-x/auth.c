/*
 * $Id: auth.c,v 1.9 2024-05-30 22:51:54+05:30 Cprogrammer Exp mbhangui $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies, Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#define REMOVE_MYSQL_H
#include <indimail_compat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <substdio.h>
#include <getln.h>
#include <open.h>
#include <strerr.h>
#include <str.h>
#include <scan.h>
#endif
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "cgi.h"
#include "show.h"
#include "util.h"
#include "common.h"

//extern char *crypt();

void
auth_system(ip_addr, pw)
	const char     *ip_addr;
	struct passwd  *pw;
{
	time_t          time1, time2;
	int             fd, match;
#ifdef IPAUTH
	static stralloc ip_value = {0};
#endif
	char            inbuf[1024];
	struct substdio ssin;

	if (chdir(RealDir.s) == -1) {
		copy_status_mesg(html_text[171]);
		if (!stralloc_append(&StatusMessage, " ") ||
				!stralloc_cat(&StatusMessage, &RealDir) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		iclose();
		iweb_exit(SETUP_FAILURE);
	}

	if (!stralloc_copys(&TmpBuf, pw->pw_dir) ||
			!stralloc_catb(&TmpBuf, "/Maildir/", 9) ||
			!stralloc_cat(&TmpBuf, &Time) ||
			!stralloc_catb(&TmpBuf, ".qw", 3) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) == -1) {
		copy_status_mesg(html_text[172]);
		show_login();
		iclose();
		iweb_exit(SESSION_FAILURE);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn3("auth_system: read: ", TmpBuf.s, ": ", &strerr_sys);
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .qw", 4) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		close(fd);
		show_login();
		iclose();
		iweb_exit(SESSION_FAILURE);
	}
	close(fd);

#ifdef IPAUTH
	GetValue(line.s, &ip_value, "ip_addr=");
	if (str_diff(ip_addr, ip_value.s) != 0) {
		unlink(TmpBuf.s);
		copy_status_mesg(html_text[142]);
		if (!stralloc_catb(&StatusMessage, " 4 (", 4) ||
				!stralloc_cats(&StatusMessage, ip_addr) ||
				!stralloc_catb(&StatusMessage, " != ", 4) ||
				!stralloc_catb(&StatusMessage, ip_value.s, ip_value.len) ||
				!stralloc_catb(&StatusMessage, ")\n", 2) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		show_login();
		iclose();
		iweb_exit(PERM_FAILURE);
	}
#endif
	scan_ulong(Time.s, (unsigned long *) &time1);
	time2 = time(0);
	if (time2 > time1 + 7200) {
		unlink(TmpBuf.s);
		copy_status_mesg(html_text[173]);
		show_login();
		iclose();
		iweb_exit(EXPIRE_FAILURE);
	}
}

void
auth_user_domain(const char *ip_addr, struct passwd *pw)
{
	time_t          time1, time2;
	int             match, fd;
#ifdef IPAUTH
	static stralloc ip_value = {0};
#endif
	char            inbuf[1024];
	struct substdio ssin;

	if (chdir(RealDir.s) == -1) {
		copy_status_mesg(html_text[171]);
		show_login();
		iclose();
		iweb_exit(SETUP_FAILURE);
	}
	if (!stralloc_copys(&TmpBuf, pw->pw_dir) ||
			!stralloc_catb(&TmpBuf, "/Maildir/", 9) ||
			!stralloc_cat(&TmpBuf, &Time) ||
			!stralloc_catb(&TmpBuf, ".qw", 3) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	if ((fd = open_read(TmpBuf.s)) == -1) {
		copy_status_mesg(html_text[172]);
		show_login();
		iclose();
		iweb_exit(SESSION_FAILURE);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	if (getln(&ssin, &line, &match, '\n') == -1) {
		strerr_warn3("auth_system: read: ", TmpBuf.s, ": ", &strerr_sys);
		copy_status_mesg(html_text[144]);
		if (!stralloc_catb(&StatusMessage, " .qw", 4) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		close(fd);
		show_login();
		iclose();
		iweb_exit(SESSION_FAILURE);
	}
	close(fd);
#ifdef IPAUTH
	GetValue(line.s, &ip_value, "ip_addr=");
	if (str_diff(ip_addr, ip_value.s)) {
		unlink(TmpBuf.s);
		copy_status_mesg(html_text[142]);
		if (!stralloc_catb(&StatusMessage, " 6 (", 4) ||
				!stralloc_cats(&StatusMessage, ip_addr) ||
				!stralloc_catb(&StatusMessage, " != ", 4) ||
				!stralloc_catb(&StatusMessage, ip_value.s, ip_value.len) ||
				!stralloc_catb(&StatusMessage, ")\n", 2) ||
				!stralloc_0(&StatusMessage))
			die_nomem();
		show_login();
		iclose();
		iweb_exit(PERM_FAILURE);
	}
#endif
	scan_ulong(Time.s, (unsigned long *) &time1);
	time2 = time(0);
	if (time2 > time1 + 7200) {
		unlink(TmpBuf.s);
		copy_status_mesg(html_text[173]);
		show_login();
		iclose();
		iweb_exit(EXPIRE_FAILURE);
	}
}

void
set_admin_type(struct passwd *pw)
{
	if (Domain.len) {
		if (!str_diffn(Username.s, "postmaster", Username.len))
			AdminType = DOMAIN_ADMIN;
		else
		if (pw->pw_gid & QA_ADMIN)
			AdminType = DOMAIN_ADMIN;
		else
			AdminType = USER_ADMIN;
	} else
		AdminType = NO_ADMIN;
}
