/*
 * $Id: vcfilter.c,v 1.7 2023-09-06 18:47:05+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vcfilter.c,v 1.7 2023-09-06 18:47:05+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <stralloc.h>
#include <strerr.h>
#include <fmt.h>
#include <scan.h>
#include <error.h>
#include <str.h>
#include <open.h>
#include <subfd.h>
#include <setuserid.h>
#endif
#include "common.h"
#include "parse_email.h"
#include "get_real_domain.h"
#include "get_assign.h"
#include "sqlOpen_user.h"
#include "sql_getpw.h"
#include "iclose.h"
#include "vfilter_display.h"
#include "vfilter_header.h"
#include "vfilter_select.h"
#include "vfilter_insert.h"
#include "vfilter_update.h"
#include "vfilter_delete.h"
#include "check_group.h"
#include "isnum.h"
#include "variables.h"
#include "case.h"

#define FATAL   "vcfilter: fatal: "
#define WARN    "vcfilter: warning: "

#define FILTER_SELECT 0
#define FILTER_INSERT 1
#define FILTER_DELETE 2
#define FILTER_UPDATE 3

static char   **header_list;
static int      FilterAction;

void
usage()
{
	int             i;
	subprintfe(subfderr, "vcfilter",
			"usage: vcfilter [options] emailid\n"
			"options: -v verbose\n"
			"         -C connect to Cluster\n"
			"         -r raw display (for -s option)\n"
			"         -s show filter(s)\n"
			"         -i add filter\n"
			"         -d filter_no delete filter(s)\n"
			"         -u filter_no update filter\n"
			"         -t Filter Name (textual description of filter)\n");

	subprintfe(subfderr, "vcfilter", "         -c comparision\n");
	for (i = 0; vfilter_comparision[i];) {
		if (vfilter_comparision[i + 1]) {
			subprintfe(subfderr, "vcfilter", "%11s %3d - %-28s %8s %3d - %s\n", " ", i, vfilter_comparision[i], " ", i + 1, vfilter_comparision[i + 1]);
			i += 2;
		} else {
			subprintfe(subfderr, "vcfilter", "%11s %3d - %s\n", " ", i, vfilter_comparision[i]);
			i++;
		}
	}

	subprintfe(subfderr, "vcfilter", "         -h header value\n"
			"             -1 - If comparision (-c) option is 5\n");
	if (!(header_list = headerList()))
		header_list = vfilter_header;
	for (i = 0; header_list[i];) {
		if (header_list[i + 1]) {
			subprintfe(subfderr, "vcfilter", "%11s %3d - %-28s %8s %3d - %s\n",
					" ", i, header_list[i], " ", i + 1, header_list[i + 1]);
			i += 2;
		} else {
			subprintfe(subfderr, "vcfilter", "%11s %3d - %s\n", " ", i, header_list[i]);
			i++;
		}
	}

	subprintfe(subfderr, "vcfilter",
			"         -k keyword [\"\" string if comparision (-c) option is 5]\n"
			"         -f folder [Specify /NoDeliver for delivery to be junked]\n"
			"         -b bounce action\n"
			"              0               - Do not Bounce to sender\n"
			"              1               - Bounce to sender\n"
			"              2'&user@domain' - Forward to another id\n"
			"              2'|program'     - Feed mail to another program\n"
			"              3'&user@domain' - Forward to another id and Bounce\n"
			"              3'|program'     - Feed mail to another program and Bounce\n");
	flush("vcfilter");
}

static void
die_nomem()
{
	strerr_warn1("vcfilter: out of memory", 0);
	_exit(111);
}

int
get_options(int argc, char **argv, char **email, stralloc *faddr,
		char **filter_name, char **keyword, stralloc *folder, int *header_num,
		int *comparision, int *bounce_action, int *filter_no, int *raw, int *cluster_conn)
{
	int             i, len, c, max_header_num, max_comparision;
	char           *ptr, *ptr1, *ptr2;
	char            strnum[FMT_ULONG];

	*header_num = *comparision = *bounce_action = *filter_no = -1;
	*email = *filter_name = *keyword = 0;
	FilterAction = -1;
	
	*cluster_conn = 0;
	while ((c = getopt(argc, argv, "vCsird:u:h:c:b:k:f:t:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
#ifdef CLUSTERED_SITE
		case 'C':
			*cluster_conn = 1;
			break;
#endif
		case 's':
			FilterAction = FILTER_SELECT;
			break;
		case 'r':
			*raw = 1;
			break;
		case 'i':
			FilterAction = FILTER_INSERT;
			break;
		case 'd':
			FilterAction = FILTER_DELETE;
			if (isnum(optarg))
				scan_uint(optarg, (unsigned int *) filter_no);
			else
				*filter_no = -1;
			break;
		case 'u':
			FilterAction = FILTER_UPDATE;
			if (isnum(optarg))
				scan_uint(optarg, (unsigned int *) filter_no);
			else
				*filter_no = -1;
			break;
		case 'h':
			if (!(header_list = headerList()))
				header_list = vfilter_header;
			for (max_header_num = 0; header_list[max_header_num]; max_header_num++);
			max_header_num--;
			if (isnum(optarg)) {
				scan_uint(optarg, (unsigned int *) header_num);
				if (*header_num < 0 || *header_num > max_header_num) {
					strerr_warn3("vcfilter: header value ", optarg, " out of range", 0);
					usage();
					return (1);
				}
			} else
				*header_num = headerNumber(header_list, optarg);
			break;
		case 'c':
			if (isnum(optarg)) {
				scan_uint(optarg, (unsigned int *) comparision);
			} else {
				len = str_len(optarg);
				*comparision = -1;
				for (i = 0; vfilter_comparision[i]; i++) {
					if (!str_diffn(optarg, vfilter_comparision[i], len + 1)) {
						*comparision = i;
						break;
					}
				}
			}
			for (max_comparision = 0; vfilter_comparision[max_comparision];max_comparision++);
			max_comparision--;
			if (*comparision < 0 || *comparision > max_comparision) {
				strerr_warn3("vcfilter: comparision value ", optarg, " out of range", 0);
				usage();
				return (1);
			}
			break;
		case 'b':
			if (isdigit((int) *optarg)) {
				*bounce_action = *optarg - '0';
				if (*bounce_action != 0 && *bounce_action != 1 && *bounce_action != 2 && *bounce_action != 3) {
					strnum[fmt_uint(strnum, (unsigned int) *bounce_action)] = 0;
					strerr_warn2("vcfilter: invalid bounce action ", strnum, 0);
					usage();
					return (1);
				} else
				if (*bounce_action == 2 || *bounce_action == 3) {
					if ((optarg[1] && optarg[1] != '&' && optarg[1] != '|') || !optarg[1] || !optarg[2]) {
						strerr_warn1("vcfilter: forwarding address incorrect or not specified", 0);
						usage();
						return (1);
					}
					if (!stralloc_copys(faddr, optarg + 1) || !stralloc_0(faddr))
						die_nomem();
				}
			} else
				*bounce_action = -1;
			break;
		case 't':
			*filter_name = optarg;
			break;
		case 'k':
			*keyword = optarg;
			if (*comparision == 6) {
				for (ptr1 = *keyword, ptr2 = *keyword; *ptr1; ptr1++) {
					if (!isspace((int) *ptr1))
						*ptr2++ = *ptr1;
				}
				*ptr2 = 0;
				/* Should call evaluate() here and test */
			}
			break;
		case 'f':
			if (!stralloc_copys(folder, optarg) ||
					!stralloc_0(folder))
				die_nomem();
			folder->len--;
			if (folder->s[0] == '.') {
				strnum[fmt_strn(strnum, folder->s, 1)] = 0;
				strerr_warn5("vcfilter: ", folder->s, ": Folder Name cannot start with [", strnum, "]", 0);
				folder->len = 0;
				return (1);
			} else
			if (case_diffb(folder->s, 10, "/NoDeliver")) {
				for (ptr = folder->s; *ptr; ptr++) {
					if (*ptr == '/') {
						strnum[fmt_strn(strnum, ptr, 1)] = 0;
						strerr_warn5("vcfilter", folder->s, ": Invalid Char [", strnum, "]", 0);
						folder->len = 0;
						return (1);
					}
				}
			}
			break;
		default:
			usage();
			return (1);
		}
	} /*- while ((c = getopt(argc, argv, "vCsirm:d:u:h:c:b:k:f:o:DU")) != -1) */
	if (FilterAction == -1) {
		strerr_warn1("Must specify one of -s -i, -u, -d, -m option", 0);
		usage();
		return (1);
	}
	if (optind < argc)
		*email = argv[optind++];
	if (!*email) {
		strerr_warn1("vcfilter: must supply emailid", 0);
		usage();
		return (1);
	} else {
		for (i = 0;rfc_ids[i];i++) {
			if (!str_diffn(*email, "prefilt", 7) || !str_diffn(*email, "postfilt", 8))
				continue;
			if (!str_diffn(*email, rfc_ids[i], str_len(rfc_ids[i]))) {
				strerr_warn3("vcfilter: email ", *email, " not allowed for filtering", 0);
				usage();
				return (1);
			}
		}
	}
	if (*comparision == 5) {
		*header_num = -1;
		*keyword = 0;
	}
	switch(FilterAction)
	{
	case FILTER_DELETE:
	case FILTER_UPDATE:
	case FILTER_INSERT:
		if (FilterAction == FILTER_INSERT && !*filter_name) {
			strerr_warn1("vcfilter: filter name not specified", 0);
			usage();
			return (1);
		}
		if ((FilterAction == FILTER_UPDATE || FilterAction == FILTER_DELETE) && *filter_no == -1) {
			strerr_warn1("vcfilter: filter no not specified or invalid", 0);
			usage();
			return (1);
		}
		if (FilterAction == FILTER_DELETE)
			break;
		if (*comparision == -1 || *bounce_action == -1 || !folder->len) {
			if (*comparision == -1)
				strerr_warn1("vcfilter: -c option not specified", 0);
			if (*bounce_action == -1)
				strerr_warn1("vcfilter: -b option not specified", 0);
			if (!folder->len)
				strerr_warn1("vcfilter: -f option not specified", 0);
			usage();
			return (1);
		}
		if (*comparision != 5) {
			if (*header_num == -1 || !*keyword) {
				if (*header_num == -1)
					strerr_warn1("vcfilter: -h option not specified", 0);
				if (!*keyword)
					strerr_warn1("vcfilter: -k option not specified", 0);
				usage();
				return (1);
			}
		}
		break;
	} /*- switch(FilterAction) */
	return (0);
}

