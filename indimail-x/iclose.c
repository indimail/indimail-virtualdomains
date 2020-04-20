/*
 * $Log: iclose.c,v $
 * Revision 1.2  2019-05-28 17:39:19+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-04-14 18:29:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "variables.h"
#include "dbload.h"
#include "load_mysql.h"

#ifndef	lint
static char     sccsid[] = "$Id: iclose.c,v 1.2 2019-05-28 17:39:19+05:30 Cprogrammer Exp mbhangui $";
#endif

void
iclose()
{
	/*
	 * disconnection from the database 
	 */
#ifdef CLUSTERED_SITE
	if (isopen_vauthinit[0] || isopen_vauthinit[1])
	{
		if (is_open)
			mysql_host.len = 0;
		if (isopen_cntrl)
			cntrl_host.len = 0;
		close_db();
		if (isopen_cntrl == 1 && !isopen_vauthinit[0])
			in_mysql_close(&mysql[0]);
		if (is_open == 1 && !isopen_vauthinit[1])
			in_mysql_close(&mysql[1]);
		isopen_vauthinit[0] = isopen_vauthinit[1] = 0;
		is_open = isopen_cntrl = 0;
		return;
	}
#endif
	if (is_open)
		mysql_host.len = 0;
	if (is_open == 1) {
		is_open = 0;
		in_mysql_close(&mysql[1]);
	} 
#ifdef CLUSTERED_SITE
	else
	if (is_open == 2)
		is_open = 0;
	if (isopen_cntrl)
		cntrl_host.len = 0;
	if (isopen_cntrl == 1) {
		isopen_cntrl = 0;
		in_mysql_close(&mysql[0]);
	} else
	if (isopen_cntrl == 2)
		isopen_cntrl = 0;
#endif
}
