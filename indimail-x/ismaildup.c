/*
 * $Log: ismaildup.c,v $
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
#endif
#include "dblock.h"

#ifndef	lint
static char     sccsid[] = "$Id: ismaildup.c,v 1.5 2022-05-10 20:01:31+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_SSL 
static void
die_nomem()
{
	strerr_warn1("ismaildup: out of memory", 0);
	_exit(111);
}

static int
duplicateMD5(char *fileName, char *md5buffer)
{
	static stralloc line = {0};
	char           *ptr;
	unsigned long   interval;
	char            inbuf[512], outbuf[512], strnum[FMT_ULONG];
	int             i, fd, match;
#ifdef FILE_LOCKING
	int             lockfd;
#endif
	time_t          curTime, recTime;
	substdio        ssin, ssout;

	curTime = time(0);
#ifdef FILE_LOCKING
	if ((lockfd = getDbLock(fileName, 1)) == -1) {
		strerr_warn3("ismaildup: getDbLock: ", fileName, ": ", &strerr_sys);
		return (-1);
	}
#endif
	if (access(fileName, F_OK))
		fd = open(fileName, O_CREAT|O_RDWR, 0644);
	else
		fd = open(fileName, O_RDWR);
	if (fd == -1) {
		strerr_warn3("ismaildup: open: ", fileName, ": ", &strerr_sys);
#ifdef FILE_LOCKING
		delDbLock(lockfd, fileName, 1);
#endif
		return (-1);
	}
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	getEnvConfigStr(&ptr, "DUPLICATE_INTERVAL", "900");
	scan_ulong(ptr, &interval);
	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1) {
			strerr_warn3("ismaildup: read: ", fileName, ": ", &strerr_sys);
			close(fd);
#ifdef FILE_LOCKING
			delDbLock(lockfd, fileName, 1);
#endif
			return (-2);
		}
		if (!line.len)
			break;
		if (!match)
			continue;
		if (!stralloc_0(&line))
			die_nomem();
		line.len--;
		scan_long(line.s, &recTime);
		if (curTime > recTime + interval) {
			for (ptr = line.s; *ptr && !isspace(*ptr); ptr++);
			if (!isspace(*ptr))
				continue;
			for (; *ptr && isspace(*ptr); ptr++);
			if (!*ptr)
				continue;
			if (!byte_diff(md5buffer, 32, ptr)) {
				close(fd);
#ifdef FILE_LOCKING
				delDbLock(lockfd, fileName, 1);
#endif
				return (1);
			}
		} else
			continue; /*- expired records */
	}
	if (lseek(fd, 0 - line.len, SEEK_END) == -1) {
		strerr_warn3("ismaildup: lseek: ", fileName, ": ", &strerr_sys);
		return (-1);
	}
	strnum[i = fmt_ulong(strnum, curTime)] = 0;
	if (substdio_put(&ssout, strnum, i) ||
			substdio_put(&ssout, " ", 1) ||
			substdio_puts(&ssout, md5buffer) ||
			substdio_flush(&ssout)) {
		strerr_warn3("ismaildup: write error: ", fileName, ": ", &strerr_sys);
		return (-1);
	}
	close(fd);
#ifdef FILE_LOCKING
	delDbLock(lockfd, fileName, 1);
#endif
	return (0);
}

int
ismaildup(char *maildir)
{
	int             md_len, error, code, wait_status, i, n, pim[2];
	long unsigned   pid;
	static stralloc dupfile = {0}, md5sum = {0};
	char          **argv;
	char           *ptr;
	char           *binqqargs[8];
	char            inbuf[512], strnum[FMT_ULONG];
	EVP_MD_CTX     *mdctx;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	EVP_MD_CTX     mdctxO;
#endif
	const EVP_MD   *md;
	unsigned char   md_value[EVP_MAX_MD_SIZE];

	if (pipe(pim) == -1)
		return (0);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (0);
	case 0:
		if (lseek(0, 0l, SEEK_SET) < 0) {
			strerr_warn1("ismaildup: lseek: ", &strerr_sys);
			_exit(111);
		}
		close(pim[0]);
		if (dup2(pim[1], 1) == -1)
			_exit(111);
		binqqargs[0] = PREFIX"/bin/822header";
		if ((ptr = env_get("ELIMINATE_DUPS_ARGS")) && !(argv = makeargs(ptr))) {
			strerr_warn1("ismaildup: makeargs: ", &strerr_sys);
			_exit(111);
		}
		if (ptr)
			execv(*binqqargs, argv);
		else
		{
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
	if (!(md = EVP_get_digestbyname("md5"))) {
		strerr_warn1("ismaildup: Unknown message digest md5", 0);
		return (-1);
	}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	if (!(mdctx = EVP_MD_CTX_new())) {
		strerr_warn1("ismaildup: Digest create failure", 0);
		return (-1);
	}
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
		return (-1);
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
		if ((pid = wait(&wait_status)) == -1) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_warn1("ismaildup: 822header crashed. indimail bug", 0);
			return (0);
		}
		break;
	}
	if (WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status)) {
		strerr_warn1("822header crashed.", 0);
		return (0);
	} else
	if (WIFEXITED(wait_status)) {
		if ((code = WEXITSTATUS(wait_status))) {
			strnum[fmt_int(strnum, code)] = 0;
			strerr_warn3("ismaildup: 822header failed code(", strnum, ")", 0);
			return (0);
		}
	}
	if (!error) {
		EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *) &md_len);
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
		EVP_MD_CTX_free(mdctx);
#else
		EVP_MD_CTX_cleanup(mdctx);
#endif
		for (n = 0; n < md_len; n++) {
			strnum[i = fmt_hexbyte(strnum, md_value[n])] = 0;
			if (!stralloc_catb(&md5sum, strnum, i))
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
	return (0);
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

	if (argc != 3) {
		strerr_warn1("USAGE: ismaildup directory program", 0);
		_exit (100);
	}
#ifdef MAKE_SEEKABLE
	if ((str = env_get("MAKE_SEEKABLE")) && *str != '0' && mktempfile(0)) {
		strerr_warn1("ismaildup: mktempfile: ", &strerr_sys);
		_exit(111);
	}
#endif
	/*- if we don't know the message size then read it */
	if (!(MsgSize = get_message_size())) {
		strerr_warn1("ismaildup: discarding 0 size message", 0);
		_exit(0);
	}
#ifdef HAVE_SSL 
	if (env_get("ELIMINATE_DUPS") && ismaildup(argv[1])) {
		strerr_warn1("ismaildup: discarding duplicate msg", 0);
		_exit (0);
	}
#endif
	if (lseek(0, 0, SEEK_SET) == -1) {
		strerr_warn1("ismaildup: lseek: ", &strerr_sys);
		_exit(111);
	}
	execv(argv[2], argv + 2);
	_exit(111);
}
#endif
