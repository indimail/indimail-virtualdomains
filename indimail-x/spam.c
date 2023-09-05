/*
 * $Log: spam.c,v $
 * Revision 1.5  2023-03-20 10:18:16+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.4  2023-01-22 10:40:03+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.3  2020-10-01 18:29:19+05:30  Cprogrammer
 * initialize pos variable
 *
 * Revision 1.2  2020-04-01 18:57:58+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:37:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#include <regex.h>
#ifdef HAVE_QMAIL
#include <alloc.h>
#include <stralloc.h>
#include <strerr.h>
#include <str.h>
#include <open.h>
#include <env.h>
#include <getln.h>
#include <scan.h>
#include <fmt.h>
#include <byte.h>
#include <error.h>
#include <subfd.h>
#include <getEnvConfig.h>
#include <matchregex.h>
#endif
#include "common.h"
#include "spam.h"
#include "wildmat.h"
#include "lowerit.h"

#ifndef	lint
static char     sccsid[] = "$Id: spam.c,v 1.5 2023-03-20 10:18:16+05:30 Cprogrammer Exp mbhangui $";
#endif

#define BADMAIL 1
#define BADRCPT 2
#define SPAMDB  3

static char    *parseLine1(char *);
static char    *parseLine2(char *);
static char    *parseLine3(char *);

static maddr  **spammer_hash;
static maddr  **ignored_hash;
static int      bounce, maxaddr;

static void
die_nomem()
{
	strerr_warn1("spam: out of memory", 0);
	_exit(111);
}

/*
 * This function parses one line from log file, checks if any " from "
 * is matched. If so, returns the mail address
 * @400000003d97d57c07c6d64c info msg 181049: bytes 1872 from <inditac_escalation@indimail.org> qp 1176 uid 0
 * @400000003da2eb6637759be4 info msg 181997: bytes 50526 from <#@[]> qp 8753 uid 506
 * @400000003da2eb6637759be4 info msg 181997: bytes 50526 from <> qp 8753 uid 506
 */
static char    *
parseLine1(char *str)
{
	char           *ptr, *cptr;
	char           *email;
	int             len;

	if (!(ptr = str_str(str, " from <")))
		return ((char *) 0);
	if (!(cptr = str_str(str, "> qp ")))
		return ((char *) 0);
	*cptr = 0;
	ptr += 7;
	if (!*ptr || !str_diffn(ptr, "#@[]", 5)) {
		bounce++;
		return ((char *) 0);
	}
	len = str_len(ptr) + 1;
	if (!(email = (char *) alloc(sizeof(char) * len)))
		die_nomem();
	str_copyb(email, ptr, len);
	lowerit(email);
	return (email);
}

static char    *
parseLine2(char *str)
{
	char           *ptr;
	char           *email;
	int             i, len;

	if (!(ptr = str_str(str, "starting delivery ")))
		return ((char *) 0);
	i = str_rchr(str, ' ');
	if (!str[i])
		return ((char *) 0);
	i = str_rchr(str, '\n');
	if (str[i])
		str[i] = 0;
	ptr += 1;
	len = str_len(ptr) + 1;
	if (!(email = (char *) alloc(sizeof(char) * len)))
		die_nomem();
	str_copyb(email, ptr, len);
	lowerit(email);
	return (email);
}
/*-
 * @400000004bac90a33711966c qmail-smtpd: pid 14544 from ::ffff:127.0.0.1
 * HELO <indimail.org> MAIL from <sitelist-bounces@lists.sourceforge.net>
 * RCPT <mailstore@indimail.org>
 * AUTH <local-rcpt> Size: 7330
 * X-Bogosity: No, spamicity=0.500000, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=x18cs65599wff, msgID=<E1Nv0Sc-0006w3-BF@sfs-web-2.v29.ch3.sourceforge.com>, ipaddr=216.34.181.68
 */
