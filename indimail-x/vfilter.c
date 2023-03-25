/*-
 * $Log: vfilter.c,v $
 * Revision 1.11  2023-03-26 00:33:30+05:30  Cprogrammer
 * fixed code using wait_handler
 *
 * Revision 1.10  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.9  2022-12-18 19:28:05+05:30  Cprogrammer
 * recoded wait logic
 *
 * Revision 1.8  2022-05-10 20:01:51+05:30  Cprogrammer
 * use headers from include path
 *
 * Revision 1.7  2021-09-12 20:17:58+05:30  Cprogrammer
 * moved replacestr to libqmail
 *
 * Revision 1.6  2021-07-27 18:07:39+05:30  Cprogrammer
 * set default domain using vset_default_domain
 *
 * Revision 1.5  2021-06-11 17:01:55+05:30  Cprogrammer
 * replaced Makeargs(), makeseekable() with makeargs(), mktempfile() from libqmail
 *
 * Revision 1.4  2020-04-01 18:58:43+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.3  2019-06-07 15:52:44+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:17:07+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-20 08:59:26+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter.c,v 1.11 2023-03-26 00:33:30+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_FNMATCH_H
#define _GNU_SOURCE
#include <fnmatch.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <subfd.h>
#include <substdio.h>
#include <case.h>
#include <env.h>
#include <str.h>
#include <fmt.h>
#include <error.h>
#include <sgetopt.h>
#include <replacestr.h>
#include <makeargs.h>
#include <evaluate.h>
#include <wait.h>
#endif
#include "common.h"
#include "get_real_domain.h"
#include "get_message_size.h"
#include "remove_quotes.h"
#include "eps.h"
#include "storeHeader.h"
#include "deliver_mail.h"
#include "sql_getpw.h"
#include "variables.h"
#include "parse_email.h"
#include "lowerit.h"
#include "vfilter_header.h"
#include "vfilter_select.h"
#include "vfilter_display.h"
#include "getAddressBook.h"
#include "addressToken.h"
#include "mktempfile.h"
#include "vset_default_domain.h"

#define FATAL   "vfilter: fatal: "
#define WARN    "vfilter: warning: "

int             interactive;
static char    *usage =
	"usage: vfilter [options] emailid]\n"
	"options: -V ( print version number )\n"
	"         -v ( verbose )"
	;

static void
die_nomem()
{
	strerr_warn1("vfilter: out of memory", 0);
	_exit(111);
}

static void
printBounce(char *bounce)
{
	char           *ptr, *user, *domain;

	if (str_diffn(bounce, BOUNCE_ALL, str_len(BOUNCE_ALL) + 1)) {
		subprintfe(subfdout, "vfilter", "Hi. This is the IndiMail MDA for %s\n",
				(ptr = env_get("HOST")) ? ptr : vset_default_domain());
		subprintfe(subfdout, "vfilter",
				"I'm afraid I cannot accept your message as a configured filter has decided\n"
				"to reject this mail\n"
				"Please refrain from sending such mail in future\n");
	} else {
		/*- get the last parameter in the .qmail-default file */
		if (!(ptr = env_get("EXT")))
			user = "<>";
		else
			user = ptr;
		if (!(ptr = env_get("HOST")))
			domain = (ptr = env_get("DEFAULT_DOMAIN")) ? ptr : DEFAULT_DOMAIN;
		else
			domain = ptr;
		subprintfe(subfdout, "vfilter", "No Account %s@%s here by that name. indimail (#5.1.5)", user, domain);
	}
	flush("vfilter");
	return;
}

int
execMda(char **argptr, char **mda)
{
	char           *x;
	char          **argv;
	char           *(vdelargs[]) = { PREFIX"/sbin/vdelivermail", "''", BOUNCE_ALL, 0};

	*mda = *vdelargs;
	if ((x = env_get("MDA"))) {
		if (!(argv = makeargs(x)))
			die_nomem();
		*mda = *argv;
		if (*argv[0] != '/' && *argv[0] != '.')
			execvp(*argv, argv);
		else
			execv(*argv, argv);
		strerr_warn3("vfilter: execv: ", *argv, ": ", &strerr_sys);
	} else {
		execv(*vdelargs, argptr);
		strerr_warn3("vfilter: execv: ", *vdelargs, ": ", &strerr_sys);
	}
	return (1);
}

