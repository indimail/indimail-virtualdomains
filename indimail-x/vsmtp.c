/*
 * $Log: vsmtp.c,v $
 * Revision 1.4  2022-10-20 11:59:21+05:30  Cprogrammer
 * converted function prototype to ansic
 *
 * Revision 1.3  2019-06-07 15:40:32+05:30  Cprogrammer
 * use sgetopt library for getopt()
 *
 * Revision 1.2  2019-04-22 23:20:25+05:30  Cprogrammer
 * added missing strerr.h
 *
 * Revision 1.1  2019-04-11 07:56:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef CLUSTERED_SITE
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_QMAIL
#include <sgetopt.h>
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#include <scan.h>
#include <fmt.h>
#include <subfd.h>
#include <qprintf.h>
#endif
#include "parse_email.h"
#include "vsmtp_select.h"
#include "vsmtp_insert.h"
#include "vsmtp_update.h"
#include "vsmtp_delete.h"
#include "variables.h"
#include "sql_getip.h"
#include "smtp_port.h"

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp.c,v 1.4 2022-10-20 11:59:21+05:30 Cprogrammer Exp mbhangui $";
#endif

#define FATAL   "vsmtp: fatal: "
#define WARN    "vsmtp: warning: "

#define SMTP_SELECT 0
#define SMTP_INSERT 1
#define SMTP_DELETE 2
#define SMTP_UPDATE 3

int             SmtpAction;

static char    *usage =
	"usage: vsmtp [options] [[-d|-i port|-u port] -m mta hostid@domain_name]\n"
	"       vsmtp [options] [-s domain_name]\n"
	"       vsmtp [options] [-s hostid@domain_name]\n"
	"       vsmtp [options] [-s -m mta hostid@domain_name]\n"
	"options: -V ( print version number )\n"
	"         -v ( verbose )\n"
	"         -s domain_name ( show smtp ports )\n"
	"         -d ( delete smtp ports )\n"
	"         -i port (insert smtp port)\n"
	"         -u port (update smtp port)\n"
	"         -m mta_ipaddress"
	;

static void
die_nomem()
{
	strerr_warn1("vsmtp: out of memory", 0);
	_exit (111);
}

int
get_options(int argc, char **argv, char **mdahost, char **mta, stralloc *hostid, stralloc *domain, int *port)
{
	int             c, i;

	*mta = *mdahost = 0;
	*port = -1;
	SmtpAction = SMTP_SELECT;
	while ((c = getopt(argc, argv, "vsdi:u:m:")) != opteof) {
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 's':
			SmtpAction = SMTP_SELECT;
			break;
		case 'd':
			SmtpAction = SMTP_DELETE;
			break;
		case 'i':
			SmtpAction = SMTP_INSERT;
			scan_uint(optarg, (unsigned int *) port);
			break;
		case 'u':
			SmtpAction = SMTP_UPDATE;
			scan_uint(optarg, (unsigned int *) port);
			break;
		case 'm':
			*mta = optarg;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			return(1);
		}
	}
	if (optind < argc) {
		*mdahost = argv[optind++];
		i = str_chr(*mdahost, '@');
		if ((*mdahost)[i])
			parse_email(*mdahost, hostid, domain);
		else {
			if (!stralloc_copys(domain, *mdahost) || !stralloc_0(domain))
				die_nomem();
			domain->len--;
		}
	}
	if (!*mdahost) {
		if (SmtpAction == SMTP_SELECT)
			strerr_warn1("vsmtp: must supply hostid@domain or domain_name", 0);
		else
			strerr_warn1("vsmtp: must supply hostid@domain", 0);
		strerr_warn2(WARN, usage, 0);
		return(1);
	} else
	if (!*mta && SmtpAction != SMTP_SELECT) {
		strerr_warn1("vsmtp: must supply MTA IP Address", 0);
		strerr_warn2(WARN, usage, 0);
		return(1);
	}
	return(0);
}

int
main(int argc, char **argv)
{
	char           *tmpsmtp, *ptr, *cptr, *mdahost, *mta, *dst_ip;
	char            strnum[FMT_ULONG];
	static stralloc hostid = {0}, domain = {0}, src_ip = {0}, dst_hostid = {0};
	int             oldport, err, port, len;

	if (get_options(argc, argv, &mdahost, &mta, &hostid, &domain, &port))
		return(0);
	err = 0;
	switch (SmtpAction)
	{
	case SMTP_SELECT:
		for (err = 1, oldport = 0;; oldport++) { /*- tmpsmtp = "src_ip dst_hostid" */
			if (!(tmpsmtp = vsmtp_select(domain.s, &port))) {
				if (!oldport)
					return(0);
				break;
			}
			for (len = 0, cptr = ptr = tmpsmtp;*ptr && *ptr != ' '; len++, ptr++);
			if (!stralloc_copyb(&src_ip, cptr, len) ||
					!stralloc_0(&src_ip))
				die_nomem();
			for (;*ptr && *ptr == ' '; ptr++);
			for (len = 0, cptr = ptr; *ptr; len++, ptr++);
			if (!stralloc_copyb(&dst_hostid, cptr, len) ||
					!stralloc_0(&dst_hostid))
				die_nomem();
			if (mta && str_diffn(mta, src_ip.s, src_ip.len))
				continue;
			if (hostid.len && str_diffn(hostid.s, dst_hostid.s, dst_hostid.len))
				continue;
			if (!(dst_ip = sql_getip(dst_hostid.s)))
				dst_ip = "x.x.x.x";
			if (!oldport)
				if (subprintf(subfdoutsmall,
						"Source HostIP        "
						"Destination HostID IP Address"
						"-> Port\n") == -1)
					strerr_die1sys(111, "unable to write to stdout");
			if (subprintf(subfdoutsmall, "%-20s %-18s %-18s -> %d\n", src_ip.s, dst_hostid.s, dst_ip, port) == -1)
				strerr_die1sys(111, "unable to write to stdout");
			if (substdio_flush(subfdoutsmall) == -1)
				strerr_die1sys(111, "unable to write to stdout");
			err = 0;
		}
		break;
	case SMTP_INSERT:
		if (SmtpAction == SMTP_INSERT)
			err = vsmtp_insert(hostid.s, mta, domain.s, port);
		break;
	case SMTP_DELETE:
			err = vsmtp_delete(hostid.s, mta, domain.s, port);
		break;
	case SMTP_UPDATE:
			if ((oldport = smtp_port(mta, domain.s, hostid.s)) == -1) {
				strerr_warn1("vsmtp: failed to get Old Port", 0);
				err = 1;
			} else
				err = vsmtp_update(hostid.s, mta, domain.s, oldport, port);
		break;
	default:
		strnum[fmt_uint(strnum, (unsigned int) SmtpAction)] = 0;
		strerr_warn2("vsmtp: Smtp Action is invalid ", strnum, 0);
		return (1);
		break;
	}
	return(err);
}
#else
#include <strerr.h>
int
main()
{
	strerr_warn1("IndiMail not configured with --enable-user-cluster=y", 0);
	return(0);
}
#endif
