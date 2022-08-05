/*
 * $Log: dbinfo.c,v $
 * Revision 1.5  2022-08-05 21:01:35+05:30  Cprogrammer
 * replaced fprintf with strerr_warn1
 *
 * Revision 1.4  2020-04-01 18:54:05+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.3  2019-06-07 15:59:13+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:10:07+05:30  Cprogrammer
 * added missing header strerr.h
 *
 * Revision 1.1  2019-04-17 12:47:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: dbinfo.c,v 1.5 2022-08-05 21:01:35+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#ifdef HAVE_CONFIG_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <sgetopt.h>
#include <strerr.h>
#include <env.h>
#include <fmt.h>
#include <subfd.h>
#include <scan.h>
#include <getEnvConfig.h>
#endif
#include "LoadDbInfo.h"
#include "iclose.h"
#include "variables.h"
#include "common.h"
#include "get_indimailuidgid.h"
#include "LoadDbInfo.h"
#include "runcmmd.h"
#include "dbinfoAdd.h"
#include "dbinfoDel.h"
#include "dbinfoSelect.h"
#include "dbinfoUpdate.h"

#define DBINFO_SELECT 1
#define DBINFO_INSERT 2
#define DBINFO_UPDATE 3
#define DBINFO_DELETE 4
#define DBINFO_EDIT   5
#define WARN    "dbinfo: warning: "
#define FATAL   "dbinfo: fatal: "

static int      row_format;
char           *usage =
	"usage: dbinfo [options] MCDF_file_PATH\n"
	"       dbinfo -i -S MySQL_server  -p port -o use_ssl -D database -U user -P password  [-c] -d domain -m mdahost MCDF_file\n"
	"       dbinfo -u -S MySQL_server [-p port -o use_ssl -D database -U user -P password] [-c] -d domain -m mdahost MCDF_file\n"
	"       dbinfo -r -d domain -m mdahost MCDF_file\n"
	"       dbinfo -s [-d domain] [-m mdahost] MCDF_file\n"
	"       dbinfo -e\n"
	"options: -v verbose\n"
	"         -i Insert Entry\n"
	"         -u Update Entry\n"
	"         -r Remove Entry\n"
	"         -s Show   Entries\n"
	"         -d Domain Name\n"
	"         -c If Clustered\n"
	"         -S MySQL Server IP\n"
	"         -p MySQL Port\n"
	"         -o Use SSL\n"
	"         -D MySQL Database Name\n"
	"         -U MySQL User Name\n"
	"         -P MySQL Password\n"
	"         -m Mail Store IP\n"
	"         -e Edit Dbinfo using vi\n"
	"         -l Row Format display"
	;

static int      get_options(int, char **, int *, int *, char **, int *, int *, char **, char **, char **, char **, char **, char **);
static int      editdbinfo(char *);

static void
die_nomem()
{
	strerr_warn1("dbinfo: out of memory", 0);
	_exit(111);
}

static int
getch(ch)
	char           *ch;
{
	int             r;

	if ((r = substdio_get(subfdinsmall, ch, 1)) == -1)
		strerr_die1sys(111, "dbinfo: getch: unable to read input: ");
	return (r);
}

static int
get_options(int argc, char **argv, int *option, int *distributed, char **mysql_server, int *mysql_port, int *use_ssl,
		char **mysql_database, char **mysql_user, char **mysql_pass, char **mdahost, char **domain, char **filename)
{
	int             c;

	row_format = 0;
	*distributed = 0;
	*option = *mysql_port = -1;
	*use_ssl = 1;
	*mysql_server = *mysql_database = *mysql_user = *mysql_pass = *mdahost = *domain = *filename = 0;
	while ((c = getopt(argc, argv, "viurseld:cS:p:o:D:U:P:m:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			*option = DBINFO_SELECT;
			break;
		case 'i':
			*option = DBINFO_INSERT;
			break;
		case 'u':
			*option = DBINFO_UPDATE;
			break;
		case 'r':
			*option = DBINFO_DELETE;
			break;
		case 'd':
			*domain = optarg;
			break;
		case 'c':
			*distributed = 1;
			break;
		case 'S':
			*mysql_server = optarg;
			break;
		case 'p':
			scan_uint(optarg, (unsigned int *) mysql_port);
			break;
		case 'o':
			scan_uint(optarg, (unsigned int *) use_ssl);
			break;
		case 'D':
			*mysql_database = optarg;
			break;
		case 'U':
			*mysql_user = optarg;
			break;
		case 'P':
			*mysql_pass = optarg;
			break;
		case 'm':
			*mdahost = optarg;
			break;
		case 'e':
			*option = DBINFO_EDIT;
			break;
		case 'l':
			row_format = 1;
			break;
		default:
			strerr_die3x(100, WARN, "\n", usage);
			return (1);
		} /*- switch(c) */
	} /*- while ((c = getopt(argc, argv, "viurd:cs:P:D:u:p:m:")) != -1) */
	if (optind < argc)
		*filename = argv[optind++];
	if (*option == -1 || (*option != DBINFO_SELECT && *option != DBINFO_EDIT && !*filename))
		strerr_die3x(100, WARN, "\n", usage);
	else
	switch(*option)
	{
	case DBINFO_INSERT:
		if (*distributed == -1 || !*mysql_server || *mysql_port == -1 || !*mysql_database || !*mysql_user || !*mysql_pass)
			strerr_die3x(100, WARN, "\n", usage);
	case DBINFO_UPDATE:
	case DBINFO_DELETE:
		if (!*domain || !*mdahost)
			strerr_die3x(100, WARN, "\n", usage);
		break;
	case DBINFO_EDIT:
		break;
	}
	return (0);
}
int
main(int argc, char **argv)
{
	char           *m_server, *m_database, *m_user, *m_pass, *mdahost, *domain, *filename;
	int             opt, dist, m_port, err, total, m_use_ssl;

	if (get_options(argc, argv, &opt, &dist, &m_server, &m_port, &m_use_ssl, &m_database, &m_user, &m_pass, &mdahost, &domain, &filename))
		return (1);
	switch (opt)
	{
	case DBINFO_SELECT:
		if (!(err = dbinfoSelect(filename, domain, mdahost, row_format)))
			LoadDbInfo_TXT(&total);
		break;
	case DBINFO_INSERT:
		if (filename && *filename && !env_put2("MCDFILE", filename))
			die_nomem();
		if (!(err = dbinfoAdd(domain, dist, m_server, mdahost, m_port, m_use_ssl, m_database, m_user, m_pass)))
			LoadDbInfo_TXT(&total);
		break;
	case DBINFO_UPDATE:
		if (filename && *filename && !env_put2("MCDFILE", filename))
			die_nomem();
		if (!(err = dbinfoUpdate(domain, dist, m_server, mdahost, m_port, m_use_ssl, m_database, m_user, m_pass)))
			LoadDbInfo_TXT(&total);
		break;
	case DBINFO_DELETE:
		if (filename && *filename && !env_put2("MCDFILE", filename))
			die_nomem();
		if (!(err = dbinfoDel(domain, mdahost))) {
			if (!access(filename, F_OK) && unlink(filename)) {
				strerr_warn3("dbinfo: unlink: ", filename, ": ", &strerr_sys);
				return (1);
			}
			LoadDbInfo_TXT(&total);
		}
		break;
	case DBINFO_EDIT:
		if (filename && *filename && !env_put2("MCDFILE", filename))
			die_nomem();
		err = editdbinfo(filename);
		break;
	default:
		err = -1;
	}
	iclose();
	return (err);
}

