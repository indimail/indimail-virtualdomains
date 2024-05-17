/*
 * $Id: vadddomain.c,v 1.19 2023-12-03 15:43:17+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <substdio.h>
#include <subfd.h>
#include <stralloc.h>
#include <sgetopt.h>
#include <error.h>
#include <strerr.h>
#include <scan.h>
#include <fmt.h>
#include <env.h>
#include <str.h>
#include <check_domain.h>
#include <getEnvConfig.h>
#include <makesalt.h>
#include <hashmethods.h>
#include <subfd.h>
#include <setuserid.h>
#endif
#ifdef HAVE_GSASL_H
#include <gsasl.h>
#include "gsasl_mkpasswd.h"
#endif
#include "variables.h"
#include "valias_insert.h"
#include "isvalid_domain.h"
#include "vgetpasswd.h"
#include "check_group.h"
#include "get_assign.h"
#include "r_mkdir.h"
#include "iclose.h"
#include "post_handle.h"
#include "is_user_present.h"
#include "get_local_ip.h"
#include "get_local_hostid.h"
#include "get_local_ip.h"
#include "is_distributed_domain.h"
#include "LoadDbInfo.h"
#include "vgetpasswd.h"
#include "SqlServer.h"
#include "iadddomain.h"
#include "deldomain.h"
#include "iadduser.h"
#include "setuserquota.h"
#include "vhostid_select.h"
#include "vhostid_insert.h"
#include "dbinfoAdd.h"
#include "get_indimailuidgid.h"
#include "common.h"
#include "get_hashmethod.h"

#ifndef	lint
static char     rcsid[] = "$Id: vadddomain.c,v 1.19 2023-12-03 15:43:17+05:30 Cprogrammer Exp mbhangui $";
#endif

#define WARN    "vadddomain: warning: "
#define FATAL   "vadddomain: fatal: "

#ifdef CLUSTERED_SITE
int             dbport, use_ssl = 1, distributed;
#endif
static uid_t    Uid;
static gid_t    Gid;
static stralloc dirbuf = { 0 };
static stralloc AliasLine = { 0 };
static stralloc tmp1 = { 0 }, tmp2 = { 0 };
static int      chk_rcpt, users_per_level = 0;
static char     strnum1[FMT_ULONG], strnum2[FMT_ULONG];

static char    *usage =
	"usage: vaddomain [options] virtual_domain [postmaster password]\n"
	"options\n"
	"  -V                         - print version number\n"
	"  -v                         - verbose\n"
	"  -q quota_in_bytes          - sets the quota for postmaster account\n"
	"  -l level                   - users per level\n"
	"  -R                         - Do recipient check for this domain\n"
	"  -E [email_address|maildir] - forwards all non matching user to this address [*]\n"
	"  -u user                    - sets the uid/gid based on a user in /etc/passwd\n"
	"  -B basepath                - specify the base directory for postmaster's home directory\n"
	"  -d dir                     - sets the dir to use for this domain\n"
	"  -i uid                     - sets the uid to use for this domain\n"
	"  -g gid                     - sets the gid to use for this domain\n"
	"  -f                         - sets the Domain with VFILTER capability\n"
	"  -t                         - sets the Domain for ETRN/ATRN\n"
	"  -T ipaddr                  - sets the Domain for ETRN/AUTOTURN from IP ipaddr\n"
	"  -O                         - optimize adding, for bulk adds set this for all\n"
	"                               except the last one\n"
	"  -r len                     - generate a random password of length=len\n"
	"  -e password                - set the encrypted password field\n"
	"  -h hash                    - use one of DES, MD5, SHA256, SHA512, YESCRYPT, hash method\n"
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	"  -m SCRAM method            - use one of SCRAM-SHA-1, SCRAM-SHA-256 SCRAM method\n"
	"  -C                         - store clear txt and SCRAM hex salted password in database\n"
	"                               This allows CRAM methods to be used\n"
	"  -S salt                    - use a fixed base64 encoded salt for generating SCRAM password\n"
	"                             - if salt is not specified, it will be generated\n"
	"  -I iter_count              - use iter_count instead of 4096 for generating SCRAM password\n"
#else
	"  -C                         - store clear txt password in database\n"
#endif
#else
	"  -C                         - store clear txt password in database\n"
#endif
#ifdef CLUSTERED_SITE
	"  -D database                - database name. adds a clustered domain, extra\n"
	"                               options apply as below\n"
	"  -H host                    - sqlServer host/IP\n"
	"  -U user                    - database User\n"
	"  -P pass                    - database Password\n"
	"  -p port                    - database Port\n"
	"  -c                         - add domain as a clustered domain"
#endif
	;

static void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
get_options(int argc, char **argv, char **base_path, char **dir_t, char **passwd,
	char **domain, char **user, char **quota, char **bounceEmail, char **ipaddr,
	char **database, char **sqlserver, char **dbuser, char **dbpass,
	int *encrypt_flag, int *random, int *docram, int *scram, int *iter,
	char **salt, char **hash)
{
	int             c, i, r;
	struct passwd  *mypw;
	char            optstr[51];
	char           *ptr;

	Uid = indimailuid;
	Gid = indimailgid;
	*encrypt_flag = -1;
	*random = 0;
	if (docram)
		*docram = 0;
	if (scram)
		*scram = 0;
	if (iter)
		*iter = 4096;
	if (salt)
		*salt = 0;
	/*- make sure optstr has enough size to hold all options + 1 */
	i = 0;
	i += fmt_strn(optstr + i, "ateh:T:q:l:bB:E:u:vRi:g:d:Or:C", 30);