static char     strnum1[FMT_ULONG], strnum2[FMT_ULONG];

/*
 * status value :
 *  -1 : Filter failure - pass it to vdelivermail
 *   0 : Passed through the filter
 * 111 : Temporary Error
 *   n : Message has been filtered
 */
static int
myExit(int argc, char **argv, int status, int bounce, char *DestFolder, char *forward)
{
	char           *revision = "$Revision: 1.11 $", *mda;
	static stralloc XFilter = {0};
	pid_t           pid;
	int             i, werr, wait_status, _status;

	_status = status;
	if (interactive)
		_exit(_status ? 1 : 0);
	if (!stralloc_copyb(&XFilter, "XFILTER=X-Filter: xFilter/IndiMail Revision ", 44) ||
			!stralloc_cats(&XFilter, revision + 11))
		die_nomem();
	i = str_rchr(XFilter.s, '$');
	if (XFilter.s[i])
		XFilter.len--;
	if (!stralloc_catb(&XFilter, "(http://www.indimail.org)", 25) ||
			!stralloc_0(&XFilter))
		die_nomem();
	if (!env_put2("XFILTER", XFilter.s))
		strerr_die4sys(111, FATAL, "env_put2: XFILTER=", XFilter.s, ": ");
	if (_status == 111)
		_exit(_status);
	else /*- Mail has been matched by filter */
	if (_status > 0) {
		if (DestFolder && *DestFolder && case_diffb(DestFolder, 10, "/NoDeliver")) {
			switch (pid = vfork())
			{
			case -1:
				_status = -1;
				break;
			case 0:
				if (case_diffb(DestFolder, 5, "Inbox") && !env_put2("MAILDIRFOLDER", DestFolder))
					strerr_die4sys(111, FATAL, "env_put2: MAILDIRFOLDER=", DestFolder, ": ");
				execMda(argv, &mda);
				_exit(111);
			default: /*- parent */
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
						strerr_warn3("vfilter: waitpid: ", strnum1, ": ", &strerr_sys);
						_status = -1;
					}
					if (!(i = wait_handler(wait_status, &werr)) && werr)
						continue;
					else
					if (werr == -1) {
						strnum1[fmt_ulong(strnum1, pid)] = 0;
						strerr_warn3("vfilter: ", strnum1, ": internal wait handler error", 0);
						_status = -1;
					} else
					if (werr) {
						strnum1[fmt_ulong(strnum1, pid)] = 0;
						strnum2[fmt_uint(strnum2, werr)] = 0;
						strerr_warn4("vfilter: ", strnum1, ": killed by signal ", strnum2, 0);
						_status = -1;
					}
					if (i)
						_status = -1;
					break;
				} /*- for (;;) */
				break;
			} /*- switch (pid = vfork()) */
		} /*- if (case_diffb(DestFolder, 10, "/NoDeliver")) */
		if (_status > 0) {
			if (bounce > 0) {
				if (bounce == 2 || bounce == 3) {
					if (interactive && verbose) {
						subprintfe(subfdout, "vfilter", "Delivering to %s\n", forward);
						flush("vfilter");
					}
					i = deliver_mail(forward, 0, "NOQUOTA", 0, 0, DEFAULT_DOMAIN, 0, 0);
					if (i == -1 || i == -4) {
						if (i == -1) /*- this can never happen because we passed NOQUOTA */
							;
						_exit(100);
					} else
					if (i == -2) {
						strerr_warn1("vfilter: system error: ", &strerr_sys);
						_exit(111);
					} else
					if (i == -3) { /* mail is looping */
						strerr_warn3("vfilter: address ", forward, " is looping", 0);
						_exit(100);
					} else
					if (i == -5) /*- Defer Overquota mails */
						; /*- we disregard qutoas */
				}
				if (bounce == 1 || bounce == 3) {
					printBounce(argv[2]);
					_exit(100);
				}
			}
			if (DestFolder && *DestFolder && !case_diffb(DestFolder, 10, "/NoDeliver")) {
				subprintfe(subfdout, "vfilter", "Mail BlackHoled\n");
				flush("vfilter");
			}
			_exit(0);
		}
	} /*- if (_status > 0) */
	/*- Mail has passed through the filter or filter failure */
	execMda(argv, &mda);
	_exit(111);
	return (0);/*- Not reached */
}

