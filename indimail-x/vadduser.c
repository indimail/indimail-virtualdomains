/*
 * $Log: vadduser.c,v $
 * Revision 1.10  2022-12-07 16:50:04+05:30  Cprogrammer
 * fixed incorrect order of arguments in get_options
 *
 * Revision 1.9  2022-11-02 20:03:48+05:30  Cprogrammer
 * added feature to add scram password during user addition
 *
 * Revision 1.8  2022-10-20 11:58:27+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.7  2022-08-07 13:04:37+05:30  Cprogrammer
 * removed apop setting
 *
 * Revision 1.6  2022-08-05 21:18:08+05:30  Cprogrammer
 * added encrypt_flag argument to iadduser()
 *
 * Revision 1.5  2021-07-08 11:46:09+05:30  Cprogrammer
 * add check for misconfigured assign file
 *
 * Revision 1.4  2020-06-16 17:56:08+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.3  2020-04-01 18:58:26+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 15:55:38+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-14 18:31:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_UNISTD_H
#define XOPEN_SOURCE = 600
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <env.h>
#include <strerr.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <open.h>
#include <error.h>
#include <getln.h>
#include <substdio.h>
#include <sgetopt.h>
#include <makesalt.h>
#include <setuserid.h>
#include <makesalt.h>
#include <hashmethods.h>
#include <subfd.h>
#endif
#ifdef HAVE_GSASL_H
#include <gsasl.h>
#include "gsasl_mkpasswd.h"
#endif
#include "iopen.h"
#include "iclose.h"
#include "iadduser.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "parse_quota.h"
#include "SqlServer.h"
#include "vlimits.h"
#include "parse_email.h"
#include "vgetpasswd.h"
#include "check_group.h"
#include "sql_getip.h"
#include "variables.h"
#include "post_handle.h"
#include "common.h"

#ifndef	lint
static char     rcsid[] = "$Id: vadduser.c,v 1.10 2022-12-07 16:50:04+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vadduser: fatal: "
#define WARN    "vadduser: warning: "

stralloc        Email = {0}, User = {0}, Domain = {0}, Passwd = {0},
                Quota = {0}, Gecos = {0}, envbuf = {0};
#ifdef CLUSTERED_SITE
stralloc        mdahost = {0}, hostid = {0};
#endif
int             balance_flag, actFlag = 1;

extern int      create_flag;

int             get_options(int, char **, char **, int *, int *, int *, int *, int *, int *, char **);
int             checklicense(char *, int, long, char *, int);

static char    *usage =
	"usage: vadduser [options] email_address [password]\n"
	"options\n"
	"  -V              - (print version number)\n"
	"  -v              - (verbose)\n"
	"  -q              - quota (in bytes) (sets the users quota)\n"
	"  -l level        - users per level\n"
	"  -c              - comment (sets the gecos comment field)\n"
	"  -b              - Balance distribution across filesystems\n"
	"  -B basepath     - Specify the base directory for user's home directory\n"
	"  -d              - Create the homedir (ignored if -h option is given)\n"
	"  -r len          - generate a random password of length=len\n"
	"  -e password     - set the encrypted password field\n"
	"  -h hash         - use one of DES, MD5, SHA256, SHA512, hash method\n"
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	"  -C              - store clear txt and scram hex salted password in database\n"
	"                    This allows CRAM methods to be used\n"
	"  -m SCRAM method - use one of SCRAM-SHA-1, SCRAM-SHA-256 SCRAM method\n"
	"  -S salt         - use a fixed base64 encoded salt for generating SCRAM password\n"
	"                  - if salt is not specified, it will be generated\n"
	"  -I iter_count   - use iter_count instead of 4096 for generating SCRAM password\n"
#endif
#endif
#ifdef CLUSTERED_SITE
	"  -M mdahost      - (host on which the account needs to be created - specify mdahost)\n"
	"  -H hostid       - (host on which the account needs to be created - specify hostid)\n"
#endif
	"  -i              - (sets the account as inactive)"
	;

static void
die_nomem()
{
	strerr_warn1("vadduser: out of memory", 0);
	_exit(111);
}

int
main(int argc, char **argv)
{
	int             i, j, random, users_per_level = 0, fd, match,
					encrypt_flag;
	mdir_t          q, c;
	char           *real_domain, *ptr, *base_argv0, *base_path, *domain_dir;
	static stralloc tmpbuf = {0}, quotaVal = {0}, line = {0};
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG], inbuf[512];
	uid_t           uid, uidtmp;
	gid_t           gid, gidtmp;
#ifdef ENABLE_DOMAIN_LIMITS
	int             domain_limits;
	struct vlimits  limits;
#endif
	struct substdio ssin;
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	int             scram, iter, docram;
	static stralloc result = {0};
	char           *b64salt;
#endif
#endif

#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	if (get_options(argc, argv, &base_path, &users_per_level,
				&encrypt_flag, &random, &docram, &scram, &iter, &b64salt))
		return (1);
#else
	if (get_options(argc, argv, &base_path, &users_per_level,
				&encrypt_flag, &random, 0, 0, 0, 0))
		return (1);
#endif
#else
	if (get_options(argc, argv, &base_path, &users_per_level,
				&encrypt_flag, &random, 0, 0, 0, 0))
		return (1);
#endif
	/*- parse the email address into user and domain */
	parse_email(Email.s, &User, &Domain);
	/* Do this so that users do not get added in a alias domain */
	real_domain = (char *) 0;
	if (Domain.len && !(real_domain = get_real_domain(Domain.s))) {
		strerr_warn2(Domain.s, ": No such domain", 0);
		return (1);
	}
	if (!(domain_dir = get_assign(real_domain, 0, &uid, &gid))) {
		strerr_warn2(real_domain, ": domain does not exist", 0);
		return (1);
	}
	if (!uid)
		strerr_die3x(100, "vadduser: domain ", real_domain, " with uid 0");
