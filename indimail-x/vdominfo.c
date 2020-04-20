/*
 * $Log: vdominfo.c,v $
 * Revision 1.3  2020-04-01 18:58:41+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.2  2019-06-07 15:53:04+05:30  mbhangui
 * use sgetopt library for getopt()
 *
 * Revision 1.1  2019-04-18 15:51:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <str.h>
#include <fmt.h>
#include <sgetopt.h>
#include <open.h>
#include <getln.h>
#include <error.h>
#include <scan.h>
#include <strerr.h>
#include <substdio.h>
#include <subfd.h>
#include <qprintf.h>
#include <getEnvConfig.h>
#endif
#include "get_assign.h"
#include "common.h"
#include "setuserid.h"
#include "check_group.h"
#include "iclose.h"
#include "variables.h"
#include "get_indimailuidgid.h"
#include "isvirtualdomain.h"
#include "get_real_domain.h"
#include "print_control.h"
#include "common.h"
#include "get_local_hostid.h"
#include "get_local_ip.h"
#include "vsmtp_select.h"

#ifndef	lint
static char     sccsid[] = "$Id: vdominfo.c,v 1.3 2020-04-01 18:58:41+05:30 Cprogrammer Exp mbhangui $";
#endif

#define VDOMTOKENS ":\n"
#define FATAL   "vdominfo: fatal: "
#define WARN    "vdominfo: warning: "

static char    *usage =
	"usage: vdominfo [options] [domain]\n"
	"options: -V (print version number)\n"
	"         -a (display all fields, this is the default)\n"
	"         -n (display domain name)\n"
	"         -u (display uid field)\n"
	"         -g (display gid field)\n"
	"         -d (display domain directory)\n"
	"         -b (display user's base directory)\n"
	"         -t (display total users)\n"
	"         -l (display alias domains)"
#ifdef CLUSTERED_SITE
	"\n"
	"         -p (display smtp ports)"
	;
#endif

static void
die_nomem()
{
	strerr_warn1("vdominfo: out of memory", 0);
	_exit(111);
}

#ifdef CLUSTERED_SITE
int
get_options(int argc, char **argv, int *DisplayName, int *DisplayUid, int *DisplayGid, int *DisplayDir,
		int *DisplayBaseDir, int *DisplayAll, int *DisplayTotalUsers, int *DisplayAliasDomains, int *DisplayPort,
		stralloc *Domain)
#else
int
get_options(int argc, char **argv, int *DisplayName, int *DisplayUid, int *DisplayGid, int *DisplayDir,
		int *DisplayBaseDir, int *DisplayAll, int *DisplayTotalUsers, int *DisplayAliasDomains,
		stralloc *Domain)
#endif
{
	int             c;
	extern int      optind;

	*DisplayName =  *DisplayUid = *DisplayGid = *DisplayDir = *DisplayBaseDir = 
		*DisplayTotalUsers = *DisplayAliasDomains = 0;
	*DisplayAll = 1;
#ifdef CLUSTERED_SITE
	*DisplayPort = 0;
#endif
	Domain->len = 0;
	while ((c = getopt(argc, argv, "vanugdbtpl")) != opteof) {
		switch (c)
		{
		case 'n':
			*DisplayName = 1;
			*DisplayAll = 0;
			break;
		case 'u':
			*DisplayUid = 1;
			*DisplayAll = 0;
			break;
		case 'g':
			*DisplayGid = 1;
			*DisplayAll = 0;
			break;
		case 'd':
			*DisplayDir = 1;
			*DisplayAll = 0;
			break;
		case 'b':
			*DisplayBaseDir = 1;
			*DisplayAll = 0;
			break;
		case 't':
			*DisplayTotalUsers = 1;
			*DisplayAll = 0;
			break;
		case 'l':
			*DisplayAliasDomains = 1;
			*DisplayAll = 0;
			break;
#ifdef CLUSTERED_SITE
		case 'p':
			*DisplayPort = 1;
			*DisplayAll = 0;
			break;
#endif
		case 'a':
			*DisplayAll = 1;
			break;
		default:
			strerr_warn2(WARN, usage, 0);
			break;
		}
	}
	if (optind < argc) {
		if (!stralloc_copys(Domain, argv[optind++]) ||
				!stralloc_0(Domain))
			die_nomem();
	}
	return(0);
}

static stralloc tmpbuf = {0}, line = {0};

#ifdef CLUSTERED_SITE
void
display_domain(char *domain, char *dir, uid_t uid, gid_t gid, int DisplayName,
		int DisplayUid, int DisplayGid, int DisplayDir, int DisplayBaseDir, int DisplayAll,
		int DisplayTotalUsers, int DisplayAliasDomains, int DisplayPort)
#else
display_domain(char *domain, char *dir, uid_t uid, gid_t gid, int DisplayName,
		int DisplayUid, int DisplayGid, int DisplayDir, int DisplayBaseDir, int DisplayAll,
		int DisplayTotalUsers, int DisplayAliasDomains)
#endif
{
	char           *real_domain, *base_path;
	char            strnum[FMT_ULONG], inbuf[512];
	struct substdio ssin;
	unsigned long   total;
	int             fd, i, len, match, users_per_level = 0;
#ifdef CLUSTERED_SITE
	char           *ptr, *x, *hostid, *sysconfdir, *controldir;
	static stralloc host_path = {0};
	int             Port, host_cntrl = 0;
#endif

	if (!(real_domain = get_real_domain(domain)))
		return;
	if (DisplayAll) {
		out("vdominfo", "---- Domain ");
		out("vdominfo", domain);
		len = str_len(domain);
		for (i = 25 - len; i > 0; i--)
			out("vdominfo", " ");
		out("vdominfo", " -------------------------------\n");
		if (!str_diff(real_domain, domain)) {
			out("vdominfo", "    domain: ");
			out("vdominfo", domain);
			out("vdominfo", "\n");
		} else {
			out("vdominfo", "    domain: ");
			out("vdominfo", domain);
			out("vdominfo", " aliased to ");
			out("vdominfo", real_domain);
			out("vdominfo", "\n");
		}
		strnum[fmt_ulong(strnum, uid)] = 0;
		out("vdominfo", "       uid: ");
		out("vdominfo", strnum);
		out("vdominfo", "\n");
		strnum[fmt_ulong(strnum, gid)] = 0;
		out("vdominfo", "       gid: ");
		out("vdominfo", strnum);
		out("vdominfo", "\n");
#ifdef CLUSTERED_SITE
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		if (*controldir == '/') {
			if (!stralloc_copys(&host_path, controldir) ||
					!stralloc_catb(&host_path, "/host.cntrl", 11) ||
					!stralloc_0(&host_path))
				die_nomem();
		} else {
			getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
			if (!stralloc_copys(&host_path, sysconfdir) ||
					!stralloc_append(&host_path, "/") ||
					!stralloc_cats(&host_path, controldir) ||
					!stralloc_catb(&host_path, "/host.cntrl", 11) ||
					!stralloc_0(&host_path))
				die_nomem();
		}
		if ((host_cntrl = !access(host_path.s, F_OK))) {
			if ((hostid = get_local_hostid())) {
				out("vdominfo", "   host ID: ");
				out("vdominfo", hostid);
				out("vdominfo", "\n");
			} else 
				out("vdominfo", "   host ID: ???\n");
			if ((ptr = get_local_ip(PF_INET))) {
				out("vdominfo", "   IP Addr: ");
				out("vdominfo", ptr);
				out("vdominfo", "\n");
			} else
				out("vdominfo", "   IP Addr: ??\n");
			for (total = 0; (ptr = vsmtp_select(domain, &Port)) != NULL; total++) {
				out("vdominfo", total ? "          :" : "Ports     :");
				for (x = ptr;*x && *x != ' '; x++);
				if (*x == ' ')
					*x++ = 0;
				else
					x = ptr;
				if (x != ptr) { /*- ip */
					qprintf(subfdout, ptr, "%-18s");
					qprintf(subfdout, " ", "%s");
				}
				qprintf(subfdout, x, "%+20s");
				out("vdominfo", "@");
				qprintf(subfdout, domain, "%-35s");
				strnum[fmt_uint(strnum, Port)] = 0;
				out("vdominfo", strnum);
				out("vdominfo", "\n");
			}
		}
