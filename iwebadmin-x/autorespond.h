/*
 * $Id: autorespond.h,v 1.1 2010-04-26 12:07:40+05:30 Cprogrammer Exp mbhangui $
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

void            addautorespond();
void            addautorespondnow();
void            count_autoresponders();
void            delautorespond();
void            delautorespondnow();
void            modautorespond();
void            modautorespondnow();
void            show_autoresponders(const char *user, const char *dom, time_t mytime);
void            show_autorespond_line(const char *user, const char *dom, time_t mytime, const char *dir);
