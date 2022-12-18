/*
 * $Log: initsvc.c,v $
 * Revision 1.10  2022-12-18 19:25:22+05:30  Cprogrammer
 * log additional wait status
 *
 * Revision 1.9  2021-07-08 11:32:44+05:30  Cprogrammer
 * removed LIBEXECDIR setting through env variable
 *
 * Revision 1.8  2020-10-06 14:53:02+05:30  Cprogrammer
 * fix for FreeBSD
 *
 * Revision 1.7  2020-10-04 09:28:57+05:30  Cprogrammer
 * changed Label for launchd unit file to org.indimail.svscan
 *
 * Revision 1.6  2020-10-03 12:33:37+05:30  Cprogrammer
 * changed location of indimail.plist to /Library/LaunchDaemons from /Systems/Library/LaunchDaemons
 *
 * Revision 1.5  2020-09-22 07:57:58+05:30  Cprogrammer
 * enable and start services on FreeBSD
 *
 * Revision 1.4  2020-09-21 07:52:47+05:30  Cprogrammer
 * FreeBSD port
 *
 * Revision 1.3  2020-05-29 19:07:18+05:30  Cprogrammer
 * use PREFIX/share/indimail instead of SHAREDDIR
 *
 * Revision 1.2  2020-04-01 18:55:46+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:36:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_QMAIL
#include <str.h>
#include <fmt.h>
#include <strerr.h>
#include <byte.h>
#include <getEnvConfig.h>
#endif
#include "fappend.h"
#include "common.h"

#ifndef	lint
static char     sccsid[] = "$Id: initsvc.c,v 1.10 2022-12-18 19:25:22+05:30 Cprogrammer Exp mbhangui $";
#endif

#define SV_ON    1
#define SV_OFF   2
#define SV_STAT  3
#define SV_PRINT 4

int
systemd_control(char *operation)
{
	int             pid, wStat, cStat, status;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	char           *cmd;

	if (!str_diffn(operation, "enable", 6))
		cmd =  "start";
	else
	if (!str_diffn(operation, "disable", 7))
		cmd =  "stop";
	else
	if (!str_diffn(operation, "status", 6))
		cmd =  "is-enabled";
	else {
		strerr_warn3("initsvc: unknown command [", operation, "]", 0);
		return (1);
	}

	switch (pid = fork())
	{
	case -1:
		strerr_warn1("initsvc: fork: ", &strerr_sys);
		return (1);
	case 0:
		execl("/bin/systemctl", "systemctl", operation, "svscan.service", (char *) 0);
		strerr_warn3("systemctl ", operation, " svscan.service: ", &strerr_sys);
	default:
		if ((pid = wait(&wStat)) == -1) {
			strerr_warn1("initsvc: wait: ", &strerr_sys);
			return (1);
		}
		if (WIFSTOPPED(wStat) || WIFCONTINUED(wStat)) {
			strnum1[fmt_ulong(strnum1, (unsigned long) pid)] = 0;
			strnum2[fmt_int(strnum2, WIFSTOPPED(wStat) ? WIFSTOPPED(wStat) : SIGCONT)] = 0;
			strerr_warn3("initsvc: child [", strnum1, WIFSTOPPED(wStat) ? "] stopped" : "] started", 0);
		} else
		if (WIFSIGNALED(wStat)) {
			strnum1[fmt_ulong(strnum1, (unsigned long) pid)] = 0;
			strnum2[fmt_int(strnum2, WTERMSIG(wStat))] = 0;
			strerr_warn4("initsvc: child [", strnum1, "] killed by signal ", strnum2, 0);
		} else
		if (WIFEXITED(wStat)) {
			if (!(status = WEXITSTATUS(wStat))) {
				if (!str_diffn(operation, "status", 6)) {
					if (!(pid = fork())) {
						execl("/bin/systemctl", "systemctl", cmd, "svscan.service", (char *) 0);
						strerr_warn3("systemctl ", cmd, " svscan.service: ", &strerr_sys);
					} else
					if (pid == -1)
						return (1);
					if ((pid = wait(&cStat)) == -1) {
						strerr_warn1("initsvc: wait: ", &strerr_sys);
						return (1);
					}
					if (WIFSTOPPED(cStat) || WIFCONTINUED(cStat)) {
						strnum1[fmt_ulong(strnum1, (unsigned long) pid)] = 0;
						strnum2[fmt_int(strnum2, WIFSTOPPED(cStat) ? WSTOPSIG(cStat) : SIGCONT)] = 0;
						strerr_warn3("initsvc: child [", strnum1, WIFSTOPPED(wStat) ? "] stopped" : "] started", 0);
						return (-1);
					} else
					if (WIFSIGNALED(cStat)) {
						strnum1[fmt_ulong(strnum1, (unsigned long) pid)] = 0;
						strnum2[fmt_int(strnum2, WTERMSIG(cStat))] = 0;
						strerr_warn4("initsvc: child [", strnum1, "] killed by signal ", strnum2, 0);
						return (-1);
					} else
					if (WIFEXITED(cStat))
						return (WEXITSTATUS(cStat));
					else
						return (-1);
				} else {
					execl("/bin/systemctl", "systemctl", cmd, "svscan.service", (char *) 0);
					strerr_warn3("systemctl ", cmd, " svscan.service: ", &strerr_sys);
					return (1);
				}
			} else {
				if (!str_diffn(operation, "status", 6)) {
					if (!(pid = fork())) {
						execl("/bin/systemctl", "systemctl", cmd, "svscan.service", (char *) 0);
						strerr_warn3("systemctl ", cmd, " svscan.service: ", &strerr_sys);
					} else
					if (pid == -1)
						return (1);
					if ((pid = wait(&cStat)) == -1) {
						strerr_warn1("initsvc: wait: ", &strerr_sys);
						return (1);
					}
					if (WIFSTOPPED(cStat) || WIFCONTINUED(cStat)) {
						strnum1[fmt_ulong(strnum1, (unsigned long) pid)] = 0;
						strnum2[fmt_int(strnum2, WIFSTOPPED(cStat) ? WSTOPSIG(cStat) : SIGCONT)] = 0;
						strerr_warn3("initsvc: child [", strnum1, WIFSTOPPED(wStat) ? "] stopped" : "] started", 0);
						return (1);
					} else
					if (WIFSIGNALED(cStat)) {
						strnum1[fmt_ulong(strnum1, (unsigned long) pid)] = 0;
						strnum2[fmt_int(strnum2, WTERMSIG(cStat))] = 0;
						strerr_warn4("initsvc: child [", strnum1, "] killed by signal ", strnum2, 0);
						return (-1);
					} else
					if (WIFEXITED(cStat))
						return (WEXITSTATUS(cStat));
					else
						return (-1);
				} else {
					strnum1[fmt_int(strnum1, status)] = 0;
					strerr_warn2("initsvc: systemctl exited with status ", strnum1, 0);
					return (1);
				}
			}
		} else {
			strerr_warn1("initsvc: systemctl exited with unknown status", 0);
			return (1);
		}
	}
	return (0);
}

int
main(int argc, char **argv)
{
	char           *device = "/dev/console";
	char           *jobfile = 0, *print_cmd = 0, *jobdir = 0;
	int             fd, flag, debian_version;

	if (argc != 2) {
		strerr_warn1("usage: initsvc -on|-off|-status|-print", 0);
		return (1);
	} else
	if (getuid()) {
		strerr_warn1("initsvc: this program must be run as root", 0);
		return (1);
	}
	if (!str_diffn(argv[1], "-on", 4))
		flag = SV_ON;
	else
	if (!str_diffn(argv[1], "-off", 5))
		flag = SV_OFF;
	else
	if (!str_diffn(argv[1], "-status", 8))
		flag = SV_STAT;
	else
	if (!str_diffn(argv[1], "-print", 7))
		flag = SV_PRINT;
	else {
		strerr_warn1("usage: initsvc -on|-off|-status|-print", 0);
		return (1);
	}
	if (!access("/bin/systemctl", X_OK)) {
		/* Install svscan.service */
		if (access("/lib/systemd/system/svscan.service", F_OK)) {
			if (flag == SV_OFF)
				return (0);
			out("svscan", "Installing svscan.service\n");
			flush("svscan");
			if (chdir(PREFIX"/share/indimail") || chdir("boot")) {
				strerr_warn3("svscan: chdir: ", PREFIX"/share/indimail", "/boot: ", &strerr_sys);
				return (1);
			} else
			if (fappend("systemd", "/lib/systemd/system/svscan.service", "w", 0644, 0, getgid())) {
				strerr_warn1("svscan: fappend systemd /lib/systemd/system/svscan.service: ", &strerr_sys);
				return (1);
			}
		}
		switch(flag)
		{
			case SV_ON:
				return (systemd_control("enable"));
				break;
			case SV_OFF:
				unlink("/lib/systemd/system/svscan.service");
				return (systemd_control("disable"));
				break;
			case SV_STAT:
				out("svscan", "svscan.service is ");
				out("svscan", systemd_control("status") ? "disabled" : "enabled");
				out("svscan", "\n");
				flush("svscan");
				execl("/bin/sh", "sh", "-c",
					"/bin/ls -l /lib/systemd/system/svscan.service", (char *) 0);
				strerr_warn1("svscan: execl /bin/sh: ", &strerr_sys);
				break;
			case SV_PRINT:
				execl("/bin/sh", "sh", "-c",
				"/bin/cat /lib/systemd/system/svscan.service;"
				"/bin/ls -l /lib/systemd/system/svscan.service",
				(char *) 0);
				strerr_die1sys(111, "execl: cat: ");
				break;
		}
	}