#ifdef ENABLE_DOMAIN_LIMITS
	if (!stralloc_copys(&tmpbuf, domain_dir) ||
			!stralloc_catb(&tmpbuf, ".domain_limits", 14) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	domain_limits = ((access(tmpbuf.s, F_OK) && !env_get("DOMAIN_LIMITS")) ? 0 : 1);
	if (domain_limits && vget_limits(real_domain, &limits)) {
		strerr_warn2("vadduser: Unable to get domain limits for ", real_domain, 0);
		return (1);
	}
	if (Quota.len && (limits.perm_defaultquota & VLIMIT_DISABLE_CREATE)) {
		strerr_warn2("vadduser: -q option not allowed for ", real_domain, 0);
		return (1);
	}
#endif
	/*- if the comment field is blank use the user name */
	if (Gecos.len == 0) {
		if (!stralloc_copy(&Gecos, &User) ||
				!stralloc_0(&Gecos))
			die_nomem();
	}
	uidtmp = getuid();
	gidtmp = getgid();
	if (uidtmp != 0 && uidtmp != uid && gidtmp != gid && check_group(gid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vadduser: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	if (setuser_privileges(uid, gid, "indimail")) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vadduser: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
		return (1);
	}
	/* set the users quota if set on the command line */
	if (Quota.len) {
		if (str_diffn(Quota.s, "NOQUOTA", 8)) {
			q = parse_quota(Quota.s, &c);
			strnum1[i = fmt_ulong(strnum1, q)] = 0;
			strnum2[j = fmt_ulong(strnum1, c)] = 0;
			if (!stralloc_copyb(&quotaVal, strnum1, i) ||
					!stralloc_append(&quotaVal, ",") ||
					!stralloc_catb(&quotaVal, strnum2, j) ||
					!stralloc_0(&quotaVal))
				die_nomem();
		}
	} else
#ifdef ENABLE_DOMAIN_LIMITS
	if (domain_limits) {
		if (limits.defaultquota) {
			strnum1[i = fmt_ulong(strnum1, limits.defaultquota)] = 0;
			if (!stralloc_copyb(&quotaVal, strnum1, i))
				die_nomem();
			if (limits.defaultmaxmsgcount) {
				strnum2[j = fmt_ulong(strnum2, limits.defaultmaxmsgcount)] = 0;
				if (!stralloc_append(&quotaVal, ",") || !stralloc_catb(&quotaVal, strnum2, j))
					die_nomem();
			}
			if (!stralloc_0(&quotaVal))
				die_nomem();
		} else
			quotaVal.len = 0;
	} else
		quotaVal.len = 0;
#else
		quotaVal.len = 0;
#endif
#ifdef CLUSTERED_SITE
	if (!mdahost.len && hostid.len) {
		if (!(ptr = sql_getip(hostid.s))) {
			strerr_warn4("Failed to obtain mdahost for host ", hostid.s, " domain ", real_domain, 0);
			return (1);
		} else
		if (!stralloc_copys(&mdahost, ptr) || !stralloc_0(&mdahost))
			die_nomem();
	}
	/* add the user */
	if (mdahost.len) {
		if (!(ptr = SqlServer(mdahost.s, real_domain))) {
			strerr_warn4("Failed to obtain sqlserver for mdahost ", mdahost.s, " domain ", real_domain, 0);
			return (1);
		} else
		if (iopen(ptr)) {
			strerr_warn2("Failed to connect to ", ptr, 0);
			return (1);
		}
		if (verbose) {
			subprintfe(subfdout, "vadduser", "Adding to MDAhost %s SqlServer %s\n", mdahost.s, ptr);
			flush("vadduser");
		}
	} 
#endif
	envbuf.len = 0;
	if (base_path) {
		if (!stralloc_copys(&envbuf, base_path) ||
				!stralloc_0(&envbuf))
			die_nomem();
	} else {
		if (!stralloc_copys(&tmpbuf, domain_dir) ||
				!stralloc_catb(&tmpbuf, "/.base_path", 11) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if ((fd = open_read(tmpbuf.s)) == -1) {
			if (errno != error_noent) {
				strerr_warn3("vadduser: open: ", tmpbuf.s, ": ", &strerr_sys);
				return (-1);
			}
		} else {
			substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("vadduser: read: ", tmpbuf.s, ": ", &strerr_sys);
				close(fd);
				return (-1);
			}
			if (line.len == 0 || !match)
				strerr_warn3("vadduser", tmpbuf.s, "incomplete line", 0);
			else
			if (match)
				line.len--;
			close(fd);
			if (!stralloc_copy(&envbuf, &line) || !stralloc_0(&envbuf))
				die_nomem();
			}
	}
	if (balance_flag) {
		if ((fd = open_read(SYSCONFDIR"/lastfstab")) == -1) {
			strerr_warn3("vadduser: ", SYSCONFDIR"/lastfstab", ": ", &strerr_sys);
			return (1);
		}
		substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vadduser: read: ", SYSCONFDIR"/lastfstab", ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (line.len == 0)
			strerr_warn3("vadduser", SYSCONFDIR"/lastfstab", "incomplete line", 0);
		else
		if (match)
			line.len--;
		close(fd);
		if (!stralloc_copy(&envbuf, &line) || !stralloc_0(&envbuf))
			die_nomem();
	}
	if (envbuf.len && !env_put2("BASE_PATH", envbuf.s))
		die_nomem();
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	switch (scram)
	{
	case 1: /*- SCRAM-SHA-1 */
		if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-1", iter, b64salt, docram, Passwd.s, &result)) != NO_ERR)
			strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
		break;
	case 2: /*- SCRAM-SHA-256 */
		if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-256", iter, b64salt, docram, Passwd.s, &result)) != NO_ERR)
			strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
		break;
	/*- more cases will get below when devils come up with a new RFC */
	}
	ptr = scram ? result.s : 0;
#else
	ptr = 0;
#endif
#else
	ptr = 0;
#endif
#ifdef CLUSTERED_SITE
	if ((i = iadduser(User.s, real_domain, mdahost.s, Passwd.s, Gecos.s,
		quotaVal.s, users_per_level, actFlag, encrypt_flag, ptr)) < 0)
#else
	if ((i = iadduser(User.s, real_domain, 0, Passwd.s, Gecos.s,
		quotaVal.s, users_per_level, actFlag, encrypt_flag, ptr)) < 0)
#endif
	{
		if (errno == EEXIST)
			i = errno;
		iclose();
		return (i);
	}
	iclose();
	if (random) {
		subprintfe(subfdout, "vadduser", "Password is %s\n", Passwd.s);
		flush("vadduser");
	}
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s@%s", LIBEXECDIR, base_argv0, User.s, real_domain));
	} else
		return (post_handle("%s %s@%s", ptr, User.s, real_domain));
}