#ifdef CLUSTERED_SITE
	i += fmt_strn(optstr + i, "D:H:U:P:s:p:c", 13);
#endif
#ifdef VFILTER
	i += fmt_strn(optstr + i, "f", 1);
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	i += fmt_strn(optstr + i, "m:S:I:", 6);
#endif
#endif
	if ((i + 1) > sizeof(optstr))
		strerr_die2x(100, FATAL, "allocated space for getopt string not enough");
	optstr[i] = 0;
	while ((c = getopt(argc, argv, optstr)) != opteof) {
		switch (c)
		{
		case 'B':
			if (use_etrn)
				strerr_die3x(100, WARN, "option -B not valid for ETRN/ATRN/AUTORUN\n", usage);
			else
				*base_path = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'd':
			*dir_t = optarg;
			break;
		case 'R':
			chk_rcpt = 1;
			break;
		case 'r':
			scan_uint(optarg, (unsigned int *) random);
			break;
		case 'h':
			if ((r = scan_int(optarg, &i)) != str_len(optarg))
				i = -1;
			if (!str_diffn(optarg, "DES", 3) || i == DES_HASH)
				strnum1[fmt_int(strnum1, DES_HASH)] = 0;
			else
			if (!str_diffn(optarg, "MD5", 3) || i == MD5_HASH)
				strnum1[fmt_int(strnum1, MD5_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-256", 7) || i == SHA256_HASH)
				strnum1[fmt_int(strnum1, SHA256_HASH)] = 0;
			else
			if (!str_diffn(optarg, "SHA-512", 7) || i == SHA512_HASH)
				strnum1[fmt_int(strnum1, SHA512_HASH)] = 0;
			else
			if (!str_diffn(optarg, "YESCRYPT", 8) || i == YESCRYPT_HASH)
				strnum1[fmt_int(strnum1, YESCRYPT_HASH)] = 0;
			else {
				strerr_die5x(100, FATAL, "wrong hash method ", optarg,
						". Supported HASH Methods: DES MD5 SHA-256 SHA-512 YESCRYPT\n", usage);
			}
			if (!env_put2("PASSWORD_HASH", strnum1))
				strerr_die1x(111, "out of memory");
			*hash = optarg;
			*encrypt_flag = 1;
			break;
		case 'e':
			/*- ignore encrypt flag option if -h option is provided */
			if (*encrypt_flag == -1)
				*encrypt_flag = 0;
			break;
		case 'C':
			if (docram)
				*docram = 1;
			break;
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
		case 'm':
			if (!scram)
				break;
			if (!str_diffn(optarg, "CRAM", 5))
				*scram = 0;
			else
			if (!str_diffn(optarg, "SCRAM-SHA-1", 11))
				*scram = 1;
			else
			if (!str_diffn(optarg, "SCRAM-SHA-256", 13))
				*scram = 2;
			else
				strerr_die5x(100, WARN, "wrong SCRAM method ", optarg,
						". Supported CRAM Methods: CRAM, SCRAM-SHA1 SCRAM-SHA-256\n", usage);
			break;
		case 'S':
			if (!salt)
				break;
			i = str_chr(optarg, ',');
			if (optarg[i]) {
				strerr_die3x(100, WARN, optarg, ": salt cannot have a comma character");
			}
			*salt = optarg;
			break;
		case 'I':
			if (!iter)
				break;
			scan_int(optarg, iter);
			break;
#endif
#endif
#ifdef CLUSTERED_SITE
		case 'c':
			distributed = 1;
			break;
		case 'D':
			if (distributed == -1)
				distributed = 0;
			*database = optarg;
			break;
		case 's':
			scan_int(optarg, &use_ssl);
			break;
		case 'H':
			*sqlserver = optarg;
			break;
		case 'U':
			*dbuser = optarg;
			break;
		case 'P':
			*dbpass = optarg;
			break;
		case 'p':
			scan_int(optarg, &dbport);
			break;
#endif
		case 'u':
			*user = optarg;
			if (*user) {
				if ((mypw = getpwnam(*user)) != NULL) {
					if (!stralloc_copys(&dirbuf, mypw->pw_dir))
						die_nomem();
					else
					if (!stralloc_0(&dirbuf))
						die_nomem();
					Uid = mypw->pw_uid;
					Gid = mypw->pw_gid;
					if (!Uid)
						strerr_die3x(100, "user ", *user, " has uid=0");
				} else
					strerr_die3x(100, "user ", *user, " not found in /etc/passwd");
			}
			break;
		case 'q':
			*quota = optarg;
			break;
		case 'l':
			scan_int(optarg, &users_per_level);
			break;
		case 'E':
			*bounceEmail = optarg;
			break;
		case 'i':
			scan_uint(optarg, (unsigned int *) &Uid);
			break;
		case 'g':
			scan_uint(optarg, (unsigned int *) &Gid);
			break;
#ifdef VFILTER
		case 'f':
			use_vfilter = 1;
			break;
#endif
		case 't': /*- ETRN/ATRN Support */
			use_etrn = 1;
			break;
		case 'T': /*- AUTOTURN Support */
			*ipaddr = optarg;
			use_etrn = 2;
			break;
		case 'O':
			OptimizeAddDomain = 1;
			break;
		default:
			strerr_die2x(100, WARN, usage);
		}
	}
	if (optind < argc) {
		for (ptr = argv[optind]; *ptr; ptr++) {
			if (isupper(*ptr))
				strerr_die4x(100, WARN, "domain [", argv[optind], "] has an uppercase character");
		}
		*domain = argv[optind++];
	}
	if (optind < argc)
		*passwd = argv[optind++];
	if (!*domain)
		strerr_die3x(100, WARN, "Domain not specified\n", usage);
	if (!check_domain(*domain))
		strerr_die3(111, WARN, "invalid domain: ", *domain, &check_domain_err);
#ifdef CLUSTERED_SITE
	if (distributed >=0 && (!*sqlserver || !*database || !*dbuser || !*dbpass || dbport == -1))
		strerr_die3x(100, WARN, "specify sqlserver, database, dbuser, dbpass and dbport\n", usage);
#endif
	if (*encrypt_flag == -1)
		*encrypt_flag = 1;
	return;
}

void
write_file(char *domain, char *fn, int fnlen, char *str, int len)
{
	int             fd;
	uid_t           uid;
	gid_t           gid;
	char           *s;
	char            dbuf[FMT_ULONG + FMT_ULONG + 21];

	if (!get_assign(domain, &dirbuf, &uid, &gid)) {
		strerr_warn4(WARN, "domain ", domain, " does not exist", 0);
		deldomain(domain);
		iclose();
		_exit(100);
	}
	if (!stralloc_copy(&tmp1, &dirbuf) ||
			!stralloc_append(&tmp1, "/") ||
			!stralloc_catb(&tmp1, fn, fnlen) ||
			!stralloc_0(&tmp1)) {
		deldomain(domain);
		iclose();
		die_nomem();
	}
	if ((fd = open(tmp1.s, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) == -1) {
		strerr_warn4(FATAL, "open: ", tmp1.s, ": ", &strerr_sys);
		deldomain(domain);
		iclose();
		_exit(111);
	}
	if (write(fd, str, len) == -1) {
		strerr_warn4(FATAL, "write: ", tmp1.s, ": ", &strerr_sys);
		deldomain(domain);
		iclose();
		_exit(111);
	}
	if (fchown(fd, uid, gid)) {
		s = dbuf;
		s += fmt_strn(s, ": (uid = ", 9);
		s += fmt_uint(s, uid);
		s += fmt_strn(s, ", gid = ", 8);
		s += fmt_uint(s, gid);
		s += fmt_strn(s, "): ", 3);
		*s++ = 0;
		strerr_warn4(FATAL, "fchown: ", tmp1.s, dbuf, &strerr_sys);
		deldomain(domain);
		iclose();
		_exit(111);
	}
	close(fd);
	return;
}

int
do_posthandle(int argc, char **argv)
{
	char           *ptr, *base_argv0;
	int             i;

	for (i = 1, tmp1.len = 0; i < argc; i++) {
		if (!stralloc_append(&tmp1, " ") ||
				!stralloc_cats(&tmp1, argv[i]))
			die_nomem();
	}
	if (!stralloc_0(&tmp1))
		die_nomem();
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s%s", LIBEXECDIR, base_argv0, tmp1.s));
	} else {
		if (setuser_privileges(Uid, Gid, "indimail")) {
			strnum1[fmt_ulong(strnum1, Uid)] = 0;
			strnum2[fmt_ulong(strnum2, Gid)] = 0;
			strerr_die5sys(111, "vadddomain: setuser_privilege: (", strnum1, "/", strnum2, "): ");
		}
		return (post_handle("%s%s", ptr, tmp1.s));
	}
}

int
main(int argc, char **argv)
{
	int             err, i, encrypt_flag, random, docram;
	uid_t           uid;
	gid_t           gid;
	extern int      create_flag;
	char           *domaindir, *ptr, *base_path, *dir_t, *passwd, *domain, *user,
				   *bounceEmail, *quota, *ipaddr, *s, *hash;
	char            dbuf[FMT_ULONG + FMT_ULONG + 21];
	char           *auto_ids[] = {
		"abuse",
		"mailer-daemon",
		0
	};
	static stralloc result = {0};
#ifdef CLUSTERED_SITE
	char           *database, *sqlserver, *dbuser, *dbpass;
	char           *hostid, *localIP;
	int             is_dist, user_present, total;
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	int             scram, iter;
	char           *b64salt;
#endif
#endif

	base_path = dir_t = passwd = domain = user = quota = bounceEmail = ipaddr = hash = (char *) 0;
#ifdef CLUSTERED_SITE
	sqlserver = database = dbuser = dbpass = (char *) 0;
	use_ssl = 1;
	dbport = -1;
	distributed = -1;
#endif
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	get_options(argc, argv, &base_path, &dir_t, &passwd, &domain, &user,
		&quota, &bounceEmail, &ipaddr, &database, &sqlserver, &dbuser, &dbpass,
		&encrypt_flag, &random, &docram, &scram, &iter, &b64salt, &hash);
#else
	get_options(argc, argv, &base_path, &dir_t, &passwd, &domain, &user,
		&quota, &bounceEmail, &ipaddr, &database, &sqlserver, &dbuser, &dbpass,
		&encrypt_flag, &random, &docram, 0, 0, 0, &hash);
#endif
#else
	get_options(argc, argv, &base_path, &dir_t, &passwd, &domain, &user,
		&quota, &bounceEmail, &ipaddr, &database, &sqlserver, &dbuser, &dbpass,
		&encrypt_flag, &random, &docram, 0, 0, 0, &hash);
#endif
	if (!isvalid_domain(domain))
		strerr_die3x(100, WARN, "Invalid domain ", domain);
	if (!use_etrn && !passwd) {
		if (random)
			passwd = genpass(random);
		else
			passwd = vgetpasswd("postmaster");
	}
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, indimailuid)] = 0;
		strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
		strerr_die6x(100, WARN, "you must be root or domain user (uid=", strnum1, ", gid=", strnum2, ") to run this program");
	}
	if (uid & setuid(0))
		strerr_die2sys(111, FATAL, "setuid: ");
	if (!dir_t) {
		getEnvConfigStr(&domaindir, "DOMAINDIR", DOMAINDIR);
		if (use_etrn) {
			if (!(ptr = get_assign("autoturn", 0, &uid, &gid))) {
				if (!stralloc_copys(&dirbuf, domaindir) ||
						!stralloc_catb(&dirbuf, "/autoturn", 9) ||
						!stralloc_0(&dirbuf))
					die_nomem();
			} else {
				Uid = uid; /*- autoturn uid */
				Gid = gid; /*- autoturn gid */
				if (!stralloc_copys(&dirbuf, ptr) || !stralloc_0(&dirbuf))
					die_nomem();
			}
		} else {
			if (!stralloc_copys(&dirbuf, domaindir) || !stralloc_0(&dirbuf))
				die_nomem();
		}
	} else
	if (!stralloc_copys(&dirbuf, dir_t) || !stralloc_0(&dirbuf))
		die_nomem();
	if (base_path && !use_etrn) {
		if (access(base_path, F_OK) && r_mkdir(base_path, INDIMAIL_DIR_MODE, Uid, Gid))
			strerr_die3sys(111, FATAL, base_path, ": ");
		if (!env_put2("BASE_PATH", base_path))
			strerr_die4sys(111, FATAL, "env_put2: BASE_PATH=", base_path, ": ");
	}
	/*
	 * add domain to virtualdomains and optionally to chkrcptdomains
	 * add domain to users/assign, users/cdb
	 * create .qmail-default file
	 */
	if ((err = iadddomain(domain, ipaddr, dirbuf.s, Uid, Gid, chk_rcpt)) != VA_SUCCESS) {
		iclose();
		return (err);
	}
	if (use_etrn)
		return (do_posthandle(argc, argv));
	if (users_per_level) {
		s = dbuf;
		s += (i = fmt_uint(s, (unsigned int) users_per_level));
		write_file(domain, ".users_per_level", 16, s, i);
	}
	if (hash) {
		if (!stralloc_copys(&tmp2, hash) ||
				!stralloc_append(&tmp2, "\n"))
			die_nomem();
		write_file(domain, "hash_method", 11, tmp2.s, tmp2.len);
	} else
	if (!env_get("PASSWORD_HASH")) {
		if ((i = get_hashmethod(domain)) == -1)
			strerr_die2sys(111, FATAL, "get_hashmethod: ");
		strnum1[fmt_int(strnum1, i)] = 0;
		if (!env_put2("PASSWORD_HASH", strnum1))
			strerr_die1x(111, "out of memory");
	}
	if (base_path && !use_etrn) {
		if (!stralloc_copys(&tmp2, base_path) ||
				!stralloc_append(&tmp2, "\n"))
			die_nomem();
		write_file(domain, ".base_path", 10, tmp2.s, tmp2.len);
	}
