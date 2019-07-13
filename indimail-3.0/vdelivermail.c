/*
 * $Log: vdelivermail.c,v $
 * Revision 1.5  2019-06-17 23:24:18+05:30  Cprogrammer
 * fixed SMTPROUTE, QMTPROUTE env variable
 *
 * Revision 1.4  2019-04-22 23:18:56+05:30  Cprogrammer
 * replaced exit with _exit
 *
 * Revision 1.3  2019-04-21 16:16:04+05:30  Cprogrammer
 * fixed directory length returned by prepare_maildir()
 *
 * Revision 1.2  2019-04-21 11:41:58+05:30  Cprogrammer
 * fixed directory lengths
 *
 * Revision 1.1  2019-04-18 08:15:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <getln.h>
#include <substdio.h>
#include <env.h>
#include <str.h>
#include <error.h>
#include <scan.h>
#include <fmt.h>
#include <byte.h>
#include <open.h>
#endif
#include "iclose.h"
#include "lowerit.h"
#include "variables.h"
#include "remove_quotes.h"
#include "get_real_domain.h"
#include "get_indimailuidgid.h"
#include "get_assign.h"
#include "r_mkdir.h"
#include "variables.h"
#ifdef USE_MAILDIRQUOTA
#include "parse_quota.h"
#endif
#include "recalc_quota.h"
#include "makeseekable.h"
#include "deliver_mail.h"
#include "sql_getpw.h"
#include "findhost.h"
#include "is_distributed_domain.h"
#include "get_local_hostid.h"
#include "sql_gethostid.h"
#include "vset_lastauth.h"
#include "vset_lastdeliver.h"
#include "getEnvConfig.h"
#include "valias_select.h"
#include "wildmat.h"
#include "indimail.h"
#include "common.h"
#include "runcmmd.h"
#include "strToPw.h"
#include "qmail_remote.h"
#include "get_message_size.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdelivermail.c,v 1.5 2019-06-17 23:24:18+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vdelivermail: fatal: "
#define WARN    "vdelivermail: warning: "

/*- Globals */
static stralloc tmpbuf = {0};
static char     strnum[FMT_ULONG];

/*- Function Prototypes */
static int      bhfcheck(char *);
static void     vdl_exit(int);

static void
die_nomem()
{
	strerr_warn1("vdelivermail: out of memory", 0);
	_exit(111);
}

void
vdl_exit(int err)
{
	iclose();
	if (env_get("DISCARD_BOUNCE") && err == 100)
		_exit(0);
	_exit(err);
}

/*
 * Get the command line arguments and the environment variables.
 * Force addresses to be lower case and set the default domain
 */
static void
get_arguments(int argc, char **argv, char **user, char **domain, char **user_ext, char **bounce)
{
	char           *tmpstr;
	static stralloc ReturnPathEnv = {0}, _user = {0}, _domain = {0};
#ifdef QMAIL_EXT
	static stralloc _user_ext = {0};
	int             i;
#endif

	*user = *domain = *bounce = 0;
	if (argc == 3) {
		*bounce = argv[2];
		/*- get the last parameter in the .qmail-default file */
		if (!(tmpstr = env_get("EXT"))) {
			strerr_warn1("No EXT environment variable", 0);
			vdl_exit(100);
		} else {
			if (*tmpstr)
				*user = tmpstr;
			else
				*user = "postmaster";
		}
		if (!(tmpstr = env_get("HOST")))
			*domain = ((tmpstr = (char *) env_get("DEFAULT_DOMAIN")) ? tmpstr : DEFAULT_DOMAIN);
		else
			*domain = tmpstr;
		if (remove_quotes(user)) {
			strerr_warn3("vdelivermail: invalid user [", *user, "]", 0);
			vdl_exit(100);
		}
	} else
	if (argc == 11) { /*- qmail-local */
		*user = argv[6];
		*domain = argv[7];
		if (!stralloc_copyb(&ReturnPathEnv, "Return-Path: ", 13) ||
				!stralloc_append(&ReturnPathEnv, "<") ||
				!stralloc_cats(&ReturnPathEnv, argv[8]) ||
				!stralloc_catb(&ReturnPathEnv, ">\n", 2) ||
				!stralloc_0(&ReturnPathEnv))
			die_nomem();
		if (!env_put2("RPLINE", ReturnPathEnv.s))
			strerr_die2sys(111, FATAL, "env_put2: RPLINE: ");
	} else {
		strerr_warn1("vdelivermail: invalid number of arguments", 0);
		vdl_exit(111);
	}
	if (!stralloc_copys(&_user, *user) || !stralloc_0(&_user))
		die_nomem();
	if (!stralloc_copys(&_domain, *domain) || !stralloc_0(&_domain))
		die_nomem();
	*user = _user.s;
	*domain = _domain.s;
	lowerit(*user);
	lowerit(*domain);
#ifdef QMAIL_EXT
	*user_ext = 0;
	if (env_get("QMAIL_EXT")) {
		for (i = 0; *user[i] && *user[i] != '-'; i++);
		if (!stralloc_copyb(&_user_ext, *user, i) || !stralloc_0(&_user_ext))
			die_nomem();
		*user_ext = _user_ext.s;
	}
#endif
	if (!(tmpstr = get_real_domain(*domain))) {
		strerr_warn3("vdelivermail: ", "No such domain ", *domain, 0);
		if (userNotFound)
			vdl_exit(100);
		else
			vdl_exit(111);
	} else {
		if (!stralloc_copys(&_domain, tmpstr) || !stralloc_0(&_domain))
			die_nomem();
		*domain = _domain.s;
	}
	return;
}