int
main(int argc, char **argv)
{
	int             i, status = -1, raw = 0, cluster_conn = 0, header_num,
					comparision, bounce_action, filter_no;
	uid_t           uid, uidtmp;
	gid_t           gid, gidtmp;
	struct passwd  *pw;
	char           *real_domain, *emailid, *filter_name, *keyword;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	static stralloc user = {0}, domain = {0}, folder = {0}, vfilter_file = {0}, faddr = {0};

	if (get_options(argc, argv, &emailid, &faddr, &filter_name, &keyword, &folder,
			&header_num, &comparision, &bounce_action, &filter_no, &raw,
			&cluster_conn))
		return (1);
	parse_email(emailid, &user, &domain);
	if (!(real_domain = get_real_domain(domain.s)))
		real_domain = domain.s;
	if (!get_assign(real_domain, 0, &uid, &gid)) {
		strerr_warn3("vcfilter: ", real_domain, ": domain does not exist", 0);
		return (1);
	}
	if (!uid)
		strerr_die3x(100, "vcfilter: domain ", real_domain, " with uid 0");
	if (cluster_conn && sqlOpen_user(emailid, 0) == -1) {
		if (userNotFound) {
			strerr_warn3("vcfilter: ", emailid, ": No such user", 0);
			return (1);
		} else {
			strerr_warn1("vcfilter: temporary database error", 0);
			return (1);
		}
	}
	if (FilterAction != FILTER_SELECT) {
		uidtmp = getuid();
		gidtmp = getgid();
		if (uidtmp != 0 && uidtmp != uid && gidtmp != gid && check_group(gid, FATAL) != 1) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn5("vcfilter: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
			if (cluster_conn)
				iclose();
			return (1);
		}
		if (setuser_privileges(uid, gid, "indimail")) {
			strnum1[fmt_ulong(strnum1, uid)] = 0;
			strnum2[fmt_ulong(strnum2, gid)] = 0;
			strerr_warn5("vcfilter: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
			if (cluster_conn)
				iclose();
			return (1);
		}
	}
	if (FilterAction == FILTER_INSERT || FilterAction == FILTER_DELETE) {
		if (!(pw = sql_getpw(user.s, real_domain))) {
			if (userNotFound)
				strerr_warn5("vcfilter: ", user.s, "@", real_domain, ": No such user", 0);
			else
				strerr_warn1("vcfilter: temporary database error", 0);
			if (cluster_conn)
				iclose();
			return (1);
		}
		if (!stralloc_copys(&vfilter_file, pw->pw_dir) ||
				!stralloc_catb(&vfilter_file, "/Maildir/vfilter", 16) ||
				!stralloc_0(&vfilter_file))
			die_nomem();
	}
	switch (FilterAction)
	{
	case FILTER_SELECT:
		status = vfilter_display(emailid, raw);
		break;
	case FILTER_INSERT:
		status = vfilter_insert(emailid, filter_name, header_num,
			comparision, keyword, folder.s, bounce_action, faddr.s);
		if (!status && str_diffn(emailid, "prefilt@", 8) && str_diffn(emailid, "postfilt@", 9)
			&& access(vfilter_file.s, F_OK)) {
			if ((i = open_trunc(vfilter_file.s)) == -1) {
				strerr_warn3("vcfilter: open_trunc: ", vfilter_file.s, ": ", &strerr_sys);
				status = -1;
			} else {
				close(i);
				if (chown(vfilter_file.s, uid, gid)) {
					strerr_warn3("vcfilter: chown: ", vfilter_file.s, ": ", &strerr_sys);
					status = -1;
				}
			}
		}
		break;
	case FILTER_DELETE:
		/*
		 * int
		 * vfilter_select(char *emailid, int *filter_no, stralloc *filter_name,
		 *		int *header_num, int *comparision, stralloc *keyword,
		 *		stralloc *destination, int *bounce_action, stralloc *forward)
		 */
		if (!(status = vfilter_delete(emailid, filter_no))) {
			if (vfilter_select(emailid, 0, 0, 0, 0, 0, 0, 0, 0) == -2)
				unlink(vfilter_file.s);
		}
		break;
	case FILTER_UPDATE:
		status = vfilter_update(emailid, filter_no, header_num, comparision, keyword, folder.s, bounce_action, faddr.s);
		break;
	}
#ifdef DEBUG
	printf("action %d, header %d keyword [%s] comparision %d folder [%s] bounce_action %d Forward %s email [%s]\n",
			FilterAction, header_num, keyword, comparision, folder, bounce_action, bounce_action == 2 ? faddr.s : "N/A", emailid);
#endif
	if (cluster_conn)
		iclose();
	return (status);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-vfilter=y", 0);
	return (0);
}
#endif

/*
 * $Log: vcfilter.c,v $
 * Revision 1.7  2023-09-06 18:47:05+05:30  Cprogrammer
 * allow both textual and numberical values for -c and -h option
 * removed "Sender not in addressbook"
 *
 * Revision 1.6  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.5  2021-07-08 11:47:52+05:30  Cprogrammer
 * add check for misconfigured assign file
 *
 * Revision 1.4  2020-06-16 17:56:23+05:30  Cprogrammer
 * moved setuserid function to libqmail
 *
 * Revision 1.3  2019-06-07 15:54:31+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:16:58+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-18 08:38:43+05:30  Cprogrammer
 * Initial revision
 *
 */
