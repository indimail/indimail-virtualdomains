/*
 * $Log: vfilter_display.c,v $
 * Revision 1.4  2023-09-11 09:10:04+05:30  Cprogrammer
 * removed row separator for raw display
 *
 * Revision 1.3  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.2  2022-10-08 23:46:27+05:30  Cprogrammer
 * fixed SIGSEGV
 * fixed formatting
 *
 * Revision 1.1  2019-04-18 08:33:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_display.c,v 1.4 2023-09-11 09:10:04+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <strerr.h>
#include <subfd.h>
#endif
#include "common.h"
#include "vfilter_select.h"
#include "vfilter_header.h"
#include "variables.h"

static void
die_nomem()
{
	strerr_warn1("vfilter_display: out of memory", 0);
	_exit(111);
}

void
print_hyphen(substdio *ss, char *c, int num)
{
	int             i;

	for (i = 0; i < num; i++)
		if (substdio_put(ss, c, 1))
			strerr_die1sys(111, "write: Unable to write output: ");
	if (substdio_put(ss, "\n", 1) == -1)
		strerr_die1sys(111, "write: Unable to write output: ");
}

void
format_filter_display(int type, int filter_no, char *emailid, stralloc *filter_name, int header_name, int comparision,
		stralloc *keyword, stralloc *folder, stralloc *forward, int bounce_action)
{
	static stralloc _filterName = {0}, _keyword = {0};
	char           *ptr, *_hname;
	int             max_header_value;
	static char   **header_list;

	if (!type) {
		if (!header_list && !(header_list = headerList()))
			header_list = vfilter_header;
		for (max_header_value = 0; header_list[max_header_value]; max_header_value++);
		if (header_name >= max_header_value)
			_hname = "invalid header";
		else
			_hname = header_list[header_name];
		subprintfe(subfdout, "vfilter", "%3d %-29s %-20s %-15s %-26s %-15s %-15s %-6s %s\n",
				filter_no, emailid, filter_name->s,
				header_name == -1 ? "N/A" : _hname,
				vfilter_comparision[comparision],
				keyword->len ? keyword->s : "N/A",
				!str_diffn(folder->s, "/NoDeliver", 11) ? "Void" : folder->s,
				(bounce_action == 1 || bounce_action == 3) ? "Yes" : "No",
				(bounce_action == 2 || bounce_action == 3) ? forward->s : "No");
	} else { /*- raw display*/
		if (!stralloc_copy(&_filterName, filter_name) || !stralloc_0(&_filterName))
			die_nomem();
		if (!stralloc_copy(&_keyword, keyword) || !stralloc_0(&_keyword))
			die_nomem();
		for (ptr = _filterName.s; *ptr; ptr++) {
			if (isspace((int) *ptr))
				*ptr = '~';
		}
		for (ptr = _keyword.s; *ptr; ptr++) {
			if (isspace((int) *ptr))
				*ptr = '~';
		}
		subprintfe(subfdout, "vfilter", "%d %s %s %d %d %s %s %s\n",
				filter_no, emailid, _filterName.s, header_name, comparision,
				_keyword.len ? _keyword.s : "N/A",
				folder->s,
				bounce_action ? ((bounce_action == 2 || bounce_action == 3) ? forward->s : "Bounce") : (str_diffn(folder->s, "/NoDeliver", 11) ? "Deliver" : "Vapour"));
	}
	flush("vfilter");
	return;
}

int
vfilter_display(char *emailid, int disp_type)
{
	int             i, j, status = -1;
	int                   filter_no, header_name, comparision, bounce_action;
	static stralloc       filter_name = {0}, keyword = {0}, folder = {0}, forward = {0};

	for (j = 0;;) {
		i = vfilter_select(emailid, &filter_no, &filter_name, &header_name, &comparision, &keyword, &folder, &bounce_action, &forward);
		if (i == -1)
			break;
		else
		if (i == -2)
			break;
		if (!j++ && !disp_type) {
			subprintfe(subfdout, "vfilter",
					"No  EmailId                       FilterName"
					"           Header          Comparision      "
					"          Keyword         Folder          "
					"Bounce Forward\n");
			print_hyphen(subfdout, "-", 144);
		}
		status = 0;
		format_filter_display(disp_type, filter_no, emailid, &filter_name, header_name, comparision, &keyword, &folder,
			&forward, bounce_action);
	}
	if (!disp_type)
		print_hyphen(subfdout, "-", 144);
	flush("vfilter");
	if (status == -1 && i == -2)
		return(-2);
	return(status);
}
#endif
