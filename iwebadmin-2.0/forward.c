/*
 * $Id: forward.c,v 1.3 2011-11-17 22:10:30+05:30 Cprogrammer Exp mbhangui $
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
#include <indimail.h>
#include <indimail_compat.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#endif
#include "alias.h"
#include "forward.h"
#include "iwebadmin.h"
#include "iwebadminx.h"
#include "template.h"
#include "show.h"
#include "util.h"
#include "common.h"

int
show_forwards(char *user, char *dom, time_t mytime)
{
	if (AdminType != DOMAIN_ADMIN) {
		copy_status_mesg(html_text[142]);
		iclose();
		exit(0);
	}
	count_forwards();
	if (CurForwards == 0 && CurBlackholes == 0) {
		copy_status_mesg(html_text[232]);
		show_menu(Username.s, Domain.s, mytime);
		iclose();
		exit(0);
	} else {
		send_template("show_forwards.html");
	}
	return 0;
}

void
count_forwards()
{
	char           *alias_line, *p1, *p2;
	static stralloc alias_name = {0}, domain = {0}, this_alias = {0};
	int             isforward, i;

	CurForwards = 0;
	CurBlackholes = 0;
	if (!stralloc_copy(&domain, &Domain) || !stralloc_0(&domain))
		die_nomem();
	domain.len--;
	alias_line = valias_select_all(&alias_name, &domain);
	while (alias_line) {
		if (!stralloc_copy(&this_alias, &alias_name) || !stralloc_0(&this_alias))
			die_nomem();
		this_alias.len--;
		if (*alias_line == '#')
			CurBlackholes++;
		else {
			isforward = 1;
			while (isforward && alias_line &&
					!str_diffn(this_alias.s, alias_name.s, this_alias.len > alias_name.len ? this_alias.len : alias_name.len)) {
				p1 = str_str(alias_line, "/ezmlm-");
				i = str_chr(alias_line, ' ');
				if (alias_line[i])
					p2 = alias_line + i;
				else
					p2 = (char *) 0;
				if (p1 && (!p2 || p1 < p2))
					isforward = 0;
				if (str_str(alias_line, "/autorespond "))
					isforward = 0;
				alias_line = valias_select_all(&alias_name, &domain);
			}
			if (isforward)
				CurForwards++;
		}
		/*
		 * burn through remaining lines for this alias, if necessary 
		 */
		while (alias_line &&
				!str_diffn(this_alias.s, alias_name.s, this_alias.len > alias_name.len ? this_alias.len : alias_name.len)) {
			alias_line = valias_select_all(&alias_name, &domain);
		}
	}
	return;
}
