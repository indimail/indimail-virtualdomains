/*
 * $Log: vadddomain.c,v $
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
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
#include <getEnvConfig.h>
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

#ifndef	lint
static char     rcsid[] = "$Id: vadddomain.c,v 1.6 2020-04-01 18:58:24+05:30 Cprogrammer Exp mbhangui $";
#endif

#define WARN    "vadddomain: warning: "
#define FATAL   "vadddomain: fatal: "

#ifdef CLUSTERED_SITE
int             dbport, use_ssl = 1, distributed;
#endif
int             Apop;
uid_t           Uid;
gid_t           Gid;
stralloc        dirbuf = { 0 };
stralloc        AliasLine = { 0 };
stralloc        tmpbuf = { 0 };
static int      chk_rcpt, users_per_level = 0;

static char    *usage =
	"usage: vaddomain [options] virtual_domain [postmaster password]\n"
	"options: -V print version number\n"
	"         -v verbose\n"
	"         -q quota_in_bytes (sets the quota for postmaster account)\n"
	"         -l level (users per level)\n"
	"         -C (Do recipient check for this domain)\n"
	"         -e [email_address|maildir] (forwards all non matching user to this address [*])\n"
	"         -u user (sets the uid/gid based on a user in /etc/passwd)\n"
	"         -B basepath Specify the base directory for postmaster's home directory\n"
	"         -d dir (sets the dir to use for this domain)\n"
	"         -i uid (sets the uid to use for this domain)\n"
	"         -g gid (sets the gid to use for this domain)\n"
	"         -a sets the account to use APOP, default is POP\n"
	"         -f Sets the Domain with VFILTER capability\n"
	"         -t Sets the Domain for ETRN/ATRN\n"
	"         -T ipaddr Sets the Domain for AUTOTURN from IP ipaddr\n"
	"         -O optimize adding, for bulk adds set this for all\n"
	"            except the last one"
#ifdef CLUSTERED_SITE
	"\n"
	"         -D database (adds a clustered domain, extra options apply as below)\n"
	"         -S SqlServer host/IP\n"
	"         -U Database User\n"
	"         -P Database Password\n"
	"         -p Database Port\n"
	"         -c Add domain as a clustered domain"
#endif
	;

void            get_options(int argc, char **argv, char **, char **, char **, char **,
					char **, char **, char **, char **, char **, char **, char **, char **);
static void
die_nomem(char *prefix)
{
	strerr_warn2(prefix, ": out of memory", 0);
	_exit (111);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             err, fd, i;
	uid_t           uid;
	gid_t           gid;
	extern int      create_flag;
	char           *domaindir, *ptr, *base_argv0, *base_path, *dir_t, *passwd, *domain, *user,
				   *bounceEmail, *quota, *ipaddr, *s;
	char            dbuf[FMT_ULONG + FMT_ULONG + 21];
	char           *auto_ids[] = {
		"abuse",
		"mailer-daemon",
		0
	};
#ifdef CLUSTERED_SITE
	char           *database, *sqlserver, *dbuser, *dbpass;
	char           *hostid, *localIP;
	int             is_dist, user_present, total;
#endif

	base_path = dir_t = passwd = domain = user = quota = bounceEmail = ipaddr = (char *) 0;
#ifdef CLUSTERED_SITE
	sqlserver = database = dbuser = dbpass = (char *) 0;
	use_ssl = 1;
	dbport = -1;
	distributed = -1;
#endif
	get_options(argc, argv, &base_path, &dir_t, &passwd, &domain, &user, &quota, 
		&bounceEmail, &ipaddr, &database, &sqlserver, &dbuser, &dbpass);
	if (!isvalid_domain(domain))
		strerr_die3x(100, WARN, "Invalid domain ", domain);
	if (!use_etrn && !passwd)
		passwd = vgetpasswd("postmaster");
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid, FATAL) != 1)
		strerr_die1x(100, "you must be root or indimail to run this program");
	if (uid & setuid(0))
		strerr_die2sys(111, FATAL, "setuid: ");
	if (!dir_t) {
		getEnvConfigStr(&domaindir, "DOMAINDIR", DOMAINDIR);
		if (use_etrn) {
			if (!(ptr = get_assign("autoturn", 0, &uid, &gid))) {
				if (!stralloc_copys(&dirbuf, domaindir) ||
						!stralloc_catb(&dirbuf, "/autoturn", 9) ||
						!stralloc_0(&dirbuf))
					die_nomem(FATAL);
			} else {
				Uid = uid; /*- autoturn uid */
				Gid = gid; /*- autoturn gid */
				if (!stralloc_copys(&dirbuf, ptr) || !stralloc_0(&dirbuf))
					die_nomem(FATAL);
			}
		} else {
			if (!stralloc_copys(&dirbuf, domaindir) || !stralloc_0(&dirbuf))
				die_nomem(FATAL);
		}
	}
	/*
	 * add domain to virtualdomains and optionally to chkrcptdomains
	 * add domain to users/assign, users/cdb
	 * create .qmail-default file
	 */
	if (base_path && !use_etrn) {
		if (access(base_path, F_OK) && r_mkdir(base_path, INDIMAIL_DIR_MODE, Uid, Gid))
			strerr_die3sys(111, FATAL, base_path, ": ");
		if (!env_put2("BASE_PATH", base_path))
			strerr_die4sys(111, FATAL, "env_put2: BASE_PATH=", base_path, ": ");
	}
	if ((err = iadddomain(domain, ipaddr, dirbuf.s, Uid, Gid, chk_rcpt)) != VA_SUCCESS) {
		iclose();
		return (err);
	}
	if (users_per_level) {
		if (!get_assign(domain, &dirbuf, &uid, &gid)) {
			strerr_warn4(FATAL, "domain ", domain, " does not exist", 0);
			deldomain(domain);
			iclose();
			_exit(100);
		}
		if (!stralloc_copy(&tmpbuf, &dirbuf) ||
				!stralloc_catb(&tmpbuf, "/.users_per_level", 17) ||
				!stralloc_0(&tmpbuf)) {
			deldomain(domain);
			iclose();
			die_nomem(FATAL);
		}
		if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) == -1) {
			strerr_warn4(FATAL, "open: ", tmpbuf.s, ": ", &strerr_sys);
			deldomain(domain);
			iclose();
			_exit(111);
		}
		s = dbuf;
		s += (i = fmt_uint(s, (unsigned int) users_per_level));
		if (write(fd, dbuf, i) == -1) {
			strerr_warn4(FATAL, "write: ", tmpbuf.s, ": ", &strerr_sys);
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
			strerr_warn4(FATAL, "fchown: ", tmpbuf.s, dbuf, &strerr_sys);
			deldomain(domain);
			iclose();
			_exit(111);
		}
		close(fd);
	}
	if (base_path && !use_etrn) {
		if (!get_assign(domain, &dirbuf, &uid, &gid)) {
			strerr_warn4(FATAL, "domain ", domain, " does not exist", 0);
			deldomain(domain);
			iclose();
			_exit(100);
		}
		if (!stralloc_copy(&tmpbuf, &dirbuf) ||
				!stralloc_catb(&tmpbuf, "/.base_path", 11) ||
				!stralloc_0(&tmpbuf))
			die_nomem(FATAL);
		if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) == -1) {
			strerr_warn4(FATAL, "open: ", tmpbuf.s, ": ", &strerr_sys);
			deldomain(domain);
			iclose();
			_exit(111);
		}
		if (write(fd, base_path, str_len(base_path)) == -1) {
			strerr_warn4(FATAL, "write: ", tmpbuf.s, ": ", &strerr_sys);
			deldomain(domain);
			iclose();
			_exit(111);
		}
		if (fchown(fd, uid, gid)) {
			s = dbuf;
			s += fmt_strn(s, "(uid = ", 7);
			s += fmt_uint(s, uid);
			s += fmt_strn(s, ", gid = ", 8);
			s += fmt_uint(s, gid);
			s += fmt_strn(s, "): ", 3);
			*s = 0;
			strerr_warn4(FATAL, "fchown: ", tmpbuf.s, s, &strerr_sys);
			deldomain(domain);
			iclose();
			return (1);
		}
		close(fd);
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
	if (use_etrn)
		return (0);
	if (bounceEmail) {
		if (bounceEmail[i = str_chr(bounceEmail, '@')] || *bounceEmail == '/') {
			if (!stralloc_copy(&tmpbuf, &dirbuf) ||
					!stralloc_catb(&tmpbuf, "/.qmail-default", 15) ||
					!stralloc_0(&tmpbuf)) {
				deldomain(domain);
				iclose();
				die_nomem(FATAL);
			}
			if ((fd = open(tmpbuf.s, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) == -1) {
				strerr_warn4(FATAL, "open: ", tmpbuf.s, ": ", &strerr_sys);
				deldomain(domain);
				iclose();
				_exit(111);
			}
			if (!stralloc_copyb(&tmpbuf, "| ", 2) || !stralloc_cats(&tmpbuf, PREFIX) ||
					!stralloc_catb(&tmpbuf, "/sbin/vdelivermail '' ", 22) ||
					!stralloc_cats(&tmpbuf, bounceEmail) ||
					!stralloc_append(&tmpbuf, "\n")) {
				deldomain(domain);
				iclose();
				die_nomem(FATAL);
			}
			if (write(fd, tmpbuf.s, tmpbuf.len) == -1) {
				strerr_warn4(FATAL, "write: ", tmpbuf.s, ": ", &strerr_sys);
				deldomain(domain);
				iclose();
				_exit(111);
			}
			if (fchown(fd, uid, gid)) {
				s = dbuf;
				s += fmt_strn(s, "(uid = ", 7);
				s += fmt_uint(s, uid);
				s += fmt_strn(s, ", gid = ", 8);
				s += fmt_uint(s, gid);
				s += fmt_strn(s, "): \n", 4);
				*s = 0;
				strerr_warn4(FATAL, "fchown: ", tmpbuf.s, s, &strerr_sys);
				deldomain(domain);
				iclose();
				return (1);
			}
			close(fd);
		} else {
			deldomain(domain);
			iclose();
			strerr_die3x(100, FATAL, "Invalid bounce email address ",  bounceEmail);
		}
	}
	create_flag = 1;
	/*- Add the postmaster user */
#ifdef CLUSTERED_SITE
	if ((is_dist = is_distributed_domain(domain)) == -1) {
		deldomain(domain);
		iclose();
		strerr_warn4(WARN, "Unable to verify ", domain, " as a distributed domain", 0);
		_exit(100);
	} else
	if (is_dist) {
		if ((user_present = is_user_present("postmaster", domain)) == -1) {
			iclose();
			strerr_warn2(WARN, "auth db error", 0);
			_exit(100);
		} else
		if (user_present) {
			iclose();
			return (0);
		}
	}
#endif
	if ((err = iadduser("postmaster", domain, 0, passwd, "Postmaster", 0, users_per_level,
		Apop, 1)) != VA_SUCCESS) {
		if (errno != EEXIST) {
			deldomain(domain);
			iclose();
			_exit(111);
		}
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
			die_nomem(FATAL);
		}
		if (valias_insert(auto_ids[i], domain, AliasLine.s, 0)) {
			deldomain(domain);
			iclose();
			_exit (111);
		}
	}
	iclose();
	if (!(ptr = env_get("POST_HANDLE"))) {
		i = str_rchr(argv[0], '/');
		if (!*(base_argv0 = (argv[0] + i)))
			base_argv0 = argv[0];
		else
			base_argv0++;
		return (post_handle("%s/%s %s", LIBEXECDIR, base_argv0, domain));
	} else
		return (post_handle("%s %s", ptr, domain));
}

