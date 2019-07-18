/*
 * $Id: iwebadminx.h,v 1.2 2019-06-03 06:47:21+05:30 Cprogrammer Exp mbhangui $
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
#include <sys/types.h>
#include <iwebadmin.h>

extern stralloc Username, Domain, Password, Gecos, Quota, Time, ActionUser, Newu,
				Password1, Password2, Crypted, Alias, LineData, Action, Message,
				StatusMessage, SearchUser, TmpBuf, RealDir, line;
extern int      CGIValues[256];
extern time_t   mytime;
extern char     Pagenumber[];
extern char    *TmpCGI;
extern int      Compressed, actout;
extern char    *html_text[MAX_LANG_STR + 1];

extern struct vlimits Limits;
extern int      num_of_mailinglist;
extern int      AdminType;
extern int      MaxPopAccounts;
extern int      MaxAliases;
extern int      MaxForwards;
extern int      MaxAutoResponders;
extern int      MaxMailingLists;

extern int      CallVmoduser;
extern int      DisablePOP;
extern int      DisableIMAP;
extern int      DisableDialup;
extern int      DisablePasswordChanging;
extern int      DisableWebmail;
extern int      DisableRelay;

extern int      CurPopAccounts;
extern int      CurForwards;
extern int      CurBlackholes;
extern int      CurAutoResponders;
extern int      CurMailingLists;

extern int      Uid;
extern int      Gid;
extern char     Lang[40];