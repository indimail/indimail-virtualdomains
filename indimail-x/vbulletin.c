/*
 * $Id: vbulletin.c,v 1.11 2025-05-13 20:37:02+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <error.h>
#include <strmsg.h>
#include <open.h>
#include <substdio.h>
#include <getln.h>
#include <str.h>
#include <fmt.h>
#include <env.h>
#include <setuserid.h>
#include <wait.h>
#endif
#include "iopen.h"
#include "iclose.h"
#include "get_assign.h"
#include "check_group.h"
#include "bulletin.h"
#include "get_real_domain.h"
#include "fappend.h"
#include "variables.h"
#include "indimail.h"
#include "CopyEmailFile.h"
#include "sql_getall.h"
#include "sql_getpw.h"

#ifndef	lint
static char     sccsid[] = "$Id: vbulletin.c,v 1.11 2025-05-13 20:37:02+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL            "vbulletin: fatal: "
#define WARN             "vbulletin: warning: "
#define COPY_IT          0
#define HARD_LINK_IT     1
#define SYMBOLIC_LINK_IT 2
#define BULK_BULLETIN    3
#define USER_BULLETIN    4
#define DOMAIN_BULLETIN  5

static stralloc tmpbuf = {0};
static int      DoNothing, DeliveryMethod = COPY_IT;
static char     strnum1[FMT_ULONG], strnum2[FMT_ULONG];

static char    *usage =
	"usage: vbulletin -f email_file [-e exclude_email_addr_file]\n"
	"                               [-v (verbose)]\n"
	"                               [-n (don't mail)]\n"
	"                               [-c (default, copy file)]\n"
	"                               [-h (use hard links)]\n"
	"                               [-s (use symbolic links)]\n"
	"                               [-S Subsriber_list_file (use Instant Mysql Bulletin)]\n"
	"                               [-a (Instant Bulletin for entire domain)]\n"
	"                               [ virtual_domain... | Email]"
	;

static int
get_options(int argc, char **argv, char **email, char **domain, char **emailFile, char **excludeFile, char **subscriberList)
{
	int             c, i;
	char           *ptr;

	*email = *domain = *emailFile = *excludeFile = *subscriberList = 0;
	verbose = 0;
	DoNothing = 0;
	while ((c = getopt(argc, argv, "VcshnaS:f:e:")) != opteof) {
		switch (c)
		{
		case 'V':
			verbose = 1;
			break;
		case 'c':
			if (DeliveryMethod != BULK_BULLETIN && DeliveryMethod != DOMAIN_BULLETIN)
				DeliveryMethod = COPY_IT;
			break;
		case 's':
			if (DeliveryMethod != BULK_BULLETIN && DeliveryMethod != DOMAIN_BULLETIN)
				DeliveryMethod = SYMBOLIC_LINK_IT;
			break;
		case 'h':
			if (DeliveryMethod != BULK_BULLETIN && DeliveryMethod != DOMAIN_BULLETIN)
				DeliveryMethod = HARD_LINK_IT;
			break;
		case 'n':
			DoNothing = 1;
			break;
		case 'e':
			if (*optarg != '/') {
				strerr_warn3("vbulletin: path [", optarg, "] is not absolute", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			} else
			if (access(optarg, F_OK)) {
				strerr_warn2(optarg, ": ", &strerr_sys);
				return (1);
			} else
				*excludeFile = optarg;
			break;
		case 'f':
			if (*optarg != '/') {
				strerr_warn3("vbulletin: path [", optarg, "] is not absolute", 0);
				strerr_warn2(WARN, usage, 0);
				return (1);
			} else
			if (access(optarg, F_OK)) {
				strerr_warn2(optarg, ": ", &strerr_sys);
				return (1);
			} else
				*emailFile = optarg;
			break;
		case 'a':
			DeliveryMethod = DOMAIN_BULLETIN;
			break;
		case 'S':
			DeliveryMethod = BULK_BULLETIN;
			if (!*optarg) {
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
			if (access(optarg, F_OK)) {
				strerr_warn2(optarg, ": ", &strerr_sys);
				return (1);
			}
			*subscriberList = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (optind < argc) {
		i = str_chr(argv[optind], '@');
		if (argv[optind][i]) {
			for (ptr = argv[optind]; *ptr; ptr++) {
				if (isupper(*ptr))
					strerr_die4x(100, WARN, "email [", argv[optind], "] has an uppercase character");
			}
			DeliveryMethod = USER_BULLETIN;
			*email = argv[optind++];
		} else {
			for (ptr = argv[optind]; *ptr; ptr++) {
				if (isupper(*ptr))
					strerr_die4x(100, WARN, "domain [", argv[optind], "] has an uppercase character");
			}
			*domain = argv[optind++];
		}
	}
	if (DeliveryMethod == BULK_BULLETIN || DeliveryMethod == DOMAIN_BULLETIN) {
		if (*excludeFile || *email) {
			strerr_warn1("options -e or email cannot be used with bulk bulletin", 0);
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (DeliveryMethod == USER_BULLETIN) {
		if (*excludeFile) {
			strerr_warn1("option -e cannot be used with user bulletin", 0);
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
	}
	if (!*email && !*domain) {
		strerr_warn2(WARN, usage, 0);
		return (1);
	}
	return (0);
}

static void
die_nomem()
{
	strerr_warn1("vbulletin: out of memory", 0);
	_exit(111);
}

int
in_exclude_list(char *excludeFile, int fdx, char *user, char *domain)
{
	static stralloc line = {0};
	char            inbuf[1024];
	char           *ptr;
	struct substdio ssin;
	int             match;

	if (fdx == -1)
		return (0);
	if (!stralloc_copys(&tmpbuf, user) ||
			!stralloc_append(&tmpbuf, "@") ||
			!stralloc_cats(&tmpbuf, domain) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if (lseek(fdx, 0, SEEK_SET) != 0)
		strerr_die1sys(111, "vbulletin: lseek error: ");
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fdx, inbuf, sizeof (inbuf));
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die3sys(111, "vbulletin: read: ", excludeFile, ": ");
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len)
				continue;
			line.s[line.len] = 0;
		} else {
			if (!stralloc_0(&line))
				die_nomem();
			line.len--;
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (!str_diffn(tmpbuf.s, ptr, tmpbuf.len))
			return (1);
	}
	return (0);
}

int
copy_email(char *emailFile, int fdi, char *name, char *domain, struct passwd *pwent)
{
	char            inbuf[1024], outbuf[512];
	int             fdw;
	struct substdio ssin, ssout;

	if (DeliveryMethod == COPY_IT) {
		if (!stralloc_copys(&tmpbuf, pwent->pw_dir) ||
				!stralloc_catb(&tmpbuf,"/Maildir/new/", 13) ||
				!stralloc_cats(&tmpbuf, name) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		if ((fdw = open_trunc(tmpbuf.s)) == -1) {
			strerr_warn3("vbulletin: open: ", tmpbuf.s, ": ", &strerr_sys);
			return (1);
		}
		substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fdw, outbuf, sizeof(outbuf));
		if (substdio_put(&ssout, "To: ", 4) ||
				substdio_puts(&ssout, pwent->pw_name) ||
				substdio_put(&ssout, "@", 1) ||
				substdio_puts(&ssout, domain) ||
				substdio_put(&ssout, "\n", 1))
		{
			strerr_warn3("vbulletin: write: ", tmpbuf.s, ": ", &strerr_sys);
			close(fdw);
			return (1);
		}
		if (lseek(fdi, 0, SEEK_SET) != 0)
			strerr_die1sys(111, "vbulletin: lseek error: ");
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fdi, inbuf, sizeof (inbuf));
		switch (substdio_copy(&ssout, &ssin))
		{
		case -2: /*- read error */
			strerr_warn1("vbulletin: read error: ", &strerr_sys);
			close(fdw);
			_exit(111);
		case -3: /*- write error */
			strerr_warn3("vbulletin: write error: ", tmpbuf.s, ": ", &strerr_sys);
			close(fdw);
			_exit(111);
		}
		if (substdio_flush(&ssout) == -1) {
			strerr_warn3("vbulletin: write error: ", tmpbuf.s, ": ", &strerr_sys);
			close(fdw);
			_exit(111);
		}
		close(fdw);
	} else
	if (DeliveryMethod == HARD_LINK_IT) {
		if (link(emailFile, tmpbuf.s) < 0) {
			strerr_warn5("vbulletin: hardlink: ", emailFile, " --> ", tmpbuf.s, ": ", &strerr_sys);
		}
	} else
	if (DeliveryMethod == SYMBOLIC_LINK_IT) {
		if (symlink(emailFile, tmpbuf.s) < 0)
			strerr_warn5("vbulletin: symlink: ", emailFile, " --> ", tmpbuf.s, ": ", &strerr_sys);
	} else
		strmsg_out1("no delivery method set\n");
	return (0);
}