#ifdef CLUSTERED_SITE
	/*
	 * add domain to dbinfo
	 */
	if (distributed >= 0) {
		if (!(localIP = get_local_ip(PF_INET))) {
			strerr_warn2(FATAL, "failed to get local IP", 0);
			iclose();
			_exit(111);
		}
		if (!(ptr = vhostid_select())) {
			if (!(hostid = get_local_hostid())) {
				strerr_warn2(FATAL, "failed to get local hostid", 0);
				iclose();
				_exit(111);
			}
			if (vhostid_insert(hostid, localIP)) {
				strerr_warn2(FATAL, "failed to get insert hostid", 0);
				iclose();
				_exit(111);
			}
		}
		if (dbinfoAdd(domain, distributed, sqlserver, localIP, dbport, use_ssl, database, dbuser, dbpass)) {
			strerr_warn5(FATAL, "Failed to add ", distributed == 1 ? "Clustered" : "NonClustered", "domain", domain, 0);
			iclose();
			_exit(111);
		}
		LoadDbInfo_TXT(&total);
	}
#endif
	if (bounceEmail) {
		if (bounceEmail[i = str_chr(bounceEmail, '@')] || *bounceEmail == '/') {
			if (!stralloc_copyb(&tmp2, "| ", 2) || !stralloc_cats(&tmp2, PREFIX) ||
					!stralloc_catb(&tmp2, "/sbin/vdelivermail '' ", 22) ||
					!stralloc_cats(&tmp2, bounceEmail) ||
					!stralloc_append(&tmp2, "\n")) {
				deldomain(domain);
				iclose();
				die_nomem();
			}
			write_file(domain, ".qmail_default", 14, tmp2.s, tmp2.len);
		} else {
			deldomain(domain);
			iclose();
			strerr_die3x(100, WARN, "Invalid bounce email address ",  bounceEmail);
		}
	}
	create_flag = 1;
	/*- Add the postmaster user */