static char    *
parseLine3(char *str)
{
	char           *ptr, *cptr, *tmp;
	char           *email;
	int             len;

	if (!(ptr = str_str(str, " qmail-smtpd: pid ")))
		return ((char *) 0);
	if (!(ptr = str_str(str, "> MAIL from <")))
		return ((char *) 0);
	ptr += 13;
	for (cptr = ptr; *cptr && *cptr != '>'; cptr++);
	*cptr = 0;
	tmp = cptr + 1;
	if (!(cptr = str_str(tmp, "X-Bogosity")))
		return ((char *) 0);
	for (; *cptr && *cptr != ':'; cptr++);
	if (!*cptr)
		return ((char *) 0);
	for (cptr++;*cptr && isspace(*cptr);cptr++);
	if (!*cptr)
		return ((char *) 0);
	if (byte_diff(cptr, 3, "Yes"))
		return ((char *) 0);
	if (!*ptr || !str_diffn(ptr, "#@[]", 5)) {
		bounce++;
		return ((char *) 0);
	}
	len = str_len(ptr) + 1;
	if (!(email = (char *) alloc(sizeof(char) * len)))
		die_nomem();
	str_copyb(email, ptr, len);
	lowerit(email);
	return (email);
}

/*
 * fills our ignore list. these addresses will not be treated as spammers
 */
int
loadIgnoreList(char *fn)
{
	char           *ptr, *cptr;
	int             len, status, fd, match;
	static stralloc line = {0};
	char            inbuf[4096];
	struct substdio ssin;

	if ((fd = open_read(fn)) == -1) {
		if (errno != error_noent)
			strerr_die3sys(111, "spam: open: ", fn, ": ");
		return (0);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (status = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("spam: read: ", fn, ": ", &strerr_sys);
			close(fd);
			return (-1);
		}
		if (!line.len)
			break;
		if (match) {
			line.len--;
			if (!line.len) {
				strerr_warn2("spam: incomplete line: ", fn, 0);
				continue;
			}
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
		match = str_chr(ptr, ' ');
		if (ptr[match])
			ptr[match] = 0;

		len = str_len(ptr);
		len++;
		if (!(cptr = (char *) alloc(sizeof(char) * len)))
			die_nomem();
		str_copyb(cptr, ptr, len);
		lowerit(cptr);
		if(!insertAddr(IGNOREHASHTAB, cptr))
			status = -1;
	}
	close(fd);
	return (status);
}

/*
 * traverse "from" linked list and decide whether the hit count per mail address
 * exceeds the "spammer threshold"
 */
int
spamReport(int spamNumber, char *outfile)
{
	static stralloc tmpbuf = {0};
	char           *sysconfdir, *controldir, *ptr;
	char           *(spamprog[3]);
	maddr          *p;
	int             i, fd, flag, spamcnt = 0;
	char            strnum[FMT_ULONG], outbuf[512];
	struct substdio ssout;

	if ((fd = open_append(outfile)) == -1)
		strerr_die3sys(111, "spam: open: ", outfile, ": ");
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	subprintfe(subfderr, "spam", "%-40s Mail Count\n", "Spammer's Email Address");
	errflush("spam");
	if(!maxaddr) {
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		scan_int(ptr, &maxaddr);
	}
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (!stralloc_copys(&tmpbuf, controldir) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&tmpbuf, sysconfdir) ||
				!stralloc_append(&tmpbuf, "/") ||
				!stralloc_cats(&tmpbuf, controldir) ||
				!stralloc_0(&tmpbuf))
			die_nomem();
	}
	tmpbuf.len--;
	if (!byte_diff(outfile, tmpbuf.len, tmpbuf.s))
		flag = 1;
	else
		flag = 0;
	for (i = 0; spammer_hash && i < maxaddr; i++) {
		for (p = spammer_hash[i]; p != NULL; p = p->next) {
			if (p->cnt >= spamNumber && !isIgnored(p->mail)) {
				spamcnt++;
				if(flag) {
					if (substdio_puts(&ssout, p->mail) == -1 ||
							substdio_put(&ssout, "\n", 1) == -1)
					{
						strerr_warn3("spam: write: ", outfile, ": ", &strerr_sys);
						return (-1);
					}
				} else {
					if (substdio_puts(&ssout, p->mail) == -1 ||
							substdio_put(&ssout, " ", 1) == -1 ||
							substdio_put(&ssout, strnum, fmt_ulong(strnum, (unsigned long) p->cnt)) == -1 ||
							substdio_put(&ssout, "\n", 1) == -1)
					{
						strerr_warn3("spam: write: ", outfile, ": ", &strerr_sys);
						return (-1);
					}
				}
				subprintfe(subfderr, "spam", "%-40s %d\n", p->mail, p->cnt);
			}
		}
	}
	if (substdio_flush(&ssout) == -1) {
		strerr_warn3("spam: write: ", outfile, ": ", &strerr_sys);
		return (-1);
	}
	close(fd);
	subprintfe(subfderr, "spam", "Bounces: %d\n%d Spammers detected\n", bounce, spamcnt);
	errflush("spam");
	if (flag && spamcnt) {
		spamprog[0] = PREFIX"/sbin/qmail-cdb";
		i = str_rchr(outfile, '/');
		if (outfile[i])
			ptr = outfile + i + 1;
		else
			ptr = outfile;
		spamprog[1] = ptr;
		spamprog[2] = 0;
		execv(*spamprog, spamprog);
		strerr_warn3("spam: exec: ", *spamprog, ": ", &strerr_sys);
	}
	return (spamcnt);
}