int
process_domain(char *emailFile, char *excludeFile, char *domain)
{
	static stralloc filename = {0}, Dir = {0};
	char            hostname[128];
	static struct passwd *pwent;
	time_t          tm;
	int             i, j, fdi = -1, fdx = -1, pid, first = 1;
	uid_t           uid, myuid;
	gid_t           gid, mygid;

	if (*emailFile && fdi == -1 && (fdi = open_read(emailFile)) == -1) {
		strerr_warn3("vbulletin: open_read: ", emailFile, ": ", &strerr_sys);
		return (1);
	}
	if (*excludeFile && fdx == -1 && (fdx = open_read(excludeFile)) == -1) {
		strerr_warn3("vbulletin: open_read: ", excludeFile, ": ", &strerr_sys);
		return (1);
	}
	if (gethostname(hostname, sizeof(hostname) - 1) == -1) {
		strerr_warn1("vbulletin: gethostname: ", &strerr_sys);
		return (1);
	}
	pid = getpid();
	time(&tm);
	if (!get_assign(domain, &Dir, &uid, &gid)) {
		strerr_warn2(domain, ": No such Domain", 0);
		return (1);
	}
	if (!uid)
		strerr_die3x(100, "vbulletin: domain ", domain, " with uid 0");
	myuid = getuid();
	mygid = getgid();
	if (myuid != 0 && myuid != uid && mygid != gid && check_group(gid, FATAL) != 1) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vbulletin: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
		return (1);
	}
	if (setuser_privileges(uid, gid, "indimail")) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vbulletin: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
		return (1);
	}
	strnum1[i = fmt_ulong(strnum1, tm)] = 0;
	strnum2[j = fmt_ulong(strnum1, pid)] = 0;
	if (!stralloc_copyb(&filename, strnum1, i) ||
			!stralloc_append(&filename, ".") ||
			!stralloc_catb(&filename, strnum2, j) ||
			!stralloc_append(&filename, ".") ||
			!stralloc_cats(&filename, hostname) ||
			!stralloc_0(&filename))
		die_nomem();
	first = 1;
	while ((pwent = sql_getall(domain, first, 0)) != NULL) {
		first = 0;
		if (!in_exclude_list(excludeFile, fdx, pwent->pw_name, domain)) {
			if (verbose == 1)
				strmsg_out5("Processing ", pwent->pw_name, "@", domain, "\n");
			if (DoNothing == 0)
				copy_email(emailFile, fdi, filename.s, domain, pwent);
		}
	}
	return (0);
}