void
get_options(int argc, char **argv, char **base_path, char **dir_t, char **passwd,
	char **domain, char **user, char **quota, char **bounceEmail, char **ipaddr,
	char **database, char **sqlserver, char **dbuser, char **dbpass)
{
	int             c;
	struct passwd  *mypw;
	char            optbuf[40];
	char           *s;

	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	Uid = indimailuid;
	Gid = indimailgid;
	Apop = USE_POP;
#ifdef CLUSTERED_SITE
	s = optbuf;
	s += fmt_strn(s, "atT:q:l:bB:e:u:vCci:g:d:D:S:U:P:s:p:O", 35);
#else
	s = optbuf;
	s += fmt_strn(s, "atT:q:l:bB:e:u:vCi:g:d:O", 24);
#endif
#ifdef VFILTER
	s += fmt_strn(s, "f", 1);
#endif
	*s++ = 0;
	while ((c = getopt(argc, argv, optbuf)) != opteof) {
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
		case 'C':
			chk_rcpt = 1;
			break;
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
		case 'S':
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
						die_nomem(FATAL);
					else
					if (!stralloc_0(&dirbuf))
						die_nomem(FATAL);
					Uid = mypw->pw_uid;
					Gid = mypw->pw_gid;
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
		case 'e':
			*bounceEmail = optarg;
			break;
		case 'i':
			scan_uint(optarg, (unsigned int *) &Uid);
			break;
		case 'g':
			scan_uint(optarg, (unsigned int *) &Gid);
			break;
		case 'a':
			Apop = USE_APOP;
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
	if (optind < argc)
		*domain = argv[optind++];
	if (optind < argc)
		*passwd = argv[optind++];
	if (!*domain)
		strerr_die3x(100, WARN, "Domain not specified\n", usage);
#ifdef CLUSTERED_SITE
	if (distributed >=0 && (!*sqlserver || !*database || !*dbuser || !*dbpass || dbport == -1))
		strerr_die3x(100, WARN, "specify sqlserver, database, dbuser, dbpass and dbport\n", usage);
#endif
	return;
}