/*
 * This function reads the logfile and fills the " from " linked list
 */
int
readLogFile(char *fn, int type, int count)
{

	char           *email;
	static stralloc keyfile = {0}, line = {0};
	char            strnum[FMT_ULONG], buf[1024];
	int             status, fd, keyfd, match;
	unsigned long   pos = 0, seekPos;
	char            inbuf[4096], outbuf[512];
	struct substdio ssin, ssout;

	if (!fn || !*fn)
		fd = 0;
	else {
		if ((fd = open_read(fn)) == -1) {
			strerr_warn3("readLogFile: open: ", fn, ": ", &strerr_sys);
			return (-1);
		}
		if (!stralloc_copyb(&keyfile, "/tmp/", 5) ||
				!stralloc_catb(&keyfile, strnum, fmt_int(strnum, ftok(fn, count == -1 ? 1 : count))) ||
				!stralloc_0(&keyfile))
			die_nomem();
		pos = 0;
		if ((keyfd = open_read(keyfile.s)) == -1) {
			if (errno != error_noent) {
				close(fd);
				strerr_warn3("readLogFile: open: ", keyfile.s, ": ", &strerr_sys);
				return (-1);
			}
		} else {
			substdio_fdbuf(&ssin, read, keyfd, inbuf, sizeof(inbuf));
			if (getln(&ssin, &line, &match, '\n') == -1) {
				strerr_warn3("readLogFile: read: ", keyfile.s, ": ", &strerr_sys);
				close(fd);
				close(keyfd);
				return (-1);
			}
			close(keyfd);
			if (!line.len) {
				strerr_warn2("readLogFile: incomplete line: ", keyfile.s, 0);
				close(fd);
				close(keyfd);
				return (-1);
			}
			if (match) {
				line.len--;
				if (!line.len) {
					strerr_warn2("readLogFile: incomplete line: ", keyfile.s, 0);
					close(fd);
					close(keyfd);
					return (-1);
				}
				line.s[line.len] = 0;
			} else {
				if (!stralloc_0(&line)) {
					close(fd);
					close(keyfd);
					die_nomem();
				}
				line.len--;
			}
			scan_ulong(line.s, &pos);
		}
	}
	if (lseek(fd, pos, SEEK_SET) == -1) {
		strerr_warn3("readLogFile: lseek: ", fn, ": ", &strerr_sys);
		close(fd);
		return (-1);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	for (status = 0, seekPos = -1;;) {
		seekPos = lseek(fd, (off_t) 0, SEEK_CUR);
		switch (type)
		{
			case BADMAIL:
				if ((email = parseLine1(buf)) && !insertAddr(SPAMMERHASHTAB, email))
					status = -1;
			case BADRCPT:
				if ((email = parseLine2(buf)) && !insertAddr(SPAMMERHASHTAB, email))
					status = -1;
			case SPAMDB:
				if ((email = parseLine3(buf)) && !insertAddr(SPAMMERHASHTAB, email))
					status = -1;
		}
	}
	if (fd) {
		close(fd);
		if (seekPos != -1 && seekPos > pos) {
			if ((keyfd = open_write(keyfile.s)) == -1) {
				strerr_warn3("readLogFile: open: ", keyfile.s, ": ", &strerr_sys);
				close(fd);
				return (-1);
			}
			substdio_fdbuf(&ssout, write, keyfd, outbuf, sizeof(outbuf));
			if (substdio_put(&ssout, strnum, fmt_ulong(strnum, seekPos)) == -1 ||
					substdio_put(&ssout, "\n", 1) == -1 ||
					substdio_flush(&ssout) == -1) {
				strerr_warn3("readLogFile: write: ", keyfile.s, ": ", &strerr_sys);
				close(fd);
				close(keyfd);
				return (-1);
			}
			close(keyfd);
		}
	}
	return (status);
}

/*
 * check if the supplied mail address exist in our "ignored" table
 */
int
isIgnored(char *email)
{
	maddr          *p;
	char           *ptr;
	static stralloc pattern = {0};
	int             i, usewildmat;
	char           *qregex;

	if(!maxaddr) {
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		scan_int(ptr, &maxaddr);
	}
	qregex = env_get("QREGEX");
	for (i = 0; i < maxaddr; i++) {
		for (p = ignored_hash[i]; p != NULL; p = p->next) {
			if (qregex) {
				if (*(p->mail) == '@') {
					if (!stralloc_copyb(&pattern, ".*", 2) ||
							!stralloc_cats(&pattern, p->mail) ||
							!stralloc_0(&pattern))
						die_nomem();
				} else {
					if (!stralloc_copys(&pattern, p->mail) ||
							!stralloc_0(&pattern))
						die_nomem();
				}
				if (matchregex(email, pattern.s, 0) == 1)
					return (1);
			} else {
				if (*(p->mail) == '@')
					usewildmat = 1;
				else
					usewildmat = 0;
				for (ptr = p->mail;*ptr && !usewildmat;ptr++) {
					switch(*ptr)
					{
						case '?':
						case '*':
						case '[':
						case ']':
							usewildmat = 1;
							break;
					}
				}
				if(!usewildmat) {
					if(!str_diff(p->mail, email))
						return(1);
				} else {
					if(*(p->mail) == '@') {
						if (!stralloc_copyb(&pattern, "*", 1) ||
								!stralloc_cats(&pattern, p->mail) ||
								!stralloc_0(&pattern))
							die_nomem();
					} else {
						if (!stralloc_copys(&pattern, p->mail) ||
								!stralloc_0(&pattern))
							die_nomem();
					}
					if (wildmat(email, pattern.s))
						return(1);
				}
			} /*- if (qregex) */
		}
	}
	return (0);
}

unsigned int
hash(char *str)
{
	unsigned int    h;
	char           *ptr;

	h = 0;
	for (ptr = str; *ptr; ptr++)
		h = MULTIPLIER * h + *ptr;
	if (!maxaddr) {
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		scan_int(ptr, &maxaddr);
	}
	return h % maxaddr;
}

void
print_list(int list)
{
	maddr         **hash_tab;
	maddr          *sym;
	char           *ptr;
	int             i;

	switch (list)
	{
	case IGNOREHASHTAB:
		hash_tab = ignored_hash;
		break;
	case SPAMMERHASHTAB:
		hash_tab = spammer_hash;
		break;
	default:
		return;
		break;
	}
	getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
	scan_int(ptr, &maxaddr);
	for (i = 0; i < maxaddr; i++) {
		for (sym = hash_tab[i]; sym; sym = sym->next)
			subprintfe(subfderr, "spam", "%s %d mails\n", sym->mail, sym->cnt);
	}
	errflush("spam");
	return;
}

/*
 * Check if the email address if already in table.
 * If so, increment its hit count; if not,
 * add it to table
 */
maddr          *
insertAddr(int ht, char *email)
{
	int             h;
	maddr          *sym;
	maddr         **hash_tab;
	char           *ptr;

	if(!maxaddr) {
		getEnvConfigStr(&ptr, "MAXADDR", MAXADDR);
		scan_int(ptr, &maxaddr);
	}
	if(!spammer_hash && !(spammer_hash = (maddr **) alloc(sizeof(maddr *) * maxaddr)))
		die_nomem();
	if(!ignored_hash && !(ignored_hash = (maddr **) alloc(sizeof(maddr *) * maxaddr)))
		die_nomem();
	switch (ht)
	{
	case IGNOREHASHTAB:
		hash_tab = ignored_hash;
		break;
	case SPAMMERHASHTAB:
		hash_tab = spammer_hash;
		break;
	default:
		return((maddr *) 0);
		break;
	}
	/*
	 * Calculate hash sum, locate and increment count
	 * if email exists, otherwise place it in spammer_hash
	 */
	h = hash(email);
	for (sym = hash_tab[h]; sym; sym = sym->next) {
		if (!str_diffn(email, sym->mail, 1024)) {
			sym->cnt++;
			return sym;
		}
	}
	sym = (maddr *) alloc(sizeof(maddr));
	/*- No need a malloc for mail, because, email's been malloc'ed before */
	sym->mail = email;
	sym->cnt = 1;
	sym->next = hash_tab[h];
	hash_tab[h] = sym;
	return sym;
}