#ifdef FREEBSD
	else
	if (!access("/usr/local/etc/rc.d", F_OK)) {
		if (access("/usr/local/etc/rc.d/svscan", F_OK)) {
			if (flag == SV_OFF)
				return (0);
			out("svscan", "Installing /usr/local/etc/rc.d/svscan\n");
			flush("svscan");
			if (chdir(PREFIX"/share/indimail") || chdir("boot")) {
				strerr_warn3("svscan: chdir: ", PREFIX"/share/indimail", "/boot: ", &strerr_sys);
				return (1);
			} else
			if (fappend("svscan", "/usr/local/etc/rc.d/svscan", "w", 0644, 0, getgid())) {
				strerr_warn1("svscan: fappend systemd /usr/local/etc/rc.d/svscan: ", &strerr_sys);
				return (1);
			}
			system("/usr/sbin/service svscan enable");
		}
		switch(flag)
		{
			case SV_ON:
				execl("/bin/sh", "sh", "-c",
					"/usr/sbin/service svscan enable; /usr/sbin/service svscan start",
					(char *) 0);
				strerr_die1sys(111, "execl: /bin/sh: ");
				break;
			case SV_OFF:
				execl("/bin/sh", "sh", "-c",
					"service svscan stop; service svscan disable; service svscan delete",
					(char *) 0);
				strerr_die1sys(111, "execl: /bin/sh: ");
				break;
			case SV_STAT:
				out("svscan", "svscan.service in /etc/rc.conf is ");
				flush("svscan");
				execl("/bin/sh", "sh", "-c",
					"/usr/bin/grep svscan_enable /etc/rc.conf;/bin/ls -l /usr/local/etc/rc.d/svscan",
					(char *) 0);
				strerr_die1sys(111, "execl: /bin/sh: ");
			case SV_PRINT:
				execl("/bin/sh", "sh", "-c",
					"/bin/cat /usr/local/etc/rc.d/svscan;/bin/ls -l /usr/local/etc/rc.d/svscan",
					(char *) 0);
				strerr_die1sys(111, "execl: /bin/sh: ");
				break;
		}
		return (1);
	}