static int
get_options(int argc, char **argv, char **bounce, stralloc *emailid, stralloc *user, stralloc *domain, stralloc *Maildir)
{
	int             c, local;
	char           *tmpstr, *real_domain = 0;
	static stralloc pwstruct = {0};
	struct passwd  *pw;

	local = 0;
	*bounce = 0;
	if (argc == 3 && str_diffn(argv[1], "-v", 3) && str_diffn(argv[1], "-V", 3)) {
		*bounce = argv[2];
		/*- get the last parameter in the .qmail-default file */
		if (!(tmpstr = env_get("EXT"))) {
			strerr_warn1("vfilter: No EXT environment variable", 0);
			myExit(argc, argv, 100, 1, 0, 0);
		} else {
			if (*tmpstr) {
				if (!stralloc_copys(user, tmpstr) || !stralloc_0(user))
					die_nomem();
				user->len--;
			} else {
				if (!(tmpstr = env_get("LOCAL"))) {
					strerr_warn1("vfilter: No LOCAL environment variable", 0);
					myExit(argc, argv, 100, 1, 0, 0);
				} else {
					if (!stralloc_copys(user, tmpstr) || !stralloc_0(user))
						die_nomem();
					user->len--;
					local = 1;
				}
			}
		}
		if (local) {
			tmpstr = vset_default_domain();
			if (!stralloc_copys(domain, tmpstr) || !stralloc_0(domain))
				die_nomem();
			domain->len--;
		} else {
			if (!(tmpstr = env_get("HOST"))) {
				strerr_warn1("vfilter: No HOST environment variable", 0);
				myExit(argc, argv, 100, 1, 0, 0);
			} else {
				if (!stralloc_copys(domain, tmpstr) || !stralloc_0(domain))
					die_nomem();
				domain->len--;
			}
		}
		tmpstr = user->s;
		if (remove_quotes(&tmpstr)) {
			strerr_warn3("vfilter: Invalid user [", user->s, "]", 0);
			myExit(argc, argv, 100, 1, 0, 0);
		}
		if (tmpstr != user->s) {
			if (!stralloc_copys(user, tmpstr) || !stralloc_0(user))
				die_nomem();
			user->len--;
		}
		lowerit(user->s);
		lowerit(domain->s);
		if (!(real_domain = get_real_domain(domain->s))) {
			if (userNotFound)
				myExit(argc, argv, 100, 1, 0, 0);
			myExit(argc, argv, 111, 1, 0, 0);
		}
		if (!stralloc_copy(emailid, user) ||
				!stralloc_append(emailid, "@") ||
				!stralloc_cats(emailid, real_domain) ||
				!stralloc_0(emailid))
			die_nomem();
		emailid->len--;
	} else
	if (argc == 11)	{ /*- qmail-local */
		if (!stralloc_copys(user, argv[6]) || !stralloc_0(user))
			die_nomem();
		user->len--;
		if (!stralloc_copys(domain, argv[7]) || !stralloc_0(domain))
			die_nomem();
		domain->len--;
		lowerit(user->s);
		lowerit(domain->s);
		if (!(real_domain = get_real_domain(domain->s)))
			real_domain = domain->s;
		if (!stralloc_copy(emailid, user) ||
				!stralloc_append(emailid, "@") ||
				!stralloc_cats(emailid, real_domain) ||
				!stralloc_0(emailid))
			die_nomem();
		emailid->len--;
	} else
	if (argc == 2 || (argc > 1 && (!str_diffn(argv[1], "-v", 3) || !str_diffn(argv[1], "-V", 3)))) {
		/*
		 * Procedure for manually testing vfilter
		 *
		 * cat full_path_of_an_existing_email_file | vfilter -v user@domain.com
		 * cat full_path_of_an_existing_email_file | vfilter user@domain.com
		 *
		 */
		interactive = 1;
		while ((c = getopt(argc, argv, "v")) != opteof)
		{
			switch (c)
			{
			case 'v':
				verbose = 1;
				break;
			default:
				strerr_warn2(WARN, usage, 0);
				return (1);
			}
		}
		if (optind < argc) {
			if (!stralloc_copys(user, argv[optind++]) || !stralloc_0(user))
				die_nomem();
			user->len--;
		}
		if (!emailid->len) {
			strerr_warn1("vfilter: must supply emailid", 0);
			strerr_warn2(WARN, usage, 0);
			return (1);
		}
		user->len = domain->len = 0;
		parse_email(emailid->s, user, domain);
		if (!(real_domain = get_real_domain(domain->s)))
			real_domain = domain->s;
	} else {
		strerr_warn1("vfilter: Invalid number of arguments", 0);
		myExit(argc, argv, 111, 0, 0, 0);
	}
	if (!(pw = (local ? getpwnam(user->s) : sql_getpw(user->s, real_domain)))) {
		if (userNotFound) {
			if (interactive) {
				strerr_warn5("vfilter: ", user->s, "@", real_domain, ": No such user", 0);
				return (1);
			}
			if (!stralloc_copyb(&pwstruct, "No such user ", 13) ||
					!stralloc_cat(&pwstruct, user) ||
					!stralloc_append(&pwstruct, "@") ||
					!stralloc_cats(&pwstruct, real_domain) ||
					!stralloc_0(&pwstruct))
				die_nomem();
			if (!env_put2("PWSTRUCT", pwstruct.s)) {
				strerr_warn3("vfilter: env_put2: PWSTRUCT=", pwstruct.s, ": ", &strerr_sys);
				if (interactive)
					return (1);
				_exit(111);
			}
			myExit(argc, argv, -1, 0, 0, 0);
		} else {
			strerr_warn1("vfilter: temporary database error", 0);
			if (interactive)
				return (1);
			myExit(argc, argv, 111, 0, 0, 0);
		}
	}
	if (!stralloc_copys(&pwstruct, pw->pw_name) ||
			!stralloc_append(&pwstruct, "@") ||
			!stralloc_cats(&pwstruct, real_domain) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, pw->pw_passwd) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_catb(&pwstruct, strnum1, fmt_uint(strnum1, pw->pw_uid)) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_catb(&pwstruct, strnum1, fmt_uint(strnum1, pw->pw_gid)) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, pw->pw_gecos && *pw->pw_gecos ? pw->pw_gecos : pw->pw_name) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, pw->pw_dir) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_cats(&pwstruct, local ? "NOQUOTA" : pw->pw_shell) ||
			!stralloc_append(&pwstruct, ":") ||
			!stralloc_catb(&pwstruct, strnum1, fmt_uint(strnum1, local ? 0 : is_inactive)) ||
			!stralloc_0(&pwstruct))
		die_nomem();
	if (!env_put2("PWSTRUCT", pwstruct.s)) {
		strerr_warn3("vfilter: env_put2: PWSTRUCT=", pwstruct.s, ": ", &strerr_sys);
		if (interactive)
			return (1);
		_exit(111);
	}
	if (!stralloc_copys(Maildir, pw->pw_dir) ||
			!stralloc_catb(Maildir, "/Maildir/", 9) ||
			!stralloc_0(Maildir))
		die_nomem();
	Maildir->len--;
	return (0);
}