#ifdef CLUSTERED_SITE
	if ((is_dist = is_distributed_domain(domain)) == -1) {
		deldomain(domain);
		iclose();
		strerr_warn4(FATAL, "Unable to verify ", domain, " as a distributed domain", 0);
		_exit(100);
	} else
	if (is_dist) {
		if ((user_present = is_user_present("postmaster", domain)) == -1) {
			iclose();
			strerr_warn2(FATAL, "auth db error", 0);
			_exit(100);
		} else
		if (user_present) {
			iclose();
			return (0);
		}
	}
#endif
#ifdef HAVE_GSASL
#if GSASL_VERSION_MAJOR == 1 && GSASL_VERSION_MINOR > 8 || GSASL_VERSION_MAJOR > 1
	switch (scram)
	{
	case 1: /*- SCRAM-SHA-1 */
		if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-1", iter, b64salt, docram, passwd, &result)) != NO_ERR)
			strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
		break;
	case 2: /*- SCRAM-SHA-256 */
		if ((i = gsasl_mkpasswd(verbose, "SCRAM-SHA-256", iter, b64salt, docram, passwd, &result)) != NO_ERR)
			strerr_die2x(111, "gsasl error: ", gsasl_mkpasswd_err(i));
		break;
	/*- more cases will get below when devils come up with a new RFC */
	}
	ptr = scram > 0 ? result.s : 0;
