/*
 * $Log: CopyEmailFile.c,v $
 * Revision 1.1  2019-04-18 08:38:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <open.h>
#include <str.h>
#include <fmt.h>
#include <strerr.h>
#endif
#include "indimail.h"
#include "GetPrefix.h"
#include "get_indimailuidgid.h"
#include "r_mkdir.h"
#include "fappend.h"
#include "update_quota.h"
#include "variables.h"

#ifndef	lint
static char     sccsid[] = "$Id: CopyEmailFile.c,v 1.1 2019-04-18 08:38:38+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
die_nomem()
{
	strerr_warn1("CopyEmailFile: out of memory", 0);
	_exit(111);
}

/*
 * copy_method 
 * 0 - Copy File
 * 1 - Link email to file in bulk_mail
 */
int
CopyEmailFile(homedir, fname, email, To, From, Subject, setDate, copy_method, message_size)
	const char     *homedir;
	char           *fname;
	const char     *email;
	char           *To, *From, *Subject;
	int             setDate, copy_method;
	long            message_size;
{
	time_t          tm;
	int             wfd, rfd, count, i, j, k;
	static int      counter;
	pid_t           pid;
	static stralloc hostname = {0};
	static stralloc TmpFile = {0};
	static stralloc buffer = {0};
	static stralloc MailFile = {0};
	static stralloc DeliveredTo = {0};
	static stralloc Tolist = {0};
	static stralloc Fromlist = {0};
	static stralloc SubJect = {0};
	static stralloc Date = {0};
	char           *ptr, *cptr;
	char            inbuf[4096], strnum1[FMT_ULONG], strnum2[FMT_ULONG], strnum3[FMT_ULONG];
	struct stat     statbuf;

	tm = time(0);
	pid = getpid();
	if (!stralloc_ready(&hostname, 64))
		die_nomem();
	if (gethostname(hostname.s, 64)) {
		strerr_warn1("CopyEmailFile: gethostname: ", &strerr_sys);
		return (-1);
	}
	if (!stralloc_0(&hostname))
		die_nomem();
	hostname.len--;
	if (copy_method == 1 && (ptr = GetPrefix((char *) email, (char *) homedir))) {
		if (*ptr == '_')
			*ptr = '/';
		for (cptr = ptr + 1;*cptr && *cptr != '_';cptr++);
		*cptr = 0;
		if (!stralloc_copys(&TmpFile, ptr) ||
				!stralloc_catb(&TmpFile, "/bulk_mail", 10) ||
				!stralloc_0(&TmpFile))
			die_nomem();
		if (indimailuid == -1 || indimailgid == -1)
			get_indimailuidgid(&indimailuid, &indimailgid);
		if (!r_mkdir(TmpFile.s, 0755, indimailuid, indimailgid)) {
			i = str_rchr(fname, '/');
			if (fname[i])
				cptr = fname + i + 1;
			else
				cptr = fname;
			TmpFile.len--;
			if (!stralloc_cats(&TmpFile, cptr) || !stralloc_0(&TmpFile))
				die_nomem();
			strnum1[i = fmt_ulong(strnum1, tm)] = 0;
			strnum2[j = fmt_ulong(strnum2, pid)] = 0;
			strnum3[k = fmt_ulong(strnum3, message_size)] = 0;
			if (!stralloc_copys(&MailFile, (char *) homedir) ||
					!stralloc_catb(&MailFile, "/Maildir/new/", 13) ||
					!stralloc_catb(&MailFile, strnum1, i) ||
					!stralloc_append(&MailFile, ".") ||
					!stralloc_catb(&MailFile, strnum2, j) ||
					!stralloc_append(&MailFile, ".") ||
					!stralloc_cat(&MailFile, &hostname) ||
					!stralloc_catb(&MailFile, ",S=", 3) ||
					!stralloc_catb(&MailFile, strnum3, k) ||
					!stralloc_0(&MailFile))
				die_nomem();
			if (stat(TmpFile.s, &statbuf)) {
				if (errno == ENOENT &&
						!fappend(fname, TmpFile.s, "w", 0644, indimailuid, indimailgid) &&
						!link(TmpFile.s, MailFile.s)) {
					if (!stralloc_copys(&buffer, (char *) homedir) ||
							!stralloc_catb(&buffer, "/Maildir", 8) ||
							!stralloc_0(&buffer))
						die_nomem();
					(void) update_quota(buffer.s, message_size);
					return (0);
				}
			} else
			if (statbuf.st_nlink > MAX_LINK_COUNT) {
				if (!unlink(TmpFile.s) &&
						!fappend(fname, TmpFile.s, "w", 0644, indimailuid, indimailgid) &&
						!link(TmpFile.s, MailFile.s)) {
					if (!stralloc_copys(&buffer, (char *) homedir) ||
							!stralloc_catb(&buffer, "/Maildir", 7) ||
							!stralloc_0(&buffer))
						die_nomem();
					(void) update_quota(buffer.s, message_size);
					return (0);
				}
			} else
			if (!link(TmpFile.s, MailFile.s)) {
				if (!stralloc_copys(&buffer, (char *) homedir) ||
						!stralloc_catb(&buffer, "/Maildir", 8) ||
						!stralloc_0(&buffer))
					die_nomem();
				(void) update_quota(buffer.s, message_size);
				return (0);
			}
		}
		/*- Some failure occurred. So try Copy Method */
	}
	if (!stralloc_copyb(&DeliveredTo, "Delivered-To: ", 14) ||
			!stralloc_cats(&DeliveredTo, (char *) email) ||
			!stralloc_append(&DeliveredTo, "\n") ||
			!stralloc_0(&DeliveredTo))
		die_nomem();
	DeliveredTo.len--;
	count = DeliveredTo.len;
	if (setDate) {
		if (!stralloc_copyb(&Date, "Date: ", 6) ||
				!stralloc_cats(&Date, ctime(&tm)) ||
				!stralloc_0(&Date))
			die_nomem();
		Date.len--;
		count += Date.len;
	}
	tm += counter++;
	if (To && *To) {
		if (!stralloc_copyb(&Tolist, "To: ", 4) ||
				!stralloc_cats(&Tolist, To) ||
				!stralloc_append(&Tolist, "\n") ||
				!stralloc_0(&Tolist))
			die_nomem();
		Tolist.len--;
		count += Tolist.len;
	}
	if (From && *From) {
		if (!stralloc_copyb(&Fromlist, "From: ", 6) ||
				!stralloc_cats(&Fromlist, To) ||
				!stralloc_append(&Fromlist, "\n") ||
				!stralloc_0(&Fromlist))
			die_nomem();
		Fromlist.len--;
		count += Fromlist.len;
	}
	if (Subject && *Subject) {
		if (!stralloc_copyb(&SubJect, "Subject: ", 9) ||
				!stralloc_cats(&SubJect, To) ||
				!stralloc_append(&SubJect, "\n") ||
				!stralloc_0(&SubJect))
			die_nomem();
		SubJect.len--;
		count += SubJect.len;
	}
	strnum1[i = fmt_ulong(strnum1, tm)] = 0;
	strnum2[j = fmt_ulong(strnum2, pid)] = 0;
	strnum3[k = fmt_ulong(strnum3, message_size + count)] = 0;
	if (!stralloc_copys(&TmpFile, (char *) homedir) ||
			!stralloc_catb(&TmpFile, "/Maildir/tmp/", 13) ||
			!stralloc_catb(&TmpFile, strnum1, i) ||
			!stralloc_append(&TmpFile, ".") ||
			!stralloc_catb(&TmpFile, strnum2, j) ||
			!stralloc_append(&TmpFile, ".") ||
			!stralloc_cat(&TmpFile, &hostname) ||
			!stralloc_catb(&TmpFile, ",S=", 3) ||
			!stralloc_catb(&TmpFile, strnum3, k) ||
			!stralloc_0(&TmpFile))
		die_nomem();
	if (!stralloc_copys(&MailFile, (char *) homedir) ||
			!stralloc_catb(&MailFile, "/Maildir/new/", 13) ||
			!stralloc_catb(&MailFile, strnum1, i) ||
			!stralloc_append(&MailFile, ".") ||
			!stralloc_catb(&MailFile, strnum2, j) ||
			!stralloc_append(&MailFile, ".") ||
			!stralloc_cat(&MailFile, &hostname) ||
			!stralloc_catb(&MailFile, ",S=", 3) ||
			!stralloc_catb(&MailFile, strnum3, k) ||
			!stralloc_0(&MailFile))
		die_nomem();
	if ((rfd = open_read(fname)) == -1) {
		strerr_warn3("CopyEmailFile: open: ", fname, ": ", &strerr_sys);
		return (-2);
	}
	if ((wfd = open(TmpFile.s, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
		strerr_warn3("CopyEmailFile: open: ", TmpFile.s, ": ", &strerr_sys);
		close(rfd);
		return (-2);
	}
	if (write(wfd, DeliveredTo.s, DeliveredTo.len) != DeliveredTo.len) {
		strerr_warn3("CopyEmailFile: write-header: ", TmpFile.s, ": ", &strerr_sys);
		close(wfd);
		close(rfd);
		unlink(TmpFile.s);
		return (-2);
	}
	if (setDate && write(wfd,  Date.s, Date.len) != Date.len) {
		strerr_warn3("CopyEmailFile: write-header: ", TmpFile.s, ": ", &strerr_sys);
		close(wfd);
		close(rfd);
		unlink(TmpFile.s);
		return (-2);
	}
	if (Tolist.len && write(wfd,  Tolist.s, Tolist.len) != Tolist.len) {
		strerr_warn3("CopyEmailFile: write-header: ", TmpFile.s, ": ", &strerr_sys);
		close(wfd);
		close(rfd);
		unlink(TmpFile.s);
		return (-2);
	}
	if (Fromlist.len && write(wfd,  Fromlist.s, Fromlist.len) != Fromlist.len) {
		strerr_warn3("CopyEmailFile: write-header: ", TmpFile.s, ": ", &strerr_sys);
		close(wfd);
		close(rfd);
		unlink(TmpFile.s);
		return (-2);
	}
	if (SubJect.len && write(wfd,  SubJect.s, SubJect.len) != SubJect.len) {
		strerr_warn3("CopyEmailFile: write-header: ", TmpFile.s, ": ", &strerr_sys);
		close(wfd);
		close(rfd);
		unlink(TmpFile.s);
		return (-2);
	}
	for (;;) {
		if (!(count = read(rfd, inbuf, 4096)))
			break;
		else
		if (count == -1) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn3("CopyEmailFile: read: ", fname, ": ", &strerr_sys);
			close(wfd);
			close(rfd);
			unlink(TmpFile.s);
			return (-2);
		}
		if (write(wfd, inbuf, count) != count) {
			strerr_warn3("CopyEmailFile: write: ", TmpFile.s, ": ", &strerr_sys);
			close(wfd);
			close(rfd);
			unlink(TmpFile.s);
			return (-2);
		}
	} /*- for (;;) */
	close(wfd);
	close(rfd);
	if (!stralloc_copys(&buffer, (char *) homedir) ||
			!stralloc_catb(&buffer, "/Maildir", 7) ||
			!stralloc_0(&buffer))
		die_nomem();
	(void) update_quota(buffer.s, message_size + count);
	if (rename(TmpFile.s, MailFile.s)) {
		strerr_warn5("CopyEmailFile: rename ", TmpFile.s, " --> ", MailFile.s, ": ", &strerr_sys);
		unlink(TmpFile.s);
		return (-2);
	}
	return (0);
}