int
numerical_compare(char *data, char *expression)
{
	char           *ptr;
	int             i;
	static stralloc buf = {0};
	struct val      result;
	struct vartable *vt;

	/*
	 * replace all occurences of %p in expression
	 * with the value of data
	 */
	if (!(vt = create_vartable()))
		return (0);
	buf.len = 0;
	if ((i = replacestr(expression, "%p", data, &buf)) == -1)
		die_nomem();
	ptr = i ? buf.s : expression;
	switch (evaluate(ptr, &result, vt))
	{
	case ERROR_SYNTAX:
		if (interactive && verbose) {
			subprintfe(subfdout, "vfilter", "syntax error\n");
			flush("vfilter");
		}
		return (-1);
	case ERROR_VARNOTFOUND:
		if (interactive && verbose) {
			subprintfe(subfdout, "vfilter", "variable not found\n");
			flush("vfilter");
		}
		return (-1);
	case ERROR_NOMEM:
		if (interactive && verbose) {
			subprintfe(subfdout, "vfilter", "not enough memory\n");
			flush("vfilter");
		}
		return (-1);
	case ERROR_DIV0:
		if (interactive && verbose) {
			subprintfe(subfdout, "vfilter", "division by zero\n");
			flush("vfilter");
		}
		return (-1);
	case RESULT_OK:
		if (result.type == T_INT) {
			if (interactive && verbose) {
				subprintfe(subfdout, "vfilter", "result = %ld\n", result.ival);
				flush("vfilter");
			}
			return (result.ival);
		} else {
			if (interactive && verbose) {
				subprintfe(subfdout, "vfilter", "result = %.2f\n", result.rval);
				flush("vfilter");
			}
			return (0);
		}
	}
	free_vartable(vt);
	return (0);
}