static int
spost(char *EmailOrDomain, int method, char *emailFile, int bulk)
{
	static stralloc bulkdir = {0}, User = {0}, Domain = {0};
	char           *ptr;
	const char     *domain = NULL;
	int             i, copy_method;
	uid_t           uid;
	gid_t           gid;
	struct stat     statbuf;
	struct passwd  *mypw;

	if (!bulk) {
		if (method == USER_BULLETIN) {
			if (!stralloc_copys(&User, EmailOrDomain) ||
				 !stralloc_0(&User))
			die_nomem();
			User.len--;
		} else
		if (method == DOMAIN_BULLETIN) {
			if (!stralloc_copys(&Domain, EmailOrDomain) ||
				 !stralloc_0(&Domain))
			die_nomem();
			Domain.len--;
		}
		strmsg_out4("Processing ", method == USER_BULLETIN ? "Email " : "Domain ", method == USER_BULLETIN ? User.s : Domain.s, "\n");
		if (!User.len || !Domain.len) {
			strerr_warn3("Invalid Email Address [", EmailOrDomain, "]", 0);
			return (1);
		}
	} else {
		if (!stralloc_copys(&Domain, EmailOrDomain) ||
			 !stralloc_0(&Domain))
		die_nomem();
		Domain.len--;
	}
	if (!(domain = get_real_domain(Domain.s))) {
		strerr_warn2(Domain.s, ": No such domain", 0);
		return (1);
	}
	if (!stralloc_copys(&bulkdir, INDIMAILDIR) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, domain) ||
			!stralloc_append(&bulkdir, "/") ||
			!stralloc_cats(&bulkdir, (ptr = env_get("BULK_MAILDIR")) ? ptr : BULK_MAILDIR) ||
			!stralloc_0(&bulkdir))
		die_nomem();
	bulkdir.len--;
	if (access(bulkdir.s, F_OK)) {
		strerr_warn3("vbulletin: access: ", bulkdir.s, ": ", &strerr_sys);
		return (1);
	}
	i = str_rchr(emailFile, '/');
	if (emailFile[i]) {
		ptr = emailFile + i + 1;
	} else
		ptr = emailFile;
	if (bulk) {
		if (!stralloc_copy(&tmpbuf, &bulkdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, ptr) ||
				!stralloc_catb(&tmpbuf, ",all", 4) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		if (!stralloc_copy(&tmpbuf, &bulkdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, ptr) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	if (access(emailFile, F_OK)) {
		strerr_warn3("vbulletin: access: ", emailFile, ": ", &strerr_sys);
		return (1);
	} else
	if (!access(tmpbuf.s, F_OK)) {
		errno = EEXIST;
		strerr_warn3("vbulletin: ", tmpbuf.s, ": ", &strerr_sys);
		return (1);
	}
	if (!get_assign(domain, 0, &uid, &gid)) {
		strerr_warn2(domain, ": No such Domain", 0);
		return (1);
	}
	if (!uid)
		strerr_die3x(100, "vbulletin: domain ", domain, " with uid 0");
	if (fappend(emailFile, tmpbuf.s, "w", INDIMAIL_QMAIL_MODE, uid, gid)) {
		strerr_warn5("vbulletin: fappend: ", emailFile, " --> ", tmpbuf.s, ": ", &strerr_sys);
		return (-1);
	}
	if (stat(tmpbuf.s, &statbuf)) {
		strerr_warn3("vbulletin: stat: ", tmpbuf.s, ": ", &strerr_sys);
		return (1);
	}
	if (bulk)
		return (0);
	if (setuser_privileges(uid, gid, "indimail")) {
		strnum1[fmt_ulong(strnum1, uid)] = 0;
		strnum2[fmt_ulong(strnum2, gid)] = 0;
		strerr_warn5("vbulletin: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
		return (-1);
	}
	if (iopen((char *) 0))
		return (1);
	if (!(mypw = sql_getpw(User.s, domain))) {
		if (userNotFound)
			strerr_warn2(EmailOrDomain, ": No such User", 0);
		return (1);
	}
	if (DeliveryMethod == SYMBOLIC_LINK_IT)
		copy_method = 1;
	else
		copy_method = 0;
	if (CopyEmailFile(mypw->pw_dir, tmpbuf.s, EmailOrDomain, EmailOrDomain, 0, 0, 1, copy_method, statbuf.st_size)) {
		iclose();
		return (1);
	}
	iclose();
	return (0);
}

int
main(int argc, char **argv)
{
	DIR            *entry;
	struct dirent  *dp;
	pid_t           pid;
	int             fdsourcedir, wait_status, status, werr, i;
	char           *email, *domain, *emailFile, *excludeFile, *subscriberList;

	if (get_options(argc, argv, &email, &domain, &emailFile, &excludeFile, &subscriberList))
		return (1);
	if ((fdsourcedir = open_read(".")) == -1)
		strerr_die1sys(111, "vbulletin: unable to open current directory: ");
	if (DeliveryMethod == DOMAIN_BULLETIN) {
		if (DoNothing == 0)
			return (spost(domain, DeliveryMethod, emailFile, 1));
	} else
	if (DeliveryMethod == BULK_BULLETIN) {
		if (DoNothing == 0)
			return (bulletin(emailFile, subscriberList) == -1 ? 1 : 0);
	}
	if (DeliveryMethod == USER_BULLETIN) {
		if (DoNothing == 0)
			return (spost(email, DeliveryMethod, emailFile, 0));
	}
	if (*domain)
		status = process_domain(emailFile, excludeFile, domain);
	else {
		if (chdir(INDIMAILDIR) != 0) {
			strerr_warn3("vbulletin: chdir: ", INDIMAILDIR, ": ", &strerr_sys);
			return (1);
		}
		if (chdir("domains") != 0) {
			strerr_warn1("vbulletin: chdir: domains: ", &strerr_sys);
			return (1);
		}
		if (!(entry = opendir("."))) {
			strerr_warn1("vbulletin: opendir: domains: ", &strerr_sys);
			return (1);
		}
		while ((dp = readdir(entry)) != NULL) {
			if (str_diffn(dp->d_name, ".", 1) == 0)
				continue;
			switch (pid = fork())
			{
			case -1:
				break;
			case 0:
				if (process_domain(emailFile, excludeFile, dp->d_name))
					strerr_warn2(dp->d_name, ": Bulletin failed", 0);
				break;
			default:
				for (;;) {
					if (!(i = wait_pid(&wait_status, pid)))
						break;
					else
					if (i == -1) {
#ifdef ERESTART
						if (errno == error_intr || errno == error_restart)
#else
						if (errno == error_intr)
#endif
							continue;
						strnum1[fmt_ulong(strnum1, pid)] = 0;
						strerr_warn3("vbulletin: waitpid: ", strnum1, ": ", &strerr_sys);
						status = -1;
					}
					if (!(i = wait_handler(wait_status, &werr)) && werr)
						continue;
					else
					if (werr == -1) {
						strnum1[fmt_ulong(strnum1, pid)] = 0;
						strerr_warn3("vbulletin: ", strnum1, ": internal wait handler error", 0);
						status = -1;
					} else
					if (werr) {
						strnum1[fmt_ulong(strnum1, pid)] = 0;
						strnum2[fmt_uint(strnum2, werr)] = 0;
						strerr_warn4("vbulletin: ", strnum1, ": killed by signal ", strnum2, 0);
						status = -1;
					}
					if (i)
						status = -1;
					break;
				}
			} /* switch (pid = fork()) */
		} /*- while ((dp = readdir(entry)) != NULL) */
		closedir(entry);
		if (fchdir(fdsourcedir) == -1)
			strerr_die1sys(111, "vbulletin: unable to switch back to old working directory: ");
	}
	iclose();
	return (status);
}
/*
 * $Log: vbulletin.c,v $
 * Revision 1.11  2025-05-13 20:37:02+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.10  2024-05-17 16:25:48+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.9  2023-03-26 00:33:27+05:30  Cprogrammer
 * fixed code using wait_handler
 *
 * Revision 1.8  2023-03-24 12:59:25+05:30  Cprogrammer
 * fork and run bulletin for all domains with domain uid
 *
 * Revision 1.7  2023-03-20 10:33:13+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.6  2022-10-20 11:58:36+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.5  2021-07-08 11:47:41+05:30  Cprogrammer
 * add check for misconfigured assign file
 *
 * Revision 1.4  2020-10-19 12:47:03+05:30  Cprogrammer
 * use /var/indomain/domains for domain/bulk_mail
 *
 * Revision 1.3  2020-06-16 17:56:15+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.2  2019-06-07 15:54:44+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 08:38:42+05:30  Cprogrammer
 * Initial revision
 *
 */
