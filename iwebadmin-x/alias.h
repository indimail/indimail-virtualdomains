/*
 * $Id: alias.h,v 1.2 2019-06-03 06:45:55+05:30 Cprogrammer Exp mbhangui $
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
#ifndef ALIAS_H_H
#define ALIAS_H_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif

void            adddotqmail();
void            adddotqmailnow();
int             adddotqmail_shared(char *, stralloc *, int);
void            deldotqmail();
void            deldotqmailnow();
char           *dotqmail_alias_command(const char *);
void            moddotqmail();
void            moddotqmailnow();
int             show_aliases(void);
void            show_dotqmail_lines(const char *, const char *, time_t);
void            show_dotqmail_file(const char *);

#endif