static void
process_filter(int argc, char **argv, struct header **hptr, char *filterid, int *filter_no,
		stralloc *filter_name, int *header_name, int *comparision, stralloc *keyword,
		stralloc *folder, int *bounce_action, stralloc *forward)
{
	int             i, j, email_len, filter_opt, ret = 0, global_filter = 0,
					max_header_value;
	char           *str, *real_domain;
	char          **tmp_ptr, **address_list;
	static char   **header_list;
	struct header **ptr;
	static stralloc tmpUser = {0}, tmpDomain = {0};

	if (interactive && verbose) {
		subprintfe(subfdout, "vfilter", "Processing Filter %s\n", filterid);
		flush("vfilter");
	}
	if (!str_diffn(filterid, "prefilt@", 8) || !str_diffn(filterid, "postfilt@", 9))
		global_filter = 1;
	if (!header_list && !(header_list = headerList()))
		header_list = vfilter_header;
	for (max_header_value = 0; header_list[max_header_value]; max_header_value++);
	max_header_value--;
	for (j = 0;;) {
		i = vfilter_select(filterid, filter_no, filter_name, header_name, comparision,
			keyword, folder, bounce_action, forward);
		if (i == -1) {
			strerr_warn1("vfilter_select: failure", 0);
			break;
		} else
		if (i == -2)
			break;
		if (interactive && verbose && !j++) {
			if (global_filter) {
				subprintfe(subfdout, "vfilter",
							"No  global Filter                 "
							"FilterName Header          "
							"Comparision                "
							"Keyword         Folder          Action\n");
			} else {
				subprintfe(subfdout, "vfilter",
							"No  EmailId                       "
							"FilterName Header          "
							"Comparision                "
							"Keyword         Folder          "
							"Action\n");
			}
			print_hyphen(subfdout, "=", 137);
		}
		if (interactive && verbose)
			format_filter_display(0, *filter_no, filterid, filter_name, *header_name, *comparision,
				keyword, folder, forward, *bounce_action);
		flush("vfilter");
		/*
		 * comparision 5 - Sender not in Address Book
		 * comparision 6 - ID not in To, Cc, Bcc
		 */
		if (!global_filter && (*comparision == 5 || *comparision == 6)) {
			address_list = getAddressBook(filterid);
			if (interactive && verbose)
				print_hyphen(subfdout, "=", 137);
			for (ret = 0, ptr = hptr; ptr && *ptr && !ret; ptr++) {
				if (!case_diffb((*ptr)->name, 4, "From") || !case_diffb((*ptr)->name, 11, "Return-Path"))
					filter_opt = 1;
				else
				if (!case_diffb((*ptr)->name, 2, "To") || !case_diffb((*ptr)->name, 2, "Cc") ||
					!case_diffb((*ptr)->name, 3, "Bcc"))
					filter_opt = 2;
				else
					continue;
				for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr && !ret; tmp_ptr++) {
					/*- Sender not in Address Book */
					if (*comparision == 5 && filter_opt == 1) {
						for (rewindAddrToken();;) {
							if (!(str = addressToken(*tmp_ptr, &email_len)))
								break;
							if (!case_diffb(str, email_len + 1, filterid)) {
								/*- Allow the user to send himself mail */
								ret = 1;
								break;
							}
							for (i = 0; address_list && address_list[i]; i++) {
								if (!address_list[i] || !*address_list[i])
									continue;
								if (!case_diffb(str, email_len + 1, address_list[i])) {
									ret = 1;
									break;
								}
							}
						}
					}
					/*- ID not in To, Cc, Bcc */
					if (*comparision == 6 && filter_opt == 2) {
						for (rewindAddrToken();;) {
							if (!(str = addressToken(*tmp_ptr, &email_len)))
								break;
							if (!case_diffb(str, email_len + 1, filterid)) {
								ret = 1;
								break;
							}
							parse_email(str, &tmpUser, &tmpDomain);
							if (!(real_domain = get_real_domain(tmpDomain.s)))
								real_domain = tmpDomain.s;
							if (!stralloc_append(&tmpUser, "@") ||
									!stralloc_cats(&tmpUser, real_domain) ||
									!stralloc_0(&tmpUser))
								die_nomem();
							if (!case_diffb(tmpUser.s, tmpUser.len + 1, filterid)) {
								ret = 1;
								break;
							}
						}
					}
				} /*- for(tmp_ptr = (*ptr)->data;tmp_ptr && *tmp_ptr && !ret;tmp_ptr++) */
				flush("vfilter");
			} /*- for(ret = 0, ptr = hptr;ptr && *ptr && !ret;ptr++) */
			if (!ret) {
				if (interactive && verbose) {
					subprintfe(subfdout, "vfilter", "Matched Filter No %d Comparision %s\n", *filter_no, vfilter_comparision[*comparision]);
					flush("vfilter");
				}
				myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
			}
		} else
		if (*comparision < 5 || *comparision == 7 || *comparision == 8) {
			/*
			 * 0 - Equals
			 * 1 - Contains
			 * 2 - Does not contain
			 * 3 - Starts with
			 * 4 - Ends with
			 * 7 - Numerical logical expression
			 * 8 - Regular expression
			 */
			if (*header_name > max_header_value) /*- Invalid header value in vfilter table */
				continue;
			lowerit(keyword->s);
			for (ptr = hptr; ptr && *ptr; ptr++) {
				if (!case_diffb((*ptr)->name, MAX_LINE_LENGTH, header_list[*header_name])) {
					switch (*comparision)
					{
					case 0:	/*- Equals */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
							if (!case_diffb(*tmp_ptr, keyword->len, keyword->s)) {
								if (interactive && verbose) {
									subprintfe(subfdout, "vfilter", "Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword->s);
									flush("vfilter");
								}
								myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
							}
						}
						break;
					case 1:	/*- Contains */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
							if (str_str(*tmp_ptr, keyword->s)) {
								if (interactive && verbose) {
									subprintfe(subfdout, "vfilter", "Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword->s);
									flush("vfilter");
								}
								myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
							}
						}
						break;
					case 2:	/*- Does not contain */
						for (ret = 0, tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
							if (str_str(*tmp_ptr, keyword->s)) {
								ret = 1;
								break;
							}
						}
						if (!ret) {
							if (interactive && verbose) {
								subprintfe(subfdout, "vfilter", "Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword->s);
								flush("vfilter");
							}
							myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
						}
						break;
					case 3:	/*- Starts with */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
							if (!str_diffn(*tmp_ptr, keyword->s, keyword->len))
							{
								if (interactive && verbose) {
									subprintfe(subfdout, "vfilter", "Matched Filter No %d Data %s Keyword %s\n", *filter_no, *tmp_ptr, keyword->s);
									flush("vfilter");
								}
								myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
							}
						}
						break;
					case 4:	/*- Ends with */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
							if ((str = str_str(*tmp_ptr, keyword->s)) && !case_diffb(str, keyword->len, keyword->s)) {
								if (interactive && verbose) {
									subprintfe(subfdout, "vfilter", "Matched Filter No %d Comparision %s Data %s Keyword %s\n",
											*filter_no, vfilter_comparision[*comparision], *tmp_ptr, keyword->s);
									flush("vfilter");
								}
								myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
							}
						}
						break;
					case 7:	/*- Float */
						/*- e.g. tmp_ptr = 0.7, keyword = %p < 0.4 */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
							if (numerical_compare(*tmp_ptr, keyword->s)) {
								if (interactive && verbose) {
									subprintfe(subfdout, "vfilter", "Matched Filter No %d Data %s Keyword %s Folder %s\n",
											*filter_no, *tmp_ptr, keyword->s, folder->s);
									flush("vfilter");
								}
								myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
							}
						}
						break;
#ifdef HAVE_FNMATCH
					case 8:	/*- Regular Expressions */
						for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
#ifdef FNM_CASEFOLD
							if (!fnmatch(keyword->s, *tmp_ptr, FNM_PATHNAME | FNM_CASEFOLD))
#else
							if (!fnmatch(keyword->s, *tmp_ptr, FNM_PATHNAME))
#endif
							{
								if (interactive && verbose) {
									subprintfe(subfdout, "vfilter", "Matched Filter No %d Comparision %s Keyword %s\n",
											*filter_no, vfilter_comparision[*comparision], keyword->s);
									flush("vfilter");
								}
								myExit(argc, argv, 1, *bounce_action, folder->s, forward->s);
							}
						}
						break;
#endif
					} /*- switch(comparision) */
				} /*- if (!case_diffb((*ptr)->name, MAX_LINE_LENGTH, header_list[header_name])) */
			} /*- for(ptr = hptr;ptr && *ptr;ptr++) */
		} /*- if (comparision < 5 || comparision == 7 || comparision == 8) */
	} /*- for (j = 0;;) */
	return;
}

