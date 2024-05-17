/*
 * $Id: user.h,v 1.5 2024-05-17 16:20:51+05:30 mbhangui Exp mbhangui $
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

void            adduser();
void            addusernow();
void            bounceall();
int             call_hooks(const char *hook_type, const char *p1, const char *p2, const char *p3, const char *p4);
void            count_users();
void            deleteall();
void            ideluser();
void            delusergo();
void            delusernow();
void            get_catchall();
void            moduser();
void            modusergo();
void            modusernow();
int             migrate_vacation(const char *dir, const char *user);
void            parse_users_dotqmail(char newchar);
void            setremotecatchall();
void            setremotecatchallnow();
void            show_users();
int             show_user_lines(const char *user, const char *dom, time_t mytime, const char *dir);