#endif
		out("vdominfo", "Domain Dir: ");
		out("vdominfo", dir);
		out("vdominfo", "\n");
		if (!str_diff(real_domain, domain)) {
			if (!stralloc_copys(&tmpbuf, dir) ||
					!stralloc_catb(&tmpbuf, "/.base_path", 11) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			if ((fd = open_read(tmpbuf.s)) == -1) {
				if (errno != error_noent)
					strerr_die3sys(111, "vdominfo: open: ", tmpbuf.s, ": ");
				getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
				out("vdominfo", "  Base Dir: ");
				out("vdominfo", base_path);
				out("vdominfo", "\n");
			} else {
				substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
				for (;;) {
					if (getln(&ssin, &line, &match, '\n') == -1) {
						strerr_warn3("vdominfo: read: ", tmpbuf.s, ": ", &strerr_sys);
						break;
					}
					if (!match && line.len == 0)
						break;
					else {
						line.len--;
						line.s[line.len] = 0; /*- remove newline */
					}
				}
				close(fd);
				if (line.len) {
					out("vdominfo", "  Base Dir: ");
					out("vdominfo", line.s);
					out("vdominfo", "\n");
				}
			}
			if (!stralloc_copys(&tmpbuf, dir) ||
					!stralloc_catb(&tmpbuf, "/.users_per_level", 17) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			if ((fd = open_read(tmpbuf.s)) == -1) {
				if (errno != error_noent)
					strerr_die3sys(111, "vdominfo: open: ", tmpbuf.s, ": ");
			} else {
				substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
				for (;;) {
					if (getln(&ssin, &line, &match, '\n') == -1) {
						strerr_warn3("vdominfo: read: ", tmpbuf.s, ": ", &strerr_sys);
						break;
					}
					if (!match && line.len == 0)
						break;
					else {
						line.len--;
						line.s[line.len] = 0; /*- remove newline */
					}
				}
				close(fd);
				if (line.len)
					scan_int(line.s, &users_per_level);
			}
			if (!stralloc_copys(&tmpbuf, dir) ||
					!stralloc_catb(&tmpbuf, "/.filesystems", 13) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			total = print_control(tmpbuf.s, domain, users_per_level, 0);
			strnum[fmt_ulong(strnum, total)] = 0;
			out("vdominfo", "     Users: ");
			out("vdominfo", strnum);
			out("vdominfo", "\n");
			if (!stralloc_copys(&tmpbuf, dir) ||
					!stralloc_catb(&tmpbuf, "/.domain_limits", 15) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			out("vdominfo", "   vlimits: ");
			out("vdominfo", access(tmpbuf.s, F_OK) ? "disabled\n" : "enabled\n");
			if (!stralloc_copys(&tmpbuf, dir) ||
					!stralloc_catb(&tmpbuf, "/.aliasdomains", 14) ||
					!stralloc_0(&tmpbuf))
				die_nomem();
			if ((fd = open_read(tmpbuf.s)) == -1) {
				if (errno != error_noent)
					strerr_die3sys(111, "vdominfo: open: ", tmpbuf.s, ": ");
			} else {
				substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
				for (i = 0;;) {
					if (getln(&ssin, &line, &match, '\n') == -1) {
						strerr_warn3("vdominfo: read: ", tmpbuf.s, ": ", &strerr_sys);
						break;
					}
					if (!match && line.len == 0)
						break;
					else {
						line.len--;
						line.s[line.len] = 0; /*- remove newline */
						if (!i++)
							out("vdominfo", "AliasDomains:\n");
						out("vdominfo", line.s);
						out("vdominfo", "\n");
					}
				}
				close(fd);
			}
		}
	} else {
		out("vdominfo", "---- Domain ");
		out("vdominfo", domain);
		out("vdominfo", " ----------------\n");
		if (DisplayName) {
			if (!str_diff(real_domain, domain)) {
				out("vdominfo", "    domain: ");
				out("vdominfo", domain);
				out("vdominfo", "\n");
			} else {
				out("vdominfo", "    domain: ");
				out("vdominfo", domain);
				out("vdominfo", " aliased to ");
				out("vdominfo", real_domain);
				out("vdominfo", "\n");
			}
#ifdef CLUSTERED_SITE
			getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
			if (*controldir == '/') {
				if (!stralloc_copys(&host_path, controldir) ||
						!stralloc_catb(&host_path, "/host.cntrl", 11) ||
						!stralloc_0(&host_path))
					die_nomem();
			} else {
				getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
				if (!stralloc_copys(&host_path, sysconfdir) ||
						!stralloc_append(&host_path, "/") ||
						!stralloc_cats(&host_path, controldir) ||
						!stralloc_catb(&host_path, "/host.cntrl", 11) ||
						!stralloc_0(&host_path))
					die_nomem();
			}
			if ((host_cntrl = !access(host_path.s, F_OK))) {
				if ((hostid = get_local_hostid())) {
					out("vdominfo", "   host ID: ");
					out("vdominfo", hostid);
					out("vdominfo", "\n");
				} else 
					out("vdominfo", "   host ID: ???\n");
				if ((ptr = get_local_ip(PF_INET))) {
					out("vdominfo", "   IP Addr: ");
					out("vdominfo", ptr);
					out("vdominfo", "\n");
				} else
					out("vdominfo", "   IP Addr: ??\n");
			}
#endif
		}
		if (DisplayUid) {
			strnum[fmt_ulong(strnum, uid)] = 0;
			out("vdominfo", "       uid: ");
			out("vdominfo", strnum);
			out("vdominfo", "\n");
		}
		if (DisplayGid) {
			strnum[fmt_ulong(strnum, gid)] = 0;
			out("vdominfo", "       gid: ");
			out("vdominfo", strnum);
			out("vdominfo", "\n");
		}
#ifdef CLUSTERED_SITE
		if (DisplayPort && host_cntrl) {
			for (total = 0;(ptr = vsmtp_select(domain, &Port)) != NULL;total++) {
				out("vdominfo", total ? "          :" : "Ports     :");
				for (x = ptr;*x && *x != ' '; x++);
				if (*x == ' ')
					*x++ = 0;
				else
					x = ptr;
				if (x != ptr) { /*- ip */
					qprintf(subfdout, ptr, "%-18s");
					qprintf(subfdout, " ", "%s");
				}
				qprintf(subfdout, x, "%+20s");
				out("vdominfo", "@");
				qprintf(subfdout, domain, "%-35s");
				len = str_len(ptr) + str_len(domain);
				strnum[fmt_uint(strnum, Port)] = 0;
				out("vdominfo", strnum);
				out("vdominfo", "\n");
			}
		}
#endif
		if (DisplayDir) {
			out("vdominfo", "Domain Dir: ");
			out("vdominfo", dir);
			out("vdominfo", "\n");
		}
		if (!str_diff(real_domain, domain)) {
			if (DisplayBaseDir) {
				if (!stralloc_copys(&tmpbuf, dir) ||
						!stralloc_catb(&tmpbuf, "/.base_path", 11) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
				if ((fd = open_read(tmpbuf.s)) == -1) {
					if (errno != error_noent)
						strerr_die3sys(111, "vdominfo: open: ", tmpbuf.s, ": ");
					getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
					out("vdominfo", "  Base Dir: ");
					out("vdominfo", base_path);
					out("vdominfo", "\n");
				} else {
					substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
					for (;;) {
						if (getln(&ssin, &line, &match, '\n') == -1) {
							strerr_warn3("vdominfo: read: ", tmpbuf.s, ": ", &strerr_sys);
							break;
						}
						if (!match && line.len == 0)
							break;
						else {
							line.len--;
							line.s[line.len] = 0; /*- remove newline */
						}
					}
					close(fd);
					if (line.len) {
						out("vdominfo", "  Base Dir: ");
						out("vdominfo", line.s);
						out("vdominfo", "\n");
					}
				}
			}
			if (DisplayTotalUsers) {
				if (!stralloc_copys(&tmpbuf, dir) ||
						!stralloc_catb(&tmpbuf, "/.users_per_level", 17) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
				if ((fd = open_read(tmpbuf.s)) == -1) {
					if (errno != error_noent)
						strerr_die3sys(111, "vdominfo: open: ", tmpbuf.s, ": ");
				} else {
					substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
					for (;;) {
						if (getln(&ssin, &line, &match, '\n') == -1) {
							strerr_warn3("vdominfo: read: ", tmpbuf.s, ": ", &strerr_sys);
							break;
						}
						if (!match && line.len == 0)
							break;
						else {
							line.len--;
							line.s[line.len] = 0; /*- remove newline */
						}
					}
					close(fd);
					if (line.len)
						scan_int(line.s, &users_per_level);
				}
				if (!stralloc_copys(&tmpbuf, dir) ||
						!stralloc_catb(&tmpbuf, "/.filesystems", 13) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
				total = print_control(tmpbuf.s, domain, users_per_level, 0);
				strnum[fmt_ulong(strnum, total)] = 0;
				out("vdominfo", "     Users: ");
				out("vdominfo", strnum);
				out("vdominfo", "\n");
				if (!stralloc_copys(&tmpbuf, dir) ||
						!stralloc_catb(&tmpbuf, "/.domain_limits", 15) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
				out("vdominfo", "   vlimits: ");
				out("vdominfo", access(tmpbuf.s, F_OK) ? "disabled\n" : "enabled\n");
			}
			if (DisplayAliasDomains) {
				if (!stralloc_copys(&tmpbuf, dir) ||
						!stralloc_catb(&tmpbuf, "/.aliasdomains", 14) ||
						!stralloc_0(&tmpbuf))
					die_nomem();
				if ((fd = open_read(tmpbuf.s)) == -1) {
					if (errno != error_noent)
						strerr_die3sys(111, "vdominfo: open: ", tmpbuf.s, ": ");
				} else {
					substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
					for (i = 0;;) {
						if (getln(&ssin, &line, &match, '\n') == -1) {
							strerr_warn3("vdominfo: read: ", tmpbuf.s, ": ", &strerr_sys);
							break;
						}
						if (!match && line.len == 0)
							break;
						else {
							line.len--;
							line.s[line.len] = 0; /*- remove newline */
							if (!i++)
								out("vdominfo", "AliasDomains:\n");
							out("vdominfo", line.s);
							out("vdominfo", "\n");
						}
					}
					close(fd);
				}
			}
		}
	}
	flush("vdominfo");
}

#ifdef CLUSTERED_SITE
void
display_all_domains(stralloc *Domain, stralloc *Dir, int DisplayName, int DisplayUid,
		int DisplayGid, int DisplayDir, int DisplayBaseDir, int DisplayAll,
		int DisplayTotalUsers, int DisplayAliasDomains, int DisplayPort)
#else
void
display_all_domains(stralloc *Domain, stralloc *Dir, int DisplayName, int DisplayUid,
		int DisplayGid, int DisplayDir, int DisplayBaseDir, int DisplayAll,
		int DisplayTotalUsers, int DisplayAliasDomains)
#endif
{
	char           *ptr, *assigndir, *domain, *uid, *gid, *dir;
	char            inbuf[512];
	struct substdio ssin;
	uid_t           Uid;
	gid_t           Gid;
	int             fd, i, match;

	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	if (!stralloc_copys(&tmpbuf, assigndir) ||
			!stralloc_catb(&tmpbuf, "/assign", 7) ||
			!stralloc_0(&tmpbuf))
		die_nomem();
	if ((fd = open_read(tmpbuf.s)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "vdominfo: open: ", tmpbuf.s, ": ");
		strerr_warn1("vdominfo: no domains found", 0);
		return;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	/*- +example.com-:example.com:555:555:/var/indimail/domains/example.com:-:: */
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("vdominfo5: read: ", tmpbuf.s, ": ", &strerr_sys);
			break;
		}
		if (!match && line.len == 0)
			break;
		else {
			line.len--;
			line.s[line.len] = 0; /*- remove newline */
		}
		match = str_chr(line.s, '#');
		if (line.s[match])
			line.s[match] = 0;
		for (ptr = line.s; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if (*ptr != '+') /*- ignore lines that do not start with '+' */
			continue;
		i = str_chr(ptr, ':');
		if (!ptr[i])
			continue;
		domain = ptr + i + 1;
		if (!*domain)
			continue;
		i = str_chr(domain, ':');
		if (domain[i]) {
			domain[i] = 0;
		} else
			continue;
		uid = domain + i + 1;
		if (!isvirtualdomain(domain))
			continue;
		i = str_chr(uid, ':');
		if (uid[i]) {
			uid[i] = 0;
		} else
			continue;
		gid = uid + i + 1;
		i = str_chr(gid, ':');
		if (gid[i]) {
			gid[i] = 0;
		} else
			continue;
		dir = gid + i + 1;
		i = str_chr(dir, ':');
		if (dir[i]) {
			dir[i] = 0;
		} else
			continue;
		scan_uint(uid, &Uid);
		scan_uint(gid, &Gid);
#ifdef CLUSTERED_SITE
		display_domain(domain, dir, Uid, Gid, DisplayName, DisplayUid, DisplayGid,
			DisplayDir, DisplayBaseDir, DisplayAll, DisplayTotalUsers, DisplayAliasDomains, DisplayPort);
#else
		display_domain(domain, dir, Uid, Gid, DisplayName, DisplayUid, DisplayGid,
			DisplayDir, DisplayBaseDir, DisplayAll, DisplayTotalUsers, DisplayAliasDomains);
#endif
	}
	close(fd);
	return;
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	uid_t           myuid, Uid;
	gid_t           mygid, Gid;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	static stralloc Domain = {0}, Dir = {0};
	int             DisplayName, DisplayUid, DisplayGid, DisplayDir, DisplayBaseDir,
					DisplayAll, DisplayTotalUsers, DisplayAliasDomains;
#ifdef CLUSTERED_SITE
	int             DisplayPort;
#endif

#ifdef CLUSTERED_SITE
	if (get_options(argc, argv, &DisplayName, &DisplayUid, &DisplayGid, &DisplayDir, &DisplayBaseDir,
				&DisplayAll, &DisplayTotalUsers, &DisplayAliasDomains, &DisplayPort, &Domain))
#else
	if (get_options(argc, argv, &DisplayName, &DisplayUid, &DisplayGid, &DisplayDir, &DisplayBaseDir,
				&DisplayAll, &DisplayTotalUsers, &DisplayAliasDomains, &Domain))
#endif
		return(1);
	myuid = getuid();
	mygid = getgid();
	if (Domain.len) {
		if (get_assign(Domain.s, &Dir, &Uid, &Gid) == NULL) {
			strerr_warn2(Domain.s, ": domain does not exist", 0);
			return(1);
		}
		if (myuid != Uid && mygid != Gid && myuid != 0 && check_group(Gid, FATAL) != 1) {
			strnum1[fmt_ulong(strnum1, Uid)] = 0;
			strnum2[fmt_ulong(strnum2, Gid)] = 0;
			strerr_warn5("vdominfo: you must be root or domain user (uid=", strnum1, "/gid=", strnum2, ") to run this program", 0);
			return(1);
		}
		if (setuser_privileges(Uid, Gid, "indimail")) {
			strnum1[fmt_ulong(strnum1, Uid)] = 0;
			strnum2[fmt_ulong(strnum2, Gid)] = 0;
			strerr_warn5("vdominfo: setuser_privilege: (", strnum1, "/", strnum2, "): ", &strerr_sys);
			return (1);
		}
		if (isvirtualdomain(Domain.s) == 1)
#ifdef CLUSTERED_SITE
			display_domain(Domain.s, Dir.s, Uid, Gid, DisplayName, DisplayUid, DisplayGid,
				DisplayDir, DisplayBaseDir, DisplayAll, DisplayTotalUsers, DisplayAliasDomains, DisplayPort);
#else
			display_domain(Domain.s, Dir.s, Uid, Gid, DisplayName, DisplayUid, DisplayGid,
				DisplayDir, DisplayBaseDir, DisplayAll, DisplayTotalUsers, DisplayAliasDomains);
#endif
		else
			strerr_warn3("vdominfo: domain ", Domain.s, " is not a virtual domain", 0);
	} else {
		if (indimailuid == -1 || indimailgid == -1)
			get_indimailuidgid(&indimailuid, &indimailgid);
		if (setuser_privileges(0, indimailgid, "indimail")) {
			strnum2[fmt_ulong(strnum2, indimailgid)] = 0;
			strerr_warn4("vdominfo: setuser_privilege: (0", "/", strnum2, "): ", &strerr_sys);
			return (1);
		}
#ifdef CLUSTERED_SITE
		display_all_domains(&Domain, &Dir, DisplayName, DisplayUid, DisplayGid,
			DisplayDir, DisplayBaseDir, DisplayAll, DisplayTotalUsers,
			DisplayAliasDomains, DisplayPort);
#else
		display_all_domains(&Domain, &Dir, DisplayName, DisplayUid, DisplayGid,
			DisplayDir, DisplayBaseDir, DisplayAll, DisplayTotalUsers,
			DisplayAliasDomains);
#endif
	}
	iclose();
	return(0);
}
