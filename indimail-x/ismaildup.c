/*
 * $Id: ismaildup.c,v 1.8 2023-10-23 18:45:17+05:30 Cprogrammer Exp mbhangui $
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
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_OPENSSL_EVP_H
#include <openssl/evp.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <strerr.h>
#include <substdio.h>
#include <getln.h>
#include <scan.h>
#include <error.h>
#include <byte.h>
#include <fmt.h>
#include <env.h>
#include <getEnvConfig.h>
#include <makeargs.h>
#include <wait.h>
#endif
#include "dblock.h"

#ifndef	lint
static char     sccsid[] = "$Id: ismaildup.c,v 1.8 2023-10-23 18:45:17+05:30 Cprogrammer Exp mbhangui $";
#endif

static char     strnum[FMT_ULONG];

#ifdef HAVE_SSL
static void
die_nomem()
{
	strerr_warn1("ismaildup: out of memory", 0);
	_exit(111);
}

void
remove_lock(char *fn, int fd, int lockfd)
{
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, fn, 1);
#endif
}

int
fix_size(char *fn, int fd, int lockfd, size_t size)
{
	if (lseek(fd, 0l, SEEK_SET) < 0) {
		strerr_warn1("ismaildup: lseek: ", &strerr_sys);
		remove_lock(fn, fd, lockfd);
		return -1;
	}
	if (ftruncate(fd, size) == -1) {
		strerr_warn3("ismaildup: ftruncate: ", fn, ": ", &strerr_sys);
		remove_lock(fn, fd, lockfd);
		return -1;
	}
	return 0;
}

/*-
 * opens Maildir/dupmd5
 * returns
 *  1 for duplicate message
 *  0 for new message
 * -1 for error
 */
static int
duplicateMD5(char *fileName, char *md5val)
{
	static stralloc line = {0};
	char           *ptr;
	unsigned long   interval;
	char            inbuf[512], outbuf[512];
	int             i, fd, match, flag;
	size_t          size;
#ifdef FILE_LOCKING
	int             lockfd;
#endif
	time_t          curTime, recTime;
	substdio        ssin, ssout;

	curTime = time(0);
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(fileName, 1)) == -1) {
		strerr_warn3("ismaildup: getDbLock: ", fileName, ": ", &strerr_sys);
		return -1;
	}
#endif
	if (access(fileName, F_OK))
		fd = open(fileName, O_CREAT|O_RDWR, 0644);
	else
		fd = open(fileName, O_RDWR);
	if (fd == -1) {
		strerr_warn3("ismaildup: open: ", fileName, ": ", &strerr_sys);
		remove_lock(fileName, fd, lockfd);
		return -1;
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	getEnvConfigStr(&ptr, "ELIMINATE_DUPS_INT", "900");
	scan_ulong(ptr, &interval);
	for (size = 0, flag = 0;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("ismaildup: read: ", fileName, ": ", &strerr_sys);
			remove_lock(fileName, fd, lockfd);
			return -2;
		}
		if (!line.len)
			break;
		if (match) 
			line.len--;
		else {
			if (!stralloc_append(&line, "\n"))
				die_nomem();
			line.len--;
		}
		if (!line.len)
			continue;
		scan_long(line.s, &recTime);
		if (curTime > recTime + interval)
			continue;
		/*- skip past unix_time and wsp */
		for (ptr = line.s; *ptr && !isspace(*ptr); ptr++);
		if (!*ptr || !isspace(*ptr))
			continue;
		for (; *ptr && isspace(*ptr); ptr++);
		if (!*ptr)
			continue;
		if (!byte_diff(md5val, 32, ptr)) {
			flag = 1;
			continue;
		}
		size += (line.len + 1);
		if (substdio_put(&ssout, line.s, line.len + 1) == -1) {
			strerr_warn3("ismaildup: write error: ", fileName, ": ", &strerr_sys);
			remove_lock(fileName, fd, lockfd);
			return -1;
		}
	} /*- for (;;) */
	if (fix_size(fileName, fd, lockfd, size) == -1)
		return -1;
	/*- update with the latest timestamp */
	strnum[i = fmt_ulong(strnum, curTime)] = 0;
	if (substdio_put(&ssout, strnum, i) == -1 ||
			substdio_put(&ssout, " ", 1) == -1 ||
			substdio_puts(&ssout, md5val) == -1 ||
			substdio_put(&ssout, "\n", 1) == -1 ||
			substdio_flush(&ssout)== -1 ) {
		strerr_warn3("ismaildup: write error: ", fileName, ": ", &strerr_sys);
		return -1;
	}
	remove_lock(fileName, fd, lockfd);
	return flag ? 1 : 0;
}

