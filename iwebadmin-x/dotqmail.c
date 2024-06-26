/*
 * $Id: dotqmail.c,v 1.6 2024-05-17 16:17:42+05:30 mbhangui Exp mbhangui $
 * Copyright (C) 1999-2004 Inter7 Internet Technologies 
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
#include "dotqmail.h"
#include "iwebadmin.h"
#include "iwebadminx.h"

#ifdef VALIAS
int
dotqmail_delete_files(const char *user)
{
	return (!valias_delete(user, Domain.s, 0));
}

int
dotqmail_add_line(const char *user, const char *line)
{
	return (valias_insert(user, Domain.s, (char *) line, 1));
}

int
dotqmail_del_line(const char *user, const char *line)
{
	return (valias_delete(user, Domain.s, (char *) line));
}
#endif
