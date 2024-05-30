/*
 * $Id: util.h,v 1.4 2024-05-30 23:06:02+05:30 Cprogrammer Exp mbhangui $
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

#ifndef _UTIL_H
#define _UTIL_H

#include <stralloc.h>

int             check_local_user(const char *user);
int             fixup_local_name(char *addr);
int             check_email_addr(char *addr);
int             check_indimail_alias(const char *, stralloc *);
int             open_lang(char *lang);
int             open_colortable();
const char     *strstart(const char *sstr, const char *tstr);
const char     *safe_getenv(const char *var);
const char     *get_html_text(const char *index);
const char     *get_color_text(const char *index);
void            upperit(char *instr);
void            ack(const char *msg, const char *extra);
void            show_counts();

/*
 * prototypes for sorting functions in util.c 
 */
int             sort_init();
int             sort_add_entry(const char *, char);
unsigned char  *sort_get_entry(int);
void            sort_cleanup();
void            sort_dosort();
void            str_replace(char *, char, char);

void            qmail_button(const char *modu, const char *command, const char *user, const char *dom, time_t mytime, const char *png);

int             quota_to_bytes(char[], const char *);	//jhopper prototype
int             quota_to_megabytes(char[], const char *);	//jhopper prototype

void            print_user_index(const char *action, int colspan, const char *user, const char *dom, time_t mytime);
char           *cgiurl(const char *action);

#endif