static char    *
prepare_maildir(char *dir, uid_t uid, gid_t gid)
{
	int             i, olen, fd;
	char           *maildirfolder;
	static stralloc TheDir = {0};
	char           *MailDirNames[] = {
		"cur",
		"new",
		"tmp",
	};
	maildirfolder = (char *) env_get("MAILDIRFOLDER");
	if (!stralloc_copys(&TheDir, dir))
		die_nomem();
	if (maildirfolder) {
		if (TheDir.s[TheDir.len - 1] != '/' && !stralloc_append(&TheDir, "/"))
			die_nomem();
		if (!stralloc_cats(&TheDir, maildirfolder) ||
				!stralloc_append(&TheDir, "/"))
			die_nomem();
	}
	olen = TheDir.len;
	for (i = 0; i < 3; i++) {
		if (!stralloc_catb(&TheDir, MailDirNames[i], 3) ||
				!stralloc_0(&TheDir))
			die_nomem();
		TheDir.len--;
		if (access(TheDir.s, F_OK)) {
			if (r_mkdir(TheDir.s, INDIMAIL_DIR_MODE, uid, gid)) {
				strerr_warn5("vdelivermail: access: ", TheDir.s, ": ", error_str(errno), ": indimail (#5.1.2)", 0);
				vdl_exit(111);
			}
			TheDir.len -= 3; /*- remove cur, new or tmp */
			if (!stralloc_catb(&TheDir, "/maildirfolder", 14) || !stralloc_0(&TheDir))
				die_nomem();
			TheDir.len -= 15; /*- dir/maildirfolder + 1 null character */
			if ((fd = open(TheDir.s, O_CREAT | O_TRUNC, 0644)) == -1) {
				strerr_warn3("vdelivermail: open: O_CREAT|O_TRUNC: ", TheDir.s, ": ", &strerr_sys);
				vdl_exit(111);
			}
			close(fd);
		}
		TheDir.len = olen;
	}
	if (!stralloc_0(&TheDir))
		die_nomem();
	return (TheDir.s);
}

void
quota_message(char *ptr, mdir_t msgsize, mdir_t MailQuotaCount, mdir_t MailQuotaSize)
{
	errout("vdelivermail", ptr);
	errout("vdelivermail", " has insufficient quota. ");
	strnum[fmt_ulong(strnum, msgsize)] = 0;
	errout("vdelivermail", strnum);
	errout("vdelivermail", "/");
	strnum[fmt_ulong(strnum, CurBytes)] = 0;
	errout("vdelivermail", strnum);
	errout("vdelivermail", ":");
	strnum[fmt_ulong(strnum, CurCount)] = 0;
	errout("vdelivermail", strnum);
	errout("vdelivermail", "/");
	strnum[fmt_ulong(strnum, MailQuotaCount)] = 0;
	errout("vdelivermail", strnum);
	errout("vdelivermail", ":");
	strnum[fmt_ulong(strnum, MailQuotaSize)] = 0;
	errout("vdelivermail", strnum);
	errout("vdelivermail", ". indimail (#5.1.4)");
	errflush("vdelivermail");
	return;
}
#ifdef VALIAS
/*
 * Process any valiases for this user@domain
 * 
 * This will look up any valiases in indimail and
 * deliver the email to the entries
 *
 * Return 1 if aliases found
 * Return 0 if no aliases found 
 */
