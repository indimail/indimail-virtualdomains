/*
 * $Log: get_assign.c,v $
 * Revision 1.5  2020-10-18 07:47:19+05:30  Cprogrammer
 * use alloc() instead of alloc_re()
 *
 * Revision 1.4  2020-10-13 18:32:27+05:30  Cprogrammer
 * added missing alloc_free
 *
 * Revision 1.3  2020-04-01 18:59:39+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-04-10 10:09:16+05:30  Cprogrammer
 * replaced errout with strerr_warn1
 *
 * Revision 1.1  2019-03-05 17:01:27+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <stralloc.h>
#include <strerr.h>
#include <alloc.h>
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <getEnvConfig.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: get_assign.c,v 1.5 2020-10-18 07:47:19+05:30 Cprogrammer Exp mbhangui $";
#endif

extern int      cdb_seek(int, unsigned char *, unsigned int, int *);
/*
 * get uid, gid, dir from users/assign with caching
 */
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1; /* cache is on by default */
#endif
static stralloc cdbfilename = { 0 };

static void
die_nomem()
{
	strerr_warn1("get_assign: out of memory", 0);
	_exit(111);
}

char *
get_assign(char *domain, stralloc *dir, uid_t *uid, gid_t *gid)
{
	int             dlen, i, fd;
	char           *s, *ptr, *assigndir, *tmpstr, *tmpbuf;
	static char    *in_domain = 0;
	static char    *in_dir = 0;
	static int      o_alloc = 0, in_domain_size = 0;
	static int      in_dir_size = 0;
	static uid_t    in_uid = -1;
	static gid_t    in_gid = -1;

	if (!domain || !*domain)
		return ((char *) 0);
	for (s = domain; *s; s++) {
		if (isupper(*s))
			*s = tolower(*s);
	}
#ifdef QUERY_CACHE
	if (_cacheSwitch && env_get("QUERY_CACHE")) {
		if (in_domain_size && in_domain && in_dir &&
			!str_diffn(in_domain, domain, in_domain_size + 1)) {
			if (uid)
				*uid = in_uid;
			if (gid)
				*gid = in_gid;
			if (dir) {
				if (!stralloc_copys(dir, in_dir) || !stralloc_0(dir))
					die_nomem();
				dir->len--;
			}
			return (in_dir);
		}
	}
	if (!_cacheSwitch) {
		in_uid = in_gid = -1;
		if (in_domain)
			alloc_free(in_domain);
		if (in_dir)
			alloc_free(in_dir);
		in_domain = in_dir = (char *) 0;
		o_alloc = in_domain_size = in_dir_size = 0;
		_cacheSwitch = 1;
	}
#endif
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	if (!stralloc_copys(&cdbfilename, assigndir) ||
		!stralloc_catb(&cdbfilename, "/cdb", 4) ||
		!stralloc_0(&cdbfilename))
		die_nomem();
	i = str_len(domain) + 1;
	if (i > o_alloc && o_alloc)
		alloc_free(in_domain);
	if (i > o_alloc && !(in_domain = alloc(i))) {
		if (uid)
			*uid = -1;
		if (gid)
			*gid = -1;
		if (dir)
			dir->len = 0;
		in_uid = in_gid = -1;
		if (in_domain)
			alloc_free(in_domain);
		if (in_dir)
			alloc_free(in_dir);
		in_domain = in_dir = (char *) 0;
		o_alloc = in_domain_size = in_dir_size = 0;
		return ((char *) 0);
	}
	if (i > o_alloc)
		o_alloc = i;
	in_domain_size = i - 1;
	str_copyb(in_domain, domain, i); /*- copy with null */
	if (!(tmpstr = alloc(in_domain_size + 3))) {
		if (uid)
			*uid = -1;
		if (gid)
			*gid = -1;
		if (dir)
			dir->len = 0;
		in_uid = in_gid = -1;
		if (in_domain)
			alloc_free(in_domain);
		if (in_dir)
			alloc_free(in_dir);
		in_domain = in_dir = (char *) 0;
		o_alloc = in_domain_size = in_dir_size = 0;
		return ((char *) 0);
	}
	s = tmpstr;
	s += fmt_strn(s, "!", 1);
	s += fmt_strn(s, domain, in_domain_size);
	s += fmt_strn(s, "-", 1);
	*s++ = 0;
	if ((fd = open(cdbfilename.s, O_RDONLY)) == -1) {
		if (uid)
			*uid = -1;
		if (gid)
			*gid = -1;
		if (dir)
			dir->len = 0;
		alloc_free(tmpstr);
		in_uid = in_gid = -1;
		if (in_domain)
			alloc_free(in_domain);
		if (in_dir)
			alloc_free(in_dir);
		in_domain = in_dir = (char *) 0;
		o_alloc = in_domain_size = in_dir_size = 0;
		return ((char *) 0);
	}
	if ((i = cdb_seek(fd, (unsigned char *) tmpstr, in_domain_size + 2, &dlen)) == 1) {
		if (!(tmpbuf = (char *) alloc(dlen + 1))) {
			close(fd);
			alloc_free(tmpstr);
			if (uid)
				*uid = -1;
			if (gid)
				*gid = -1;
			if (dir)
				dir->len = 0;
			in_uid = in_gid = -1;
			if (in_domain)
				alloc_free(in_domain);
			if (in_dir)
				alloc_free(in_dir);
			in_domain = in_dir = (char *) 0;
			o_alloc = in_domain_size = in_dir_size = 0;
			return ((char *) 0);
		}
		alloc_free(tmpstr);
		i = read(fd, tmpbuf, dlen);
		tmpbuf[dlen] = 0;
		for (ptr = tmpbuf; *ptr; ptr++);
		ptr++;
		scan_uint(ptr, &in_uid);
		if (uid)
			*uid = in_uid;
		for (; *ptr; ptr++);
		ptr++;
		scan_uint(ptr, &in_gid);
		if (gid)
			*gid = in_gid;
		for (; *ptr; ptr++);
		ptr++;
		i = str_len(ptr) + 1;
		if (i > in_dir_size && in_dir_size)
			alloc_free(in_dir);
		if (i > in_dir_size && !(in_dir = alloc(i))) {
			close(fd);
			alloc_free(tmpbuf);
			if (uid)
				*uid = -1;
			if (gid)
				*gid = -1;
			if (dir)
				dir->len = 0;
			in_uid = in_gid = -1;
			if (in_domain)
				alloc_free(in_domain);
			if (in_dir)
				alloc_free(in_dir);
			in_domain = in_dir = (char *) 0;
			o_alloc = in_domain_size = in_dir_size = 0;
			return ((char *) 0);
		}
		if (i > in_dir_size)
			in_dir_size = i;
		if (dir) {
			if (!stralloc_copyb(dir, ptr, i - 1) || !stralloc_0(dir))
				die_nomem();
			dir->len--;
		}
		s = in_dir;
		s += fmt_strn(s, ptr, i - 1);
		*s++ = 0;
		alloc_free(tmpbuf);
		close(fd);
		return (in_dir);
	}
	close(fd);
	alloc_free(tmpstr);
	if (uid)
		*uid = -1;
	if (gid)
		*gid = -1;
	if (dir)
		dir->len = 0;
	return ((char *) 0);
}

#ifdef QUERY_CACHE
void
get_assign_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