#endif
#ifdef DARWIN
	else
	if (!access("/bin/launchctl", X_OK)) {
		/* Install svscan.plist */
		if (access("/Library/LaunchDaemons/svscan.plist", F_OK)) {
			if (flag == SV_OFF)
				return (0);
			out("svscan", "Installing svscan.plist\n");
			flush("svscan");
			if (chdir(PREFIX"/share/indimail") || chdir("boot")) {
				strerr_warn3("svscan: chdir: ", PREFIX"/share/indimail", "/boot: ", &strerr_sys);
				return (1);
			} else
			if (fappend("svscan.plist", "/Library/LaunchDaemons/org.indimail.svscan.plist", "w", 0644, 0, getgid())) {
				strerr_warn1("svscan: fappend svscan.plist Library/LaunchDaemons/org.indimail.svscan.plist: ", &strerr_sys);
				return (1);
			}
		} 
		switch(flag)
		{
			case SV_ON:
				execl("/bin/launchctl", "launchctl", "load", "/Library/LaunchDaemons/org.indimail.svscan.plist", (char *) 0);
				strerr_die1sys(111, "execl: /bin/launchctl: ");
				break;
			case SV_OFF:
				execl("/bin/launchctl", "launchctl", "unload", "/Library/LaunchDaemons/org.indimail.svscan.plist", (char *) 0);
				strerr_die1sys(111, "execl: /bin/launchctl: ");
				break;
			case SV_STAT:
				execl("/bin/sh", "sh", "-c",
				"/bin/launchctl list org.indimail.svscan || /bin/cat /Library/LaunchDaemons/org.indimail.svscan.plist;\
				/bin/ls -l /Library/LaunchDaemons/org.indimail.svscan.plist", (char *) 0);
				strerr_die1sys(111, "execl: /bin/launchctl: ");
				break;
			case SV_PRINT:
				execl("/bin/sh", "sh", "-c",
				"/bin/cat /Library/LaunchDaemons/org.indimail.svscan.plist;"
				"/bin/ls -l /Library/LaunchDaemons/org.indimail.svscan.plist",
				(char *) 0);
				strerr_die1sys(111, "execl: /bin/cat: ");
				break;
		}
		return (1);
	} /*- if (!access("/bin/launchctl", X_OK)) */