int
main(int argc, char **argv)
{
	unsigned char  *l = NULL;
	char           *str, *bounce;
	char          **tmp_ptr;
	struct header **hptr, **ptr;
	struct eps_t   *eps = NULL;
	struct header_t *h = NULL;
	static stralloc emailid = {0}, user = {0}, domain = {0}, Maildir = {0}, filter_name = {0},
					keyword = {0}, folder = {0}, filterid = {0}, forward = {0}, tmpFlag = {0};
	int             i, ret = 0, fd = 0, header_name, comparision, bounce_action, filter_no, MsgSize;

	if (!(MsgSize = get_message_size())) {
		subprintfe(subfdout, "vfilter", "Discarding 0 size message\n");
		flush("vfilter");
		_exit(0);
	}
	if (get_options(argc, argv, &bounce, &emailid, &user, &domain, &Maildir))
		myExit(argc, argv, -1, 0, 0, 0);
#ifdef MAKE_SEEKABLE
	if ((str = env_get("MAKE_SEEKABLE")) && *str != '0' && mktempfile(0))
		myExit(argc, argv, -1, 0, 0, 0);
#endif
	if (!(eps = eps_begin(INTERFACE_STREAM, (int *) &fd))) {
		strerr_warn1("eps_begin: Error", 0);
		myExit(argc, argv, -1, 0, 0, 0);
	}
	for (h = eps_next_header(eps), hptr = (struct header **) 0; h; h = eps_next_header(eps)) {
		if ((h->name) && h->data) {
			storeHeader(&hptr, h);
			if (interactive && verbose) {
				subprintfe(subfdout, "vfilter", "%s: %s\n", (char *) h->name, (char *) h->data);
				flush("vfilter");
			}
		}
		eps_header_free(eps);
	}
	if (interactive && verbose) {
		subprintfe(subfdout, "vfilter", "\n");
		flush("vfilter");
	}
	for (l = eps_next_line(eps); l; l = eps_next_line(eps)) {
		if (interactive && verbose) {
			subprintfe(subfdout, "vfilter", "%s\n", (char *) l);
			flush("vfilter");
		}
	}
	if (interactive && verbose) {
		subprintfe(subfdout, "vfilter", "\n");
		flush("vfilter");
	}
	while ((!(eps->u->b->eof)) && (eps->content_type & CON_MULTI)) {
		if (!(ret = mime_init_stream(eps)))
			break;
		for (h = mime_next_header(eps); h; h = mime_next_header(eps)) {
			if ((h->name) && (h->data)) {
				storeHeader(&hptr, h);
				if (interactive && verbose) {
					subprintfe(subfdout, "vfilter", "%s: %s\n", (char *) h->name, (char *) h->data);
					flush("vfilter");
				}
			}
			header_kill(h);
		}
		if (interactive && verbose) {
			subprintfe(subfdout, "vfilter", "\n");
			flush("vfilter");
		}
		for (l = mime_next_line(eps); l; l = mime_next_line(eps)) {
			if (interactive && verbose) {
				subprintfe(subfdout, "vfilter", "%s\n", (char *) l);
				flush("vfilter");
			}
		}
	}
	eps_end(eps);
	/*- Filter Engine */
	for (ptr = hptr; ptr && *ptr; ptr++) {
		if (verbose && interactive)
			subprintfe(subfdout, "vfilter", "%-25s", (*ptr)->name);
		for (tmp_ptr = (*ptr)->data; tmp_ptr && *tmp_ptr; tmp_ptr++) {
			lowerit(*tmp_ptr);
			if (verbose && interactive) {
				subprintfe(subfdout, "vfilter", "                          -> %s\n", *tmp_ptr);
				flush("vfilter");
			}
		}
	}
	/*- Global prefilt ID Filter */
	i = str_rchr(emailid.s, '@');
	if (emailid.s[i]) {
		if (!stralloc_copyb(&filterid, "prefilt", 7) ||
				!stralloc_cats(&filterid, emailid.s + i) ||
				!stralloc_0(&filterid))
			die_nomem();
		filterid.len--;
	} else {
		if (!stralloc_copyb(&filterid, "prefilt", 7) ||
				!stralloc_0(&filterid))
			die_nomem();
		filterid.len--;
	}
	if (!stralloc_copy(&tmpFlag, &Maildir) ||
			!stralloc_catb(&tmpFlag, "noprefilt", 9) ||
			!stralloc_0(&tmpFlag))
		die_nomem();
	tmpFlag.len--;
	if (access(tmpFlag.s, F_OK))
		process_filter(argc, argv, hptr, filterid.s, &filter_no, &filter_name, &header_name,
			&comparision, &keyword, &folder, &bounce_action, &forward);

	/*- Process User Filter */
	tmpFlag.len -= 9;
	if (!stralloc_catb(&tmpFlag, "vfilter", 7) ||
			!stralloc_0(&tmpFlag))
		die_nomem();
	tmpFlag.len--;
	if (access(tmpFlag.s, F_OK)) {
		if (interactive)
			return (0);
		myExit(argc, argv, 0, 0, 0, 0);
	}
	process_filter(argc, argv, hptr, emailid.s, &filter_no, &filter_name, &header_name, &comparision,
		&keyword, &folder, &bounce_action, &forward);

	/*- Global postfilt ID Filter */
	filterid.len -= 7;
	if (emailid.s[i]) {
		if (!stralloc_copyb(&filterid, "postfilt", 8) ||
				!stralloc_cats(&filterid, emailid.s + i) ||
				!stralloc_0(&filterid))
			die_nomem();
		filterid.len--;
	} else {
		if (!stralloc_copyb(&filterid, "postfilt", 8) ||
				!stralloc_0(&filterid))
			die_nomem();
		filterid.len--;
	}
	tmpFlag.len -= 7;
	if (!stralloc_catb(&tmpFlag, "nopostfilt", 10) ||
			!stralloc_0(&tmpFlag))
		die_nomem();
	tmpFlag.len--;
	if (access(tmpFlag.s, F_OK))
		process_filter(argc, argv, hptr, filterid.s, &filter_no, &filter_name, &header_name,
			&comparision, &keyword, &folder, &bounce_action, &forward);
	if (interactive && verbose) {
		subprintfe(subfdout, "vfilter", "Passed All Filters\n");
		flush("vfilter");
	}
	myExit(argc, argv, 0, 0, 0, 0);
	return (0);/*- Not reached */
}
#else
#include <strerr.h>
int
main(int argc, char **argv)
{
	strerr_warn1("IndiMail not configured with --enable-vfilter=y", 0);
	return (1);
}
#endif /*- #ifdef VFILTER */