int
get_options(int argc, char **argv, char **base_path, int *users_per_level,
		int *encrypt_flag, int *random, int *docram, int *scram, int *iter,
		char **salt)
{
	int             c, i;
	char            strnum[FMT_ULONG], optstr[30];

	Email.len = Passwd.len = Domain.len = Quota.len = 0;
	actFlag = 1;
	*base_path = 0;
	*random = 0;
	*encrypt_flag = -1;
	if (salt)
		*salt = 0;
	if (iter)
		*iter = 4096;
	if (scram)
		*scram = 0;
	if (docram)
		*docram = 0;
	/*- make sure optstr has enough size to hold all options + 1 */
	i = 0;
	i += fmt_strn(optstr + i, "aidbB:vc:q:l:er:h:", 18);
#ifdef CLUSTERED_SITE
	i += fmt_strn(optstr + i, "H:M:", 4);
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	i += fmt_strn(optstr + i, "Cm:S:I:", 7);
#endif
#endif
	if ((i + 1) > sizeof(optstr))
		strerr_die2x(100, FATAL, "allocated space for getopt string not enough");
	optstr[i] = 0;

	while ((c = getopt(argc, argv, optstr)) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'r':
			scan_uint(optarg, (unsigned int *) random);
			break;
		case 'i':
			actFlag = 0;
			break;
		case 'c':
			if (!stralloc_copys(&Gecos, optarg) || !stralloc_0(&Gecos))
				die_nomem();
			break;
		case 'h':
			if (!str_diffn(optarg, "DES", 3))
				strnum[fmt_int(strnum, DES_HASH)] = 0;
			else
			if (!str_diffn(optarg, "MD5", 3))
				strnum[fmt_int(strnum, MD5_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-256", 7))
				strnum[fmt_int(strnum, SHA256_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-512", 7))
				strnum[fmt_int(strnum, SHA512_HASH)] = 0;
			else
				strerr_die5x(100, FATAL, "wrong hash method ", optarg, ". Supported HASH Methods: DES MD5 SHA-256 SHA-512\n", usage);
			if (!env_put2("PASSWORD_HASH", strnum))
				strerr_die1x(111, "out of memory");
			*encrypt_flag = 1;
			break;
		case 'e':
			/*- ignore encrypt flag option if -h option is provided */
			if (*encrypt_flag == -1)
				*encrypt_flag = 0;
			break;
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
		case 'C':
			if (docram)
				*docram = 1;
			break;
		case 'm':
			if (!scram)
				break;
			if (!str_diffn(optarg, "SCRAM-SHA-1", 11))
				*scram = 1;
			else
			if (!str_diffn(optarg, "SCRAM-SHA-256", 13))
				*scram = 2;
			else
				strerr_die5x(100, FATAL, "wrong SCRAM method ", optarg, ". Supported SCRAM Methods: SCRAM-SHA1 SCRAM-SHA-256\n", usage);
			break;
		case 'S':
			if (!salt)
				break;
			i = str_chr(optarg, ',');
			if (optarg[i])
				strerr_die3x(100, WARN, optarg, ": salt cannot have a comma character");
			*salt = optarg;
			break;
		case 'I':
			if (!iter)
				break;
			scan_int(optarg, iter);
			break;
#endif
#endif
		case 'd':
#ifdef CLUSTERED_SITE
			if (!mdahost.len && !hostid.len)
				create_flag = 1;
#else
			create_flag = 1;
#endif
			break;
		case 'b':
			balance_flag = 1;
			break;
		case 'B':
			*base_path = optarg;
			break;
		case 'q':
			if (!stralloc_copys(&Quota, optarg) || !stralloc_0(&Quota))
				die_nomem();
			break;
		case 'l':
			scan_uint(optarg, (unsigned int *) users_per_level);
			break;
#ifdef CLUSTERED_SITE
		case 'M':
			if (!stralloc_copys(&mdahost, optarg) || !stralloc_0(&mdahost))
				die_nomem();
			create_flag = 0;
			break;
		case 'H':
			if (!stralloc_copys(&hostid, optarg) || !stralloc_0(&hostid))
				die_nomem();
			create_flag = 0;
			break;
#endif
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}

	if (optind < argc) {
		if (!stralloc_copys(&Email, argv[optind++]) || !stralloc_0(&Email))
			die_nomem();
		Email.len--;
	}
	if (optind < argc) {
		if (!stralloc_copys(&Passwd, argv[optind++]) || !stralloc_0(&Passwd))
			die_nomem();
		Passwd.len--;
	}
	/*- get the password if not set on command line */
	if (random && !Passwd.len) {
		if (!stralloc_copys(&Passwd, genpass(*random)) ||
				!stralloc_0(&Passwd))
			die_nomem();
	} else
	if (!Passwd.len) {
		if (!stralloc_copys(&Passwd, vgetpasswd(Email.s)) ||
				!stralloc_0(&Passwd))
			die_nomem();
	}
	if (!Passwd.len) {
		strerr_warn3(WARN, "Please input password\n", usage, 0);
		return (1);
	}
	if (!Email.len) {
		strerr_warn3(WARN, "Please input email\n", usage, 0);
		return (1);
	}
	if (*encrypt_flag == -1)
		*encrypt_flag = 1;
	return (0);
}
