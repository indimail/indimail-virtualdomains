/*
 * $Log: check_getpw.c,v $
 * Revision 1.4  2020-09-21 18:48:24+05:30  Cprogrammer
 * FreeBSD port
 *
 * Revision 1.3  2019-06-13 21:22:20+05:30  Cprogrammer
 * removed getversion_check_getpw_c function
 *
 * Revision 1.2  2018-11-06 11:28:41+05:30  Cprogrammer
 * fixed warning message
 *
 * Revision 1.1  2010-05-23 14:13:29+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#ifndef	lint
static char     rcsid[] = "$Id: check_getpw.c,v 1.4 2020-09-21 18:48:24+05:30 Cprogrammer Exp mbhangui $";
#endif

int
main(int argc, char **argv)
{
	struct passwd *pw;
#ifdef HAVE_SHADOW_H
	struct spwd   *spw;
#endif
	int i;

	if (!getuid()) {
		fprintf(stderr, "you must not be root!!\n");
		return (1);
	}
	for (i = 1;i < argc;i++) {
		if (!(pw = getpwnam(argv[i]))) {
			fprintf(stderr, "%s: No such user\n", argv[i]);
			return(1);
		}
		printf("%s:", pw->pw_name);
#ifdef HAVE_SHADOW_H
		if (!(spw = getspnam(argv[i]))) {
			fprintf(stderr, "getspnam: %s: %s\n", argv[i], errno ? strerror(errno) : "not found in shadow");
			printf("%s:", pw->pw_passwd);
		} else
			printf("%s:", spw->sp_pwdp);
#else
		printf("%s:", pw->pw_passwd);
#endif
#if defined(__FreeBSD__)
		printf("%d:%d:%ld:%s:%s:%s:%s:%ld\n", pw->pw_uid, pw->pw_gid, pw->pw_change, pw->pw_class, 
			pw->pw_gecos, pw->pw_dir, pw->pw_shell, pw->pw_expire);
#else
		printf("%d:%d:%s:%s:%s\n", pw->pw_uid, pw->pw_gid, pw->pw_gecos, pw->pw_dir, pw->pw_shell);
#endif
	}
	return(0);
}
