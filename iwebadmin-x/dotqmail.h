/*
 * $Id: dotqmail.h,v 1.2 2024-05-17 16:20:51+05:30 mbhangui Exp mbhangui $
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

int             dotqmail_delete_files(const char *user);
int             dotqmail_add_line(const char *user, const char *line);
int             dotqmail_del_line(const char *user, const char *line);
#ifndef VALIAS
int             dotqmail_open_files(const char *user);
void            dotqmail_close_files(const char *user, int keep);
int             dotqmail_cleanup(const char *user, const char *line);
int             dotqmail_count(const char *user);
#endif