#endif
	else /*- ubuntu/debian crap */
	if (!access("/sbin/initctl", F_OK) && !access("/etc/init", F_OK)) { /*- Upstart */
		jobdir = "/etc/init";
		jobfile = "/etc/init/svscan.conf";
		print_cmd = "/bin/cat /etc/init/svscan.conf;/bin/ls -l /etc/init/svscan.conf";
	} else
	if (!access("/etc/event.d", F_OK)) { /*- Upstart */
		jobdir = "/etc/event.d";
		jobfile = "/etc/event.d/svscan";
		print_cmd = "/bin/cat /etc/event.d/svscan;/bin/ls -l /etc/event.d/svscan";
	} else
		jobdir = (char *) 0;
	if (jobdir) {
		/* Install upstart */
		if (access(jobfile, F_OK)) {
			if (flag == SV_OFF)
				return (0);
			out("svscan", "Installing upstart service\n");
			flush("svscan");
			if (chdir(PREFIX"/share/indimail") || chdir("boot")) {
				strerr_warn3("svscan: chdir: ", PREFIX"/share/indimail", "/boot: ", &strerr_sys);
				return (1);
			} else
			if (fappend("upstart", jobfile, "w", 0644, 0, getgid())) {
				strerr_warn3("svscan: fappend upstart ", jobfile, ": ", &strerr_sys);
				return (1);
			}
		} 
		switch(flag)
		{
			case SV_ON:
				execl("/sbin/initctl", "initctl", "start", "svscan", (char *) 0);
				strerr_die1sys(111, "execl: /bin/initctl: ");
				break;
			case SV_OFF:
				execl("/sbin/initctl", "initctl", "stop", "svscan", (char *) 0);
				strerr_die1sys(111, "execl: /bin/initctl: ");
				break;
			case SV_STAT:
				execl("/sbin/initctl", "initctl",  "status", "svscan", (char *) 0);
				strerr_die1sys(111, "execl: /bin/initctl: ");
				break;
			case SV_PRINT:
				if (print_cmd)
					execl("/bin/sh", "sh", "-c",  print_cmd, (char *) 0);
				strerr_die3sys(111, "execl: ", print_cmd, ": ");
				break;
		}
		return (1);
	}
	if ((fd = open("/dev/console", O_RDWR)) == -1) {
#if 0
		if (errno == EPERM) /*- some virtual servers have this problem */
#endif
			device = "/dev/null";
	} else {
		if (write(fd, "", 1) == -1)
			device = "/dev/null";
		close(fd);
	}
	if (!access("/etc/inittab", F_OK)) {
		debian_version = !access("/etc/debian_version", F_OK);
		strerr_warn1("initsvc: manually add following entry for svscan in /etc/inittab", 0);
		strerr_warn7(debian_version ? "SV:2345:" : "SV:345:",
			flag == SV_ON ? "respawn:" : "off:", LIBEXECDIR, "/svscanboot <>", device, " 2<>", device, 0);
	} else
		strerr_warn1("initsvc: system not supported for svscan startup", 0);
	return (1);
}