int
process_valias(char *user, char *domain, mdir_t MsgSize)
{
	int             ret, found = 0, status = 0;
	mdir_t          MailQuotaSize, MailQuotaCount;
	char           *tmpstr;

	/*- Get the aliases for this user@domain */
	for (status = found = 0;; found++) {
		if (!(tmpstr = valias_select(user, domain)))
			break;
		if (tmpstr[0] == '/')
			tmpstr = prepare_maildir(tmpstr, indimailuid, indimailgid);
		if (!env_put2("NOALIAS", "1"))
			strerr_die2sys(111, FATAL, "env_put2: NOALIAS=1: ");
		ret = deliver_mail(tmpstr, MsgSize, "AUTO", indimailuid, indimailgid, domain, &MailQuotaSize, &MailQuotaCount);
		if (!env_unset("NOALIAS"))
			strerr_die2sys(111, FATAL, "env_unset: NOALIAS: ");
		if (ret == -1 || ret == -4) {
			if (status++) {
				out("vdelivermail", "\n\n");
				flush("vdelivermail");
			}
			if (ret == -1)
				quota_message(tmpstr, MsgSize, MailQuotaCount, MailQuotaSize);
			continue;
		} else
		if (ret == -2) {
			strerr_warn1("vdelivermail: temporary system error: ", &strerr_sys);
			vdl_exit(111);
		} else
		if (ret == -3) {	/* mail is looping */
			if (status++) {
				out("vdelivermail", "\n\n");
				flush("vdelivermail");
			}
			strerr_warn2(tmpstr, " is looping", tmpstr);
			continue;
		} else
		if (ret == -5) { /*- Defer Overquota mails */
			if (status++) {
				out("vdelivermail", "\n\n");
				flush("vdelivermail");
			}
			quota_message(tmpstr, MsgSize, MailQuotaCount, MailQuotaSize);
			vdl_exit(111);
		}
	} /*- for (found = 0;;found++) */
	if (status)
		vdl_exit(100);
	/*- Return whether we found an alias or not */
	return (found);
}
#endif

/*
 * Check if the indimail user has a .qmail file in their directory
 * and foward to each email address, Maildir or program 
 *  that is found there in that file
 *
 * Return:  1 if we found and delivered email
 *       :  0 if not found
 *       : -1 if no user .qmail file 
 */
int
doAlias(char *dir, char *user, char *domain, mdir_t MsgSize)
{
	static stralloc line = {0}, tmp = {0};
	mdir_t          MailQuotaSize, MailQuotaCount;
	char           *ptr;
	int             fd, match, ret = 0;
	struct substdio ssin;
	static char     inbuf[512];

	if ((ptr = env_get("NOALIAS")) && *ptr > '0')
		return (0);
#ifdef VALIAS
	/*- process valiases if configured */
	if (process_valias(user, domain, MsgSize))
		return (1);
#endif
	/*- format the file name */
	if (!stralloc_copys(&tmpbuf, dir) ||
			!stralloc_catb(&tmpbuf, "/.qmail", 7) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "vdelivermail: open: ", tmpbuf.s, ": ");
		return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	/*- format a simple loop checker name */
	if (!stralloc_copys(&tmp, user) ||
			!stralloc_append(&tmp, "@") ||
			!stralloc_cats(&tmp, domain) ||
			!stralloc_0(&tmp))
		die_nomem();
	tmp.len--;
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vdelivermail: read: ", tmpbuf.s, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (line.len == 0)
			break;
		if (match) {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		/*-
		 * simple loop check, if they are sending it to themselves
		 * then skip this line
		 */
		if (!str_diffn(line.s, tmp.s, tmp.len + 1))
			continue;
		if (line.s[0] == '/')
			ptr = prepare_maildir(line.s, indimailuid, indimailgid);
		else
			ptr = line.s;
		if (!env_put2("NOALIAS", "2"))
			strerr_die2sys(111, FATAL, "env_put2: NOALIAS=2: ");
		ret = deliver_mail(ptr, MsgSize, "AUTO", indimailgid, indimailgid, domain, &MailQuotaSize, &MailQuotaCount);
		if (!env_unset("NOALIAS"))
			strerr_die2sys(111, FATAL, "env_unset: NOALIAS: ");
		if (ret == 99)
			break;
		if (ret == -1 || ret == -4) {
			if (ret == -1)
				quota_message(line.s, MsgSize, MailQuotaCount, MailQuotaSize);
			vdl_exit(100);
		} else
		if (ret == -2)
			vdl_exit(111);
		else
		if (ret == -3) { /* mail is looping */
			strerr_warn2(line.s, " is looping", 0);
			vdl_exit(100);
		} else
		if (ret == -5) { /*- Defer Overquota mails */
			quota_message(line.s, MsgSize, MailQuotaCount, MailQuotaSize);
			vdl_exit(111);
		}
	}
	close(fd);
	/*- A Blank .qmail file will result in mails getting blackholed */
	return (1);
}

