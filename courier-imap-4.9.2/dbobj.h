#ifndef	dbobj_h
#define	dbobj_h


/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	1
#include	"gdbmobj/gdbmobj.h"
#define	DbObj	GdbmObj

#define	dbobj	gdbmobj
#define	dbobj_init	gdbmobj_init
#define	dbobj_open	gdbmobj_open
#define	dbobj_close	gdbmobj_close
#define	dbobj_isopen	gdbmobj_isopen
#define	dbobj_fetch	gdbmobj_fetch
#define	dbobj_exists	gdbmobj_exists
#define	dbobj_delete	gdbmobj_delete
#define	dbobj_store	gdbmobj_store
#define	dbobj_firstkey	gdbmobj_firstkey
#define	dbobj_nextkey	gdbmobj_nextkey
#endif

#if	0
#include	"bdbobj/bdbobj.h"
#define	DbObj	BDbObj

#define	dbobj	bdbobj
#define	dbobj_init	bdbobj_init
#define	dbobj_open	bdbobj_open
#define	dbobj_close	bdbobj_close
#define	dbobj_isopen	bdbobj_isopen
#define	dbobj_fetch	bdbobj_fetch
#define	dbobj_exists	bdbobj_exists
#define	dbobj_delete	bdbobj_delete
#define	dbobj_store	bdbobj_store
#define	dbobj_firstkey	bdbobj_firstkey
#define	dbobj_nextkey	bdbobj_nextkey
#endif

#endif
