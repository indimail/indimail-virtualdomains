/*
 * $Log: compat.c,v $
 * Revision 1.3  2019-05-02 14:39:55+05:30  Cprogrammer
 * added header get_assign.h
 *
 * Revision 1.2  2019-05-02 14:36:16+05:30  Cprogrammer
 * added compatibility for older vget_assign() function
 *
 * Revision 1.1  2019-04-18 07:45:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include  <stralloc.h>
#include  <str.h>
#endif
#include "iopen.h"
#include "iclose.h"
#include "sql_getpw.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "atrn_map.h"

int
vauth_open(char *dbhost)
{
	return (iopen(dbhost));
}

struct passwd  *
vauth_getpw(char *user, char *domain)
{
	return (sql_getpw(user, domain));
}

char           *
vget_real_domain(char *domain)
{
	return ((char *) get_real_domain(domain));
}

char           *
vshow_atrn_map(char **user, char **domain)
{
	return (show_atrn_map(user, domain));
}

void
vclose()
{
	iclose();
	return;
}

char *
vget_assign(char *domain, char *dir, int dir_len, uid_t *uid, gid_t *gid)
{
	static stralloc tmp_dir = {0};
	char           *ptr;

	if (dir) {
		ptr = get_assign(domain, &tmp_dir, uid, gid);
		str_copyb(dir, tmp_dir.s, tmp_dir.len <= dir_len ? tmp_dir.len : dir_len);
		return (ptr);
	} else
		return (get_assign(domain, 0, uid, gid));
}

int
iversion(stralloc *indimail_version, stralloc *module_version)
{
	if (!stralloc_copys(indimail_version, LIBVER) ||
			!stralloc_0(indimail_version))
		return (-1);
	indimail_version->len--;
	if (!stralloc_copys(module_version, MODVER) ||
			!stralloc_0(module_version))
		return (-1);
	module_version->len--;
	return (0);
}