static int
processMail(struct passwd *pw, char *user, char *domain, mdir_t MsgSize)
{
	static stralloc TheDir = {0};
	char           *ptr, *maildirfolder;
	int             i, ret;
	mdir_t          cur_size, mail_size_limit, cur_count = 0;
#ifdef USE_MAILDIRQUOTA
	mdir_t          mail_count_limit;
#endif
	mdir_t          MailQuotaSize, MailQuotaCount;
	uid_t           uid;
	gid_t           gid;

	if (!get_assign(domain, 0, &uid, &gid)) {
		strerr_warn2(domain, ": domain does not exist", 0);
		vdl_exit(111);
	}
	/*-
	 * check if the account is locked and their email
	 * should be bounced back
	 */
	if (pw->pw_gid & BOUNCE_MAIL) {
		if (!stralloc_copys(&tmpbuf, pw->pw_dir) ||
				!stralloc_catb(&tmpbuf, "/Maildir", 8) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		tmpbuf.len--;
#ifdef USE_MAILDIRQUOTA
		if ((mail_size_limit = parse_quota(pw->pw_shell, &mail_count_limit)) == -1) {
			strerr_warn3("vdelivermail: parse_quota: ", pw->pw_shell, ": ", &strerr_sys);
			return (-1);
		}
		cur_size = recalc_quota(tmpbuf.s, &cur_count, mail_size_limit, mail_count_limit, 0);
#else
		scan_ulong(pw->pw_shell, &mail_size_limit);
		cur_size = recalc_quota(tmpbuf.s, 0);
#endif
		/*-
		 * Remove bounce flag if a message size of 1024000 can be delivered to the user 
		 */
		if (str_diffn(pw->pw_shell, "NOQUOTA", 8) && ((cur_size + 1024000) < mail_size_limit))
			vset_lastdeliver(user, domain, 0);
		else {
			getEnvConfigStr(&ptr, "OVERQUOTA_CMD", LIBEXECDIR "/overquota.sh");
			if (!access(ptr, X_OK)) {
				if (!stralloc_copys(&TheDir, ptr) ||
						!stralloc_append(&TheDir, " ") ||
						!stralloc_cats(&TheDir, pw->pw_dir) ||
						!stralloc_catb(&TheDir, "/Maildir", 8))
					die_nomem();
				/*- Call overquota command with 5 arguments */
				if ((maildirfolder = (char *) env_get("MAILDIRFOLDER"))) {
					if (!stralloc_append(&TheDir, "/") ||
							!stralloc_cats(&TheDir, maildirfolder))
						die_nomem();
				}
				if (!stralloc_append(&TheDir, " "))
					die_nomem();
				strnum[i = fmt_ulong(strnum, MsgSize)] = 0;
				if (!stralloc_catb(&TheDir, strnum, i) || !stralloc_append(&TheDir, " "))
					die_nomem();
				strnum[i = fmt_ulong(strnum, cur_size)] = 0;
				if (!stralloc_catb(&TheDir, strnum, i) || !stralloc_append(&TheDir, " "))
					die_nomem();
				strnum[i = fmt_ulong(strnum, cur_count)] = 0;
				if (!stralloc_catb(&TheDir, strnum, i) || !stralloc_append(&TheDir, " "))
					die_nomem();
				if (!stralloc_cats(&TheDir, pw->pw_shell) || !stralloc_0(&TheDir))
					die_nomem();
				runcmmd(TheDir.s, 0);
			}
			errout("vdelivermail", "account ");
			errout("vdelivermail", user);
			errout("vdelivermail", "@");
			errout("vdelivermail", domain);
			errout("vdelivermail", " locked/overquota ");
			strnum[fmt_ulong(strnum, cur_size)] = 0;
			errout("vdelivermail", strnum);
			errout("vdelivermail", " / ");
			strnum[fmt_ulong(strnum, mail_size_limit)] = 0;
			errout("vdelivermail", strnum);
			errout("vdelivermail", ". indimail (#5.1.1)");
			errflush("vdelivermail");
			getEnvConfigStr(&ptr, "HOLDOVERQUOTA", "holdoverquota");
			if (ptr && *ptr == '/') {
				if (!stralloc_copys(&TheDir, ptr) || !stralloc_0(&TheDir))
					die_nomem();
			} else {
				if (!stralloc_copy(&TheDir, &tmpbuf) ||
						!stralloc_append(&TheDir, "/") ||
						!stralloc_cats(&TheDir, ptr) ||
						!stralloc_0(&TheDir))
					die_nomem();
			}
			if (!access(TheDir.s, F_OK))
				vdl_exit(111);
			vdl_exit(100);
		}
	}
	/*-
	 * check for a .qmail file in Maildir or valias table
	 * If either exists, then carry out delivery instructions
	 * and skip Maildir delivery
	 */
	ptr = (char *) env_get("EXT");
	if (ptr && *ptr && doAlias(pw->pw_dir, user, domain, MsgSize) == 1)
		vdl_exit(0);
	if (!str_diffn(pw->pw_gecos, "MailGroup ", 10)) {
		strerr_warn1("Mail delivery to group is avoided", 0);
		return (0);
	}
	if (!stralloc_copys(&TheDir, pw->pw_dir) ||
			!stralloc_catb(&TheDir, "/Maildir/", 9) ||
			!stralloc_0(&TheDir))
		die_nomem();
	ptr = prepare_maildir(TheDir.s, uid, gid);
	ret = deliver_mail(ptr, MsgSize, pw->pw_shell, uid, gid, domain, &MailQuotaCount, &MailQuotaSize);
	if (ret == -1 || ret == -4) {
		if (ret == -1) {
			if (!stralloc_copys(&tmpbuf, user) ||
					!stralloc_append(&tmpbuf, "@") ||
					!stralloc_cats(&tmpbuf, domain) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			quota_message(tmpbuf.s, MsgSize, MailQuotaCount, MailQuotaSize);
		}
		vdl_exit(100);
	} else
	if (ret == -2) {
		strerr_warn1("vdelivermail: temporary system error: ", &strerr_sys);
		vdl_exit(111);
	} else
	if (ret == -3) { /* mail is looping */
		strerr_warn2(TheDir.s, " is looping", 0);
		vdl_exit(100);
	} else
	if (ret == -5) { /*- Defer Overquota mails */
		if (!stralloc_copys(&tmpbuf, user) ||
				!stralloc_append(&tmpbuf, "@") ||
				!stralloc_cats(&tmpbuf, domain) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
		quota_message(tmpbuf.s, MsgSize, MailQuotaCount, MailQuotaSize);
		vdl_exit(111);
	}
	return (0);
}

int
reject_mail(char *user, char *domain, int status, mdir_t MsgSize, char *bounce)
{
	int             ret, i;
	static stralloc _bounce = {0};
	mdir_t          MailQuotaSize, MailQuotaCount;
	char           *ptr;

	/*-
	 * the indimail user does not exist. Follow the rest of 
	 * the directions in the .qmail-default file
	 *
	 * If they want to delete email for non existent users
	 * then just exit 0. Qmail will delete the email for us
	 */
	if (!str_diff(bounce, DELETE_ALL))
		vdl_exit(0);
	else
	if (!str_diff(bounce, BOUNCE_ALL)) {
		if (status == 0)
			strerr_warn5("account ", user, "@", domain, " is inactive. indimail(#5.1.3)", 0);
		else
		if (status == 1)
			strerr_warn5("no account ", user, "@", domain," here by that name. indimail (#5.1.5)", 0);
		vdl_exit(100);
	}
	/*-
	 * Last case: the last parameter is a Maildir, an email address, ipaddress or hostname
	 */
	if (status == 0)
		strerr_warn7("account ", user, "@", domain, " is inactive delivering to ", bounce, ". indimail(#5.1.7) ", 0);
	else
	if (status == 1)
		strerr_warn7("no account ", user, "@", domain, " - delivering to ", bounce, ". indimail (#5.1.6) ", 0);
	if (!stralloc_copys(&_bounce, bounce))
		die_nomem();
	i = str_chr(bounce, '@');
	if (*bounce != '/' && !bounce[i]) { /*- IP Address */
		if (status == 0)
			vdl_exit(100);
		for (i = 0, ptr = bounce; *ptr; ptr++) {
			if (*ptr == ':')
				i++;
		}
		if (i != 2) {
			strerr_warn3("Invalid SMTPROUTE Specification [", bounce, "]. indimail (#5.1.8)", 0);
			vdl_exit(111);
		}
		if (!env_put2("SMTPROUTE", bounce))
			strerr_die4sys(111, FATAL, "env_put2: SMTPROUTE=", bounce, ": ");
		switch (qmail_remote(user, domain))
		{
		case -1:
			strerr_warn4(user, "@", domain, " has insufficient quota. indimail (#5.1.4)", 0);
			vdl_exit(100);
			break;
		case -2:
			strerr_warn1("vdelivermail: temporary system error: ", &strerr_sys);
			vdl_exit(111);
			break;
		case 0:
			vdl_exit(0);
			break;
		case 100:
			vdl_exit(100);
			break;
		case 111:
			vdl_exit(111);
			break;
		} /*- switch() */
	} else /*- check if it is a path add the /Maildir/ for delivery */
	if (*bounce == '/' && !str_str(bounce, "/Maildir/")) {
		if (!stralloc_catb(&_bounce, "/Maildir/", 9))
			die_nomem();
	}
	if (!stralloc_0(&_bounce))
		die_nomem();
	/*- Send the email out, if we get a -1 then the user is over quota */
	ret = deliver_mail(_bounce.s, MsgSize, "AUTO", indimailuid, indimailgid, domain, &MailQuotaSize, &MailQuotaCount);
	if (ret == -1 || ret == -4) {
		if (ret == -1)
			quota_message(_bounce.s, MsgSize, MailQuotaCount, MailQuotaSize);
		vdl_exit(100);
	} else
	if (ret == -2) {
		strerr_warn1("vdelivermail: system error: ", &strerr_sys);
		vdl_exit(111);
	} else
	if (ret == -3) { /* mail is looping */
		strerr_warn2(_bounce.s, " is looping", 0);
		vdl_exit(100);
	} else
	if (ret == -5) { /*- Defer Overquota mails */
		quota_message(_bounce.s, MsgSize, MailQuotaCount, MailQuotaSize);
		vdl_exit(111);
	}
	return (0);
}

static int
bhfcheck(char *addr)
{
	char           *ptr, *mapbhf, *sysconfdir, *controldir;
	int             t, i, j, fd, count, k = 0;
	struct stat     statbuf;
	char            subvalue;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/blackholedsender", 17) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		if (!stralloc_copys(&tmpbuf, sysconfdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/blackholedsender", 17) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	if (!stat(tmpbuf.s, &statbuf)) {
		if (!(mapbhf = (char *) alloc(statbuf.st_size))) {
			strnum[fmt_ulong(strnum, statbuf.st_size)] = 0;
			strerr_warn3("vdelivermail: alloc: ", strnum, " bytes: ", &strerr_sys);
			vdl_exit(111);
		}
		if ((fd = open_read(tmpbuf.s)) == -1) {
			strerr_die3sys(111, "vdelivermail: open: ", tmpbuf.s, ": ");
			alloc_free(mapbhf);
			vdl_exit(111);
		}
		if ((count = read(fd, mapbhf, statbuf.st_size)) != statbuf.st_size) {
			strerr_warn3("vdelivermail: read: ", strnum, " bytes: ", &strerr_sys);
			alloc_free(mapbhf);
			close(fd);
			vdl_exit(111);
		}
		close(fd);
		for (ptr = mapbhf; ptr < mapbhf + statbuf.st_size; ptr++) {
			if (*ptr == '\n')
				*ptr = 0;
		}
		i = 0;
		for (j = 0; j < count; ++j) {
			if (!mapbhf[j]) {
				if (mapbhf[i]) { /*- Not a blank line */
					if (mapbhf[i] == '@') {
						t = str_chr(addr, '@');
						if (!addr[t]) {
							alloc_free(mapbhf);
							return (0);
						} else
							ptr = addr + t;
					} else
						ptr = addr;
					if (!str_diffn(ptr, mapbhf + i, str_len(mapbhf + i))) {
						alloc_free(mapbhf);
						return (1);
					}
				}
				i = j + 1;
			}
		}
		alloc_free(mapbhf);
		return (0);
	}
	if (*controldir == '/') {
		if (!stralloc_copys(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/blackholedpatterns", 19) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		if (!stralloc_copys(&tmpbuf, sysconfdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, controldir) ||
				!stralloc_catb(&tmpbuf, "/blackholedpatterns", 19) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	if (!stat(tmpbuf.s, &statbuf)) {
		if (!(mapbhf = (char *) alloc(statbuf.st_size))) {
			strnum[fmt_ulong(strnum, statbuf.st_size)] = 0;
			strerr_warn3("vdelivermail: alloc: ", strnum, " bytes: ", &strerr_sys);
			vdl_exit(111);
		}
		if ((fd = open_read(tmpbuf.s)) == -1) {
			strerr_die3sys(111, "vdelivermail: open: ", tmpbuf.s, ": ");
			vdl_exit(111);
		}
		if ((count = read(fd, mapbhf, statbuf.st_size)) != statbuf.st_size) {
			strerr_warn3("vdelivermail: read: ", strnum, " bytes: ", &strerr_sys);
			alloc_free(mapbhf);
			close(fd);
			vdl_exit(111);
		}
		close(fd);
		for (ptr = mapbhf; ptr < mapbhf + statbuf.st_size; ptr++) {
			if (*ptr == '\n')
				*ptr = 0;
		}
		i = 0;
		for (j = 0; j < count; ++j) {
			if (!mapbhf[j]) { /*- Not a blank line */
				if (mapbhf[i]) {
					subvalue = mapbhf[i] != '!';
					if (!subvalue)
						i++;
					if ((k != subvalue) && wildmat(addr, mapbhf + i))
						k = subvalue;
				}
				i = j + 1;
			}
		}
		alloc_free(mapbhf);
		return k;
	}
	return (0);
}

/*
 * The email message comes in on file descriptor 0 - stanard in
 * The user to deliver the email to is in the EXT environment variable
 * The domain to deliver the email to is in the HOST environment variable
 *
 * Deliver Exit Codes
 *  exit(0)   - exit successfully and have qmail delete the email 
 *  exit(100) - Bounce Back the email
 *  exit(111) - email remains in the queue.
 */

int
main(int argc, char **argv)
{
	int             MsgSize;
	struct passwd  *pw;
	char           *ptr, *tmp, *addr, *TheUser, *TheDomain, *Bounce;
#ifdef QMAIL_EXT
	char           *TheUserExt;
#endif
#ifdef CLUSTERED_SITE
	static stralloc remoteip = {0}, Email = {0};
	int             ret, len;
	char           *ip, *local_hostid, *remote_hostid;
#endif
#ifdef MAKE_SEEKABLE
	char           *str;
#endif

	if (get_indimailuidgid(&indimailuid, &indimailgid))
		_exit(111);
	/*- get the arguments to the program and setup things */
	TheUser = TheDomain = Bounce = 0;
#ifdef QMAIL_EXT
	TheUserExt = 0;
	get_arguments(argc, argv, &TheUser, &TheDomain, &TheUserExt, &Bounce);
#else
	get_arguments(argc, argv, &TheUser, &TheDomain, 0, &Bounce);
#endif
	signal(SIGPIPE, SIG_IGN);
#ifdef MAKE_SEEKABLE
	if ((str = env_get("MAKE_SEEKABLE")) && *str != '0' && makeseekable(0)) {
		strerr_warn1("vdelivermail: makeseekable: ", &strerr_sys);
		_exit(111);
	}
#endif
	if (!env_unset("SPAMFILTER"))
		strerr_die2sys(111, FATAL, "env_unset: SPAMFILTER: ");
	/*- if we don't know the message size then read it */
	if (!(MsgSize = get_message_size())) {
		strerr_warn1("vdelivermail: discarding 0 size message", 0);
		_exit(0);
	}
	if (!(addr = (char *) env_get("SENDER"))) {
		strerr_warn1("vdelivermail: No SENDER environment variable", 0);
		_exit(100);
	}
	if (bhfcheck(addr)) {
		strerr_warn3("vdelivermail: Discarding BlackHoled Address [", addr, "]", 0);
		_exit(0);
	}
	/*- get the user from indimail database */
	if (!(pw = ((ptr = (char *) env_get("PWSTRUCT"))) ? strToPw(ptr, str_len(ptr) + 1) : sql_getpw(TheUser, TheDomain))) {
		if (!userNotFound) {
			strerr_warn1("vdelivermail: temporary authentication error", 0);
			vdl_exit(111);
		}
#ifdef CLUSTERED_SITE
		/*- Set SMTPROUTE for qmail-remote */
		if ((ret = is_distributed_domain(TheDomain)) == -1) {
			strerr_warn2(TheDomain, ": is_distributed_domain failed", 0);
			vdl_exit(111);
		} else
		if (ret == 1) {
			if (!stralloc_copys(&Email, TheUser) ||
					!stralloc_append(&Email, "@") ||
					!stralloc_cats(&Email, TheDomain) ||
					!stralloc_0(&Email))
				die_nomem();
			if ((ip = findhost(Email.s, 0))) {
				for (ptr = ip; *ptr && *ptr != ':'; ptr++);
				tmp = ptr + 1;
				for (len = 0; *tmp && *tmp != ':';tmp++, len++);
				if (*tmp != ':') {
					strerr_warn3("vdelivermail: findhost: Invalid route [", ip, "]", 0);
					vdl_exit(111);
				}
				if (!stralloc_copyb(&remoteip, ptr + 1, len) || !stralloc_0(&remoteip))
					die_nomem();
				if (!(local_hostid = get_local_hostid())) {
					strerr_warn1("vdelivermail: get_local_hostid: Unable to get hostid", 0);
					vdl_exit(111);
				}
				if (!(remote_hostid = sql_gethostid(remoteip.s))) {
					strerr_warn2("vdelivermail: unable to get hostid for ", remoteip.s, 0);
					vdl_exit(111);
				} else /* avoid looping of mails */
				if (str_diff(remote_hostid, local_hostid)) {
					if ((ptr = env_get("ROUTES")) && (*ptr && !byte_diff(ptr, 4, "qmtp")))
						ptr = "QMTPROUTE";
					else
						ptr = "SMTPROUTE";
					if (!env_put2(ptr, ip))
						strerr_die5sys(111, FATAL, "env_put2: ", ptr, "=", ip);
					switch (qmail_remote(TheUser, TheDomain))
					{
					case -1:
						strerr_warn4(TheUser, "@", TheDomain, " has insufficient quota. indimail (#5.1.4)", 0);
						vdl_exit(100);
						break;
					case -2:
						strerr_warn1("vdelivermail: system error: ", &strerr_sys);
						vdl_exit(111);
						break;
					case 0:
						vdl_exit(0);
						break;
					case 100:
						vdl_exit(100);
						break;
					case 111:
						vdl_exit(111);
						break;
					} /*- switch() */
				}
			} else
			if (!userNotFound) {
				strerr_warn1("vdelivermail: temporary authentication error", 0);
				vdl_exit(111);
			}
		}	  /*- if (is_distributed()) */
#endif
#ifdef QMAIL_EXT
		/*-
		 * try and find user that matches the QmailEXT address if no user found, 
		 * and the QmailEXT address is different, meaning there was an extension 
		 */
		if (env_get("QMAIL_EXT") && str_diff(TheUser, TheUserExt))
			pw = sql_getpw(TheUserExt, TheDomain);
#endif
	} /*- if (!(pw = sql_getpw(TheUser, TheDomain))) */
	if (pw) {
		/*-
		 * Mail delivery has happened successfully
		 * Do not check the return status of 
		 * vset_lastauth * as that will result
		 * in duplicate mails in case
		 * of vset_lastauth() error
		 */
#ifdef ENABLE_AUTH_LOGGING
		if (env_get("ALLOW_INACTIVE")) {
			processMail(pw, pw->pw_name, TheDomain, MsgSize);
			if (!str_diffn(pw->pw_gecos, "MailGroup ", 10))	   /*- Prevent groups getting inactive */
				vset_lastauth(TheUser, TheDomain, "deli", "", pw->pw_gecos + 10, 0);
		} else {
			if (is_inactive)
				reject_mail(pw->pw_name, TheDomain, 0, MsgSize, Bounce);
			else {
				processMail(pw, pw->pw_name, TheDomain, MsgSize);
				if (!str_diffn(pw->pw_gecos, "MailGroup ", 10))	   /*- Prevent groups getting inactive */
					vset_lastauth(TheUser, TheDomain, "deli", "", pw->pw_gecos + 10, 0);
			}
		}
#else
		if (is_inactive)
			reject_mail(pw->pw_name, TheDomain, 0, MsgSize, Bounce);
		else
			processMail(pw, pw->pw_name, TheDomain, MsgSize);
#endif
	} else {
#ifdef VALIAS
		/*- process valiases if configured */
		if (!process_valias(TheUser, TheDomain, MsgSize))
			reject_mail(TheUser, TheDomain, 1, MsgSize, Bounce);
#else
		reject_mail(TheUser, TheDomain, 1, MsgSize, Bounce);
#endif
	}
	iclose();
	_exit(0);
}