int
ismaildup(char *maildir)
{
	int             md_len, error, code, wait_status, i, werr, n, pim[2];
	pid_t           pid;
	static stralloc dupfile = {0}, md5sum = {0};
	char          **argv;
	char           *ptr;
	char           *binqqargs[8];
	char            inbuf[512], strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	EVP_MD_CTX     *mdctx;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	EVP_MD_CTX     mdctxO;
#endif
	const EVP_MD   *md;
	unsigned char   md_value[EVP_MAX_MD_SIZE];

	if (pipe(pim) == -1)
		_exit(111);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		_exit(111);
	case 0:
		if (lseek(0, 0l, SEEK_SET) < 0)
			strerr_die1sys(111, "ismaildup: lseek: ");
		close(pim[0]);
		if (dup2(pim[1], 1) == -1)
			_exit(111);
		if ((ptr = env_get("ELIMINATE_DUPS_ARGS")) && !(argv = makeargs(ptr)))
			die_nomem();
		if (ptr) {
			binqqargs[0] = argv[0];
			execv(*binqqargs, argv);
		} else {
			binqqargs[0] = PREFIX"/bin/822header";
			binqqargs[1] = "-X";
			binqqargs[2] = "received";
			binqqargs[3] = "-X";
			binqqargs[4] = "delivered-to";
			binqqargs[5] = "-X";
			binqqargs[6] = "x-delivered-to";
			binqqargs[7] = 0;
			execv(*binqqargs, binqqargs);
		}
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	close(pim[1]);
	OpenSSL_add_all_digests();
	if (!(md = EVP_get_digestbyname("md5")))
		strerr_die1x(111, "ismaildup: Unknown message digest md5");
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	if (!(mdctx = EVP_MD_CTX_new()))
		strerr_die1x(111, "ismaildup: Digest create failure");
#else
	mdctx = &mdctxO;
#endif
	EVP_MD_CTX_init(mdctx);
	if (!EVP_DigestInit_ex(mdctx, md, NULL)) {
		strerr_warn1("ismaildup: Digest Initialization failure", 0);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		EVP_MD_CTX_free(mdctx);
#else
		EVP_MD_CTX_cleanup(mdctx);
#endif
		_exit (111);
	}
	error = 0;
	for (;;) {
		if ((n = read(pim[0], inbuf, sizeof(inbuf))) == -1) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			close(pim[0]);
			error = 1;
			break;
		} else
		if (!n)
			break;
		/*- Calculate Checksum */
		if (!EVP_DigestUpdate(mdctx, inbuf, n)) {
			error = 1;
			strerr_warn1("Digest Update failure", 0);
			break;
		}
	}
	for (;;) {
		if (!(i = wait_pid(&wait_status, pid)))
			break;
		else
		if (i == -1) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_warn3("ismaildup: waitpid: ", strnum1, ": ", &strerr_sys);
			error = 1;
			break;
		}
		if (!(i = wait_handler(wait_status, &werr)) && werr)
			continue;
		else
		if (werr == -1) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strerr_warn3("ismaildup: ", strnum1, ": internal wait handler error", 0);
			error = 1;
		} else
		if (werr) {
			strnum1[fmt_ulong(strnum1, pid)] = 0;
			strnum2[fmt_uint(strnum2, werr)] = 0;
			strerr_warn4("ismaildup: ", strnum1, ": killed by signal ", strnum2, 0);
			error = 1;
		} else {
			code = i;
			break;
		}
	}
	if (!error && !code) {
		EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *) &md_len);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		EVP_MD_CTX_free(mdctx);
#else
		EVP_MD_CTX_cleanup(mdctx);
#endif
		for (n = 0; n < md_len; n++) {
			strnum1[i = fmt_hexbyte(strnum1, md_value[n])] = 0;
			if (!stralloc_catb(&md5sum, strnum1, i))
				die_nomem();
		}
		if (!stralloc_0(&md5sum))
			die_nomem();
		if (!stralloc_copys(&dupfile, maildir) ||
				!stralloc_catb(&dupfile, "/dupmd5", 7) ||
				!stralloc_0(&dupfile))
			die_nomem();
		return ((n = duplicateMD5(dupfile.s, md5sum.s)) < 0 ? 0 : n);
	}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	EVP_MD_CTX_free(mdctx);
#else
	EVP_MD_CTX_cleanup(mdctx);
#endif
	if (error || code)
		_exit(111);
	return 0;
}
#endif

#ifdef MAIN
#include "mktempfile.h"
#include "get_message_size.h"

int
main(int argc, char **argv)
{
	int             MsgSize;
#ifdef MAKE_SEEKABLE
	char           *str;
#endif

	if (argc < 3)
		strerr_die1x(100, "USAGE: ismaildup directory program");
#ifdef MAKE_SEEKABLE
	if ((str = env_get("MAKE_SEEKABLE")) && *str != '0' && mktempfile(0))
		strerr_die1sys(111, "ismaildup: mktempfile: ");
#endif
	/*- if we don't know the message size then read it */
	if (!(MsgSize = get_message_size()))
		strerr_die1x(0, "ismaildup: discarding 0 size message");
#ifdef HAVE_SSL
	if (env_get("ELIMINATE_DUPS") && ismaildup(argv[1]))
		strerr_die1x(0, "ismaildup: discarding duplicate msg");
#endif
	execv(argv[2], argv + 2);
	_exit(111);
}
#endif

/*
 * $Log: ismaildup.c,v $
 * Revision 1.8  2023-10-23 18:45:17+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.8  2023-03-26 00:32:38+05:30  Cprogrammer
 * fixed code using wait_handler
 *
 * Revision 1.7  2023-03-20 10:10:50+05:30  Cprogrammer
 * standardize getln handling
 *
 * Revision 1.6  2022-12-18 19:26:39+05:30  Cprogrammer
 * recoded wait logic
 *
 * Revision 1.5  2022-05-10 20:01:31+05:30  Cprogrammer
 * use headers from include path
 *
 * Revision 1.4  2021-06-11 17:00:40+05:30  Cprogrammer
 * replaced makeseekable(), MakeArgs() with mktempfile(), makeargs() from libqmail
 *
 * Revision 1.3  2020-10-01 18:25:44+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.2  2020-04-01 18:56:39+05:30  Cprogrammer
 * moved authentication functions to libqmail
 *
 * Revision 1.1  2019-04-18 08:21:46+05:30  Cprogrammer
 * Initial revision
 *
 */