static int
editdbinfo(char *filename)
{
	static stralloc mcdFile = {0}, TmpBuf = {0};
	char           *ptr, *sysconfdir, *controldir, *mcdfile;
	char            strnum[FMT_ULONG], ch[1];
	DBINFO        **rhostsptr, **tmpPtr;
	uid_t           uid;
	gid_t           gid;
	int             total, count;

	verbose = 1;
	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (!filename || !*filename)
		getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	else
		mcdfile = filename;
	if (*mcdfile == '/' || *mcdfile == '.') {
		if (!stralloc_copys(&mcdFile, mcdfile) || !stralloc_0(&mcdFile))
			die_nomem();
	} else {
		if (*controldir == '/') {
			if (!stralloc_copys(&mcdFile, controldir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, mcdfile) ||
					!stralloc_0(&mcdFile))
				die_nomem();
			mcdFile.len--;
		} else {
			if (!stralloc_copys(&mcdFile, sysconfdir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, controldir) ||
					!stralloc_append(&mcdFile, "/") ||
					!stralloc_cats(&mcdFile, mcdfile) ||
					!stralloc_0(&mcdFile))
				die_nomem();
			mcdFile.len--;
		}
	}
	rhostsptr = LoadDbInfo_TXT(&total);
	for (count = 0, tmpPtr = rhostsptr; tmpPtr && *tmpPtr; tmpPtr++) {
		if ((*tmpPtr)->isLocal)
			continue;
		count++;
	}
	out("dbinfo", "Loaded ");
	strnum[fmt_uint(strnum, count)] = 0;
	out("dbinfo", strnum);
	out("dbinfo", " entries in file ");
	out("dbinfo", mcdFile.s);
	out("dbinfo", "\nPress ENTER to continue");
	flush("dbinfo");
	getch(ch);
	if (rhostsptr && access(mcdFile.s, F_OK)) {
		if (writemcdinfo(rhostsptr, time(0))) {
			strerr_warn1("dbinfo: writemcdinfo failed", 0);
			return (1);
		}
	}
	getEnvConfigStr(&ptr, "EDITOR", "vi");
	out("dbinfo", "Editor is ");
	out("dbinfo", ptr);
	out("dbinfo", "\n");
	flush("dbinfo");
	if (!stralloc_copys(&TmpBuf, ptr) ||
			!stralloc_append(&TmpBuf, " ") ||
			!stralloc_cat(&TmpBuf, &mcdFile) ||
			!stralloc_0(&TmpBuf))
		die_nomem();
	runcmmd(TmpBuf.s, 1);
	if (indimailuid == -1 || indimailgid == -1)
		get_indimailuidgid(&indimailuid, &indimailgid);
	uid = indimailuid;
	gid = indimailgid;
	if (chown(mcdFile.s, uid, gid))
		strerr_warn3("dbinfo: chown: ", mcdFile.s, ": ", &strerr_sys);
	return ((LoadDbInfo_TXT(&total) ? 0 : 1));
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return (0);
}
#endif