#else
	ptr = 0;
#endif
#else
	ptr = 0;
#endif
	if (!ptr && docram) {
		if (!stralloc_copyb(&result, "{CRAM}", 6) ||
				!stralloc_cats(&result, passwd) ||
				!stralloc_0(&result))
			die_nomem();
		ptr = result.s;
	}
	if ((err = iadduser("postmaster", domain, 0, passwd, "Postmaster", 0, users_per_level,
		1, encrypt_flag, ptr)) != VA_SUCCESS) {
		if (errno != EEXIST) {
			deldomain(domain);
			iclose();
			_exit(111);
		}
	}
	if (random) {
		subprintfe(subfdout, "vadddomain", "Password is %s\n", passwd);
		flush("vadddomain");
	}
	/* set quota for postmaster */
	if (quota)
		setuserquota("postmaster", domain, quota);
	for (i = 0; auto_ids[i]; i++) {
		substdio_put(subfdout, "Adding alias ", 13);
		substdio_puts(subfdout, auto_ids[i]);
		substdio_put(subfdout, "@", 1);
		substdio_puts(subfdout, domain);
		substdio_put(subfdout, " --> postmaster@", 16);
		substdio_puts(subfdout, domain);
		substdio_put(subfdout, "\n", 1);
		substdio_flush(subfdout);
		if (!stralloc_copyb(&AliasLine, "&postmaster@", 12) ||
				!stralloc_cats(&AliasLine, domain) ||
				!stralloc_0(&AliasLine)) {
			deldomain(domain);
			iclose();
			die_nomem();
		}
		if (valias_insert(auto_ids[i], domain, AliasLine.s, 0)) {
			deldomain(domain);
			iclose();
			_exit (111);
		}
	}
	iclose();
	for (i = 1, tmp1.len = 0; i < argc; i++) {
		if (!stralloc_append(&tmp1, " ") ||
				!stralloc_cats(&tmp1, argv[i]))
			die_nomem();
	}
	if (!stralloc_0(&tmp1))
		die_nomem();
	return (do_posthandle(argc, argv));
}

