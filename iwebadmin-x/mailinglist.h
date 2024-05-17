/*
 * $Id: mailinglist.h,v 1.4 2019-07-15 12:47:13+05:30 Cprogrammer Exp mbhangui $
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

#include <time.h>
#include <stralloc.h>

void            addlistgroup(const char *template);
void            addlistgroupnow(int mod);
void            addmailinglist();
void            addmailinglistnow();
void            count_mailinglists();
void            dellistgroup(const char *template);
void            dellistgroupnow(int mod);
void            delmailinglist();
void            delmailinglistnow();
int             ezmlm_sub(int mod, const char *email);
void            modmailinglist();
void            modmailinglistnow();
void            show_list_group(const char *template);
void            show_list_group_now(int mod);
void            show_mailing_lists();
void            show_mailing_list_line(const char *user, const char *dom, time_t mytime, const char *dir);
void            show_mailing_list_line2(const char *user, const char *dom, time_t mytime, const char *dir);
void            show_list_group_now(int mod);
void            show_current_list_values();
int             get_mailinglist_prefix(stralloc *prefix);