/*
 * $Log: vadddomain.c,v $
 * Revision 1.19  2023-12-03 15:43:17+05:30  Cprogrammer
 * exit early for post_handle for ETRN, ATRN domains
 *
 * Revision 1.18  2023-11-26 19:53:52+05:30  Cprogrammer
 * do post handle when creating etrn, atrn domains
 *
 * Revision 1.17  2023-07-17 11:46:59+05:30  Cprogrammer
 * set hash method from hash_method control file in controldir, domaindir
 *
 * Revision 1.16  2023-07-16 22:41:02+05:30  Cprogrammer
 * added YESCRYPT hash
 *
 * Revision 1.15  2023-07-15 00:25:20+05:30  Cprogrammer
 * Set password for CRAM authentication with {CRAM} prefix
 *
 * Revision 1.14  2023-04-25 23:35:54+05:30  Cprogrammer
 * null terminate argument passed to post_handle
 *
 * Revision 1.13  2023-03-22 08:50:29+05:30  Cprogrammer
 * run POST_HANDLE program (if set) with indimail uid/gid
 *
 * Revision 1.12  2023-03-20 10:28:35+05:30  Cprogrammer
 * call post handle script with the same arguments passed to vadddomain
 *
 * Revision 1.11  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.10  2022-11-02 20:03:34+05:30  Cprogrammer
 * added feature to add scram password during user addition
 *
 * Revision 1.9  2022-08-07 13:04:26+05:30  Cprogrammer
 * removed apop setting
 *
 * Revision 1.8  2021-08-24 11:26:55+05:30  Cprogrammer
 * added check for domain name validity
 *
 * Revision 1.7  2020-09-17 14:48:23+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.6  2020-04-01 18:58:24+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.5  2019-06-07 15:55:52+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.4  2019-04-22 23:16:17+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.3  2019-04-14 12:48:29+05:30  Cprogrammer
 * added use_ssl parameter to dbinfoAdd()
 *
 * Revision 1.2  2019-04-10 10:10:27+05:30  Cprogrammer
 * fixed base_argv0
 * replaced getuidgid() with get_indimailuidgid()
 * replaced adddomain with iadddomain
 *
 * Revision 1.1  2019-03-05 17:00:52+05:30  Cprogrammer
 * Initial revision
 *
 */
