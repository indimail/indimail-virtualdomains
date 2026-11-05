/*-
 * $Id: crc.c,v 1.10 2026-11-05 10:02:20+05:30 Cprogrammer Exp mbhangui $
 *
 * © 1993 Manvendra Bhangui
 * All intellectual property developed during the term of this agreement, including but not
 * limited to crc.c, shall be and remain the sole and exclusive property of Manvendra Bhangui.
 *
 * ============================================================================
 * File: crc.c
 * Description: Implementation of the POSIX 32-bit CRC algorithm for intrusion
 *              detection
 *
 * Portions based on the original GNU cksum/crc implementation.
 * Original Authors: Q. Frank Xia (1992)
 *                   David MacKenzie (Code Reorganization)
 * ============================================================================
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_GRP_H
#include <grp.h>
#endif
#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#ifdef HAVE_FNMATCH_H
#include <fnmatch.h>
#endif
#ifdef HAVE_REGEX_H
#include <regex.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_QMAIL
#include <open.h>
#include <subfd.h>
#include <buffer.h>
#include <strerr.h>
#include <substdio.h>
#include <stralloc.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <sgetopt.h>
#include <makeargs.h>
#include <getln.h>
#include <error.h>
#endif
#ifdef CHECK_LICENSE
#ifdef HAVE_DKIM
#include "license.h"
#endif
#endif

#ifndef	lint
static char     sccsid[] = "$Id: crc.c,v 1.10 2026-11-05 10:02:20+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_QMAIL
static char     strnum[FMT_ULONG];
static stralloc crcline = { 0 }, fscan = {0}, excl = {0}, pbuf = {0};
static int      opt_e = 0, opt_b = 0, opt_w = 0, opt_l = 0, opt_R = 0,
				fret = 0, opt_r = 0, opt_s = 0, opt_o = 0, opt_W = 0;
#define FATAL "crc: fatal: "
#define WARN  "crc: warn: "

#ifndef S_IRGRP
#define S_IRGRP	(S_IREAD >> 3)
#define S_IWGRP (S_IWRITE >> 3)
#define S_IXGRP (S_IEXEC >> 3)
#define S_IROTH (S_IREAD >> 6)
#define S_IWOTH (S_IWRITE >> 6)
#define S_IXOTH (S_IEXEC >> 6)
#endif/*- ifndef S_IRGRP -*/

#ifdef CRCTAB

#define BIT(x)   ((unsigned long)1 << (x))
#define SBIT     BIT(31)

/*
 * The generating polynomial is
 *
 * 32   26   23   22   16   12   11   10   8   7   5   4   2   1
 * G(X)=X  + X  + X  + X  + X  + X  + X  + X  + X + X + X + X + X + X + 1
 *
 * The i bit in GEN is set if X^i is a summand of G(X) except X^32.
 */

#define GEN     (BIT(26)|BIT(23)|BIT(22)|BIT(16)|BIT(12)|BIT(11)|BIT(10)\
                |BIT(8) |BIT(7) |BIT(5) |BIT(4) |BIT(2) |BIT(1) |BIT(0));

static unsigned long r[8];

static void
fill_r()
{
	int             i;

	r[0] = GEN;
	for (i = 1; i < 8; i++)
		r[i] = (r[i - 1] & SBIT) ? (r[i - 1] << 1) ^ r[0] : r[i - 1] << 1;
}

static unsigned long
_remainder(int m)
{
	unsigned long   rem = 0;
	int             i;

	for (i = 0; i < 8; i++)
		if (BIT(i) & m)
			rem = rem ^ r[i];
	return rem & 0xFFFFFFFF;	/*- Make it run on 64-bit machine.  */
}

int
main(int argc, char **argv)
{
	int             i, j, k, l;

	fill_r();
	if (substdio_put(subfdout, "unsigned long crctab[256] = {\n0x0,\n", 35) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	for (i = 0; i < 51; i++) {
		for (k = 1; k <= 5; k++) {
			strnum[j = fmt_xlong(strnum, _remainder(i * 5 + k))] = 0;
			if (substdio_put(subfdout, "0x", 2) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			for (l = 8 - j; l > 0; l--) {
				if (substdio_put(subfdout, "0", 1) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			}
			if (substdio_put(subfdout, strnum, j) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			if (i < 50)
				if (substdio_put(subfdout, k < 5 ? ", " : ",\n", 2) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			else
			if (k < 5)
				if (substdio_put(subfdout, ", ", 2) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		}
	}
	if (substdio_put(subfdout, "\n};\n", 4) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	exit(0);
}
#else /*- !CRCTAB */
#define BUFLEN   (1 << 16)	/*- Number of bytes to read at once. */
#define INIT_CRC 0L			/*- init value  : -1 0 0 */
static long     initial_crc = INIT_CRC;
stralloc        owner = { 0 };
stralloc        group = { 0 };

static unsigned long const crctab[256] = {
	0x0,
	0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
	0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
	0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
	0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
	0x5BD4B01B, 0x569796C2, 0x52568B75, 0x6A1936C8, 0x6ED82B7F,
	0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A,
	0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
	0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58,
	0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033,
	0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
	0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
	0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4,
	0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
	0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5,
	0x2AC12072, 0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
	0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 0x7897AB07,
	0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
	0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1,
	0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
	0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B,
	0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698,
	0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D,
	0x94EA7B2A, 0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
	0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
	0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
	0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80,
	0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
	0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
	0x58C1663D, 0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629,
	0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C,
	0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
	0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E,
	0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65,
	0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601, 0xDEA580D8,
	0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
	0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2,
	0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
	0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
	0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
	0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 0x7B827D21,
	0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A,
	0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087,
	0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
	0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D,
	0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
	0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB,
	0xDBEE767C, 0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
	0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 0x89B8FD09,
	0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
	0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF,
	0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

/*-
 * remove white space from the end
 * of a stralloc variable
 */
void
striptrailingwhitespace(stralloc *sa)
{
	while (sa->len > 0) {
		switch (sa->s[sa->len - 1])
		{
		case '\n':
		case ' ':
		case '\t':
			--sa->len;
			break;
		default:
			return;
		}
	}
}

/*
 * read entire file into variable sa
 * skipping line containing comments
 *
 * returns
 *  0 - file not found
 *  1 - file is opened successfuly
 * -1 - system error
 */
int
read_exclude_file(stralloc *sa, const char *fn)
{
	substdio        ss;
	int             fd, match;
	char            inbuf[BUFFER_INSIZE];

	if (!stralloc_copys(sa, ""))
		return -1;
	if ((fd = open_read(fn)) == -1) {
		if (errno == error_noent)
			return 0;
		strerr_die4sys(111, FATAL, "unable to open file ", fn, " for read: ");
	}
	substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ss, &excl, &match, '\n') == -1)
			break;
		if (!excl.len) {
			close(fd);
			return 1;
		}
		striptrailingwhitespace(&excl);
		if (!stralloc_0(&excl) ||
				(excl.s[0] && excl.s[0] != '#' && !stralloc_cat(sa, &excl)))
			break;
		if (!match) {
			close(fd);
			return 1;
		}
	}
	close(fd);
	return -1;
}

int
exclude_path(const char *path)
{
	size_t          tlen;
	char           *entry;

	if (!opt_e || !pbuf.len)
		return 0;
	for (entry = pbuf.s, tlen = 0; tlen < pbuf.len;) {
		size_t          elen = strlen(entry);
		size_t          plen = strlen(path);

		tlen += (str_len(entry) + 1);
		if (!*entry)
			break;

		/*- 1. Exact match */
		if (strcmp(path, entry) == 0)
			return 1;

		/*- 2. Prefix match */
		if (plen >= elen && strncmp(path, entry, elen) == 0) {
			if (path[elen] == '\0' || path[elen] == '/' || path[elen] == '\\' ||
			    entry[elen - 1] == '/' || entry[elen - 1] == '\\')
				return 1;
		}
		/*- 3. Pattern match or Regex match */
		if (opt_r) {
			regex_t         re;
			if (regcomp(&re, entry, REG_EXTENDED | REG_NOSUB) == 0) {
				int             rmatch = (regexec(&re, path, 0, NULL, 0) == 0);
				regfree(&re);
				if (rmatch)
					return 1;
			}
		} else
		if (fnmatch(entry, path, 0) == 0)
			return 1;
		entry = pbuf.s + tlen;
	}
	return 0;
}

static int
print_perm(unsigned int perm, substdio *ssout)
{

	char            string[20];

	(void) str_copy(string, "----------");
	/*-
     *    #define    S_IFMT         0170000  type of file
     *    #define    S_IFIFO        0010000  FIFO special
     *    #define    S_IFCHR        0020000  character special
     *    #define    S_IFDIR        0040000  directory
     *    #define    S_IFBLK        0060000  block special
     *    #define    S_IFREG        0100000  regular file
     *    #define    S_IFLNK        0120000  symbolic link
     *    #define    S_IFSOCK       0140000  socket
     *    #define    S_ISVTX        0001000  save swapped text even after use
     *    #define    S_IREAD        0000400  read permission, owner
     *    #define    S_IWRITE       0000200  write permission, owner
    -*/
	switch (perm & S_IFMT) {
	case S_IFDIR:
		string[0] = 'd';
		break;
	case S_IFBLK:
		string[0] = 'b';
		break;
	case S_IFCHR:
		string[0] = 'c';
		break;
	case S_IFIFO:
		string[0] = 'p';
		break;
	case S_IFLNK:
		string[0] = 'l';
		break;
	case S_IFSOCK:
		string[0] = 's';
		break;
	}
	/*- owner */
	if (perm & S_IREAD)
		string[1] = 'r';
	if (perm & S_IWRITE)
		string[2] = 'w';
	if (perm & S_ISUID && perm & S_IEXEC)
		string[3] = 's';
	else
	if (perm & S_IEXEC)
		string[3] = 'x';
	else
	if (perm & S_ISUID)
		string[3] = 'S';

	/*- group */
	if (perm & S_IRGRP)
		string[4] = 'r';
	if (perm & S_IWGRP)
		string[5] = 'w';
	if (perm & S_ISGID && perm & S_IXGRP)
		string[6] = 's';
	else
	if (perm & S_IXGRP)
		string[6] = 'x';
	else
	if (perm & S_ISGID)
		string[6] = 'l';

	/*- others */
	if (perm & S_IROTH)
		string[7] = 'r';
	if (perm & S_IWOTH)
		string[8] = 'w';
	if (perm & S_ISVTX && perm & S_IXOTH)
		string[9] = 't';
	else
	if (perm & S_IXOTH)
		string[9] = 'x';
	else
	if (perm & S_ISVTX)
		string[9] = 'T';
	if (substdio_put(ssout, " ", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_puts(ssout, string) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(ssout, " ", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_flush(ssout) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	return (0);
}

static void
stats(const char *path, struct stat *statptr, const char *tag, substdio *ssout)
{

	struct passwd  *entry;
	struct group   *group_entry;
	int             i;
	char            a_time[50];
	static uid_t    prev_uid = -9999;
	static gid_t    prev_gid = -9999;

	if (statptr->st_uid != prev_uid) {
		entry = getpwuid((int) statptr->st_uid);
		if (entry) {
			if (!stralloc_copys(&owner, entry->pw_name))
				strerr_die2x(111, FATAL, "out of memory");
		} else {
			strnum[i = fmt_ulong(strnum, statptr->st_uid)] = 0;
			if (!stralloc_copyb(&owner, strnum, i))
				strerr_die2x(111, FATAL, "out of memory");
		}
		prev_uid = statptr->st_uid;
	}
	if (statptr->st_gid != prev_gid) {
		group_entry = getgrgid((int) statptr->st_gid);
		if (group_entry) {
			if (!stralloc_copys(&group, group_entry->gr_name))
				strerr_die2x(111, FATAL, "out of memory");
		} else {
			strnum[i = fmt_ulong(strnum, statptr->st_gid)] = 0;
			if (!stralloc_copyb(&group, strnum, i))
				strerr_die2x(111, FATAL, "out of memory");
		}
		prev_gid = statptr->st_gid;
	}
	(void) str_copyb(a_time, ctime(&statptr->st_mtime), 24);
	a_time[24] = '\0';
	print_perm(statptr->st_mode, ssout);
	if (substdio_put(ssout, owner.s, owner.len) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(ssout, "\t", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(ssout, group.s, group.len) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(ssout, "\t", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_puts(ssout, a_time + 4) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(ssout, " ", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_puts(ssout, (char *) path ) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	if (tag) {
		if (substdio_put(ssout, ",tag=", 5))
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		else
		if (substdio_puts(ssout, tag))
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	}
	if (substdio_put(ssout, "\n", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	return;
}

uint32_t
byte_crc(const char *path, unsigned long *lcount, int statflag,
		int displayhex, const char *tag, struct stat *statbuf, substdio *ssout)
{
	int             fd, nr;
	uint32_t        crc, bytes_read = 0l;
	char           *ptr;
	char            inbuf[BUFFER_INSIZE + 1];

	/*- open the file and do a silent crc on it */
#if defined(LINUX) || defined(FREEBSD)
	/*- prevent hooking the open system call */
#if defined(SYS_openat) && defined(AT_FDCWD)
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_openat,AT_FDCWD,FILE,FLAG,MODE)
#else
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_open,FILE,FLAG,MODE)
#endif
	if ((fd = SYS_OPEN(path, O_RDONLY, 0)) == -1) {
#else
	if ((fd = open(path, O_RDONLY)) == -1) {
#endif
		if (errno == error_noent)
			return 0;
		strerr_warn3(WARN, path, ": ", &strerr_sys);
		return (-1);
	}
	for (crc = initial_crc;;) {
		if ((nr = read(fd, (char *) inbuf, BUFFER_INSIZE)) == -1) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			(void) close(fd);
			return (-1);
		}
		if (!nr)
			break;
		inbuf[nr] = 0;
		bytes_read += nr;
		for (ptr = inbuf; nr > 0; ptr++, nr--) {
			if (lcount && *ptr == '\n')
				(*lcount)++;
			crc = (crc << 8) ^ crctab[((crc >> 24) ^ *ptr) & 0xFF];
		}
	}
	while (bytes_read > 0) {
		crc = (crc << 8) ^ crctab[((crc >> 24) ^ bytes_read) & 0xFF];
		bytes_read >>= 8;
	}
	crc = ~crc & 0xFFFFFFFF;
	(void) close(fd);
	if (statflag != -1) {
		int             t;
		strnum[nr = (displayhex ? fmt_xlong : fmt_ulong) (strnum, crc)] = 0;
		if (displayhex) {
			if (substdio_put(ssout, "0x", 2) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			for (t = nr, nr = 8 - nr; t <= 8 && nr; nr--) {
				if (substdio_put(ssout, "0", 1) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			}
			nr = t;
		}
		if (substdio_put(ssout, strnum, nr) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (statflag)
			stats(path, statbuf, tag, ssout);
		else
		if (substdio_put(ssout, "\n", 1) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (substdio_flush(ssout) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	}
	return crc;
}

uint32_t
line_crc(const char *path, unsigned long *lcount, int statflag,
		int displayhex, const char *tag, struct stat *statbuf, substdio *ssout)
{
	int             fd, nr, match;
	uint32_t        crc, total_crc = 0l, bytes_read = 0l;
	char            inbuf[BUFFER_INSIZE + 1];
	char           *ptr;
	substdio        ssin;

	/*- open the file and do a silent crc on it */
#if defined(LINUX) || defined(FREEBSD)
	/*- prevent hooking the open system call */
#if defined(SYS_openat) && defined(AT_FDCWD)
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_openat,AT_FDCWD,FILE,FLAG,MODE)
#else
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_open,FILE,FLAG,MODE)
#endif
	if ((fd = SYS_OPEN(path, O_RDONLY, 0)) == -1) {
#else
	if ((fd = open(path, O_RDONLY)) == -1) {
#endif
		if (errno == error_noent)
			return 0;
		strerr_warn3(WARN, path, ": ", &strerr_sys);
		return (-1);
	}
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
	for (;;) {
		if (getln(&ssin, &crcline, &match, '\n') == -1) {
			strerr_warn4(WARN, "read: ", path, ": ", &strerr_sys);
			close(fd);
			return -1;
		}
		if (!crcline.len)
			break;
		nr = crcline.len;
		if (opt_w) {
			/*- trim trailing \n, \r, \t, and space characters */
			while (nr > 0 && (crcline.s[nr - 1] == '\n' ||
						crcline.s[nr - 1] == '\r' || crcline.s[nr - 1] == '\t' ||
						crcline.s[nr - 1] == ' ')) {
				nr--;
			}
			/*- trim leading wsp, \t */
			for (ptr = crcline.s; *ptr == ' ' || *ptr == '\t'; ptr++, nr--);
		} else
			ptr = crcline.s;
		if (!nr)
			continue;
		if (lcount)
			(*lcount)++;
		bytes_read = nr;
		for (crc = initial_crc; nr > 0; ptr++, nr--)
			crc = (crc << 8) ^ crctab[((crc >> 24) ^ *ptr) & 0xFF];
		while (bytes_read > 0) {
			crc = (crc << 8) ^ crctab[((crc >> 24) ^ bytes_read) & 0xFF];
			bytes_read >>= 8;
		}
		crc = ~crc & 0xFFFFFFFF;
		total_crc += crc;
	}
	(void) close(fd);
	if (statflag != -1) {
		int             t;
		strnum[nr = (displayhex ? fmt_xlong : fmt_ulong) (strnum, total_crc)] = 0;
		if (displayhex) {
			if (substdio_put(ssout, "0x", 2) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			for (t = nr, nr = 8 - nr; t <= 8 && nr; nr--) {
				if (substdio_put(ssout, "0", 1) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			}
			nr = t;
		}
		if (substdio_put(ssout, strnum, nr) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (statflag)
			stats(path, statbuf, tag, ssout);
		else
		if (substdio_put(ssout, "\n", 1) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (substdio_flush(ssout) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	}
	return total_crc;
}

int
special_file_crc(const char *path, struct stat *st, int statflag,
		int displayhex, const char *tag, substdio *ssout)
{
	int             nr;
	struct stat     statbuf_t;
	uint32_t        crc, bytes_read;
	char           *ptr;

	memset((char *) &statbuf_t, 0, sizeof (struct stat));
	statbuf_t.st_mode = st->st_mode;
	statbuf_t.st_ino = st->st_ino;
	statbuf_t.st_dev = st->st_dev;
	statbuf_t.st_rdev = st->st_rdev;
	statbuf_t.st_size = st->st_size;
	/*-
	 * 	Theoritically one can use mtime, ctime
	 * 	too in the crc calculations also.
	 *	statbuf_t.st_atime = st->st_atime;
	 *	statbuf_t.st_mtime = st->st_mtime;
	 *	statbuf_t.st_ctime = st->st_ctime;
	 */
	nr = sizeof(struct stat);
	for (crc = initial_crc, ptr = (char *) &statbuf_t;nr > 0; ptr++, nr--)
		crc = (crc << 8) ^ crctab[((crc >> 24) ^ *ptr) & 0xFF];
	bytes_read = sizeof(struct stat);
	while (bytes_read > 0) {
		crc = (crc << 8) ^ crctab[((crc >> 24) ^ bytes_read) & 0xFF];
		bytes_read >>= 8;
	}
	crc = ~crc & 0xFFFFFFFF;
	if (statflag != -1) {
		strnum[nr = (displayhex ? fmt_xlong : fmt_ulong) (strnum, crc)] = 0;
		if (displayhex){
			if (substdio_put(ssout, "0x", 2) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			for (nr = 8 - nr; nr; nr--) {
				if (substdio_put(ssout, "0", 1) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			}
		}
		if (substdio_puts(ssout, strnum) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (statflag)
			stats(path, st, tag, ssout);
		else
		if (substdio_put(ssout, "\n", 1) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (substdio_flush(ssout) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	}
	return crc;
}

/*
 * Calculate and print the checksum and length in bytes
 * Return crc if successful, -1 if an error occurs.
 * When scanning recursively it returns -1 if an error
 * occurs scanning any path and return 0 if no error is
 * encountered while scanning.
 */
uint32_t
printcrc(const char *p, unsigned long *lcount, int statflag,
		int displayhex, const char *tag, substdio *ssout)
{
	int             is_dir, do_dirs_only;
	const char     *path;
	uint32_t        crc;
	struct stat     statbuf, statbuf_t;
	static stralloc tmp1 = {0}, tmp2 = {0};

	if (*p == '-') {
		path = p + 1;
		do_dirs_only = 1;
	} else {
		path = p;
		do_dirs_only = 0;
	}
	if (exclude_path(path))
		return 0;
	/*-
	 * use lstat to get details for the link instead for
	 * the target path (impiles opt_s is not selected)
	 */
	is_dir = 0;
	if ((opt_s ? stat : lstat) (path, &statbuf)) {
		if (errno != error_noent)
			strerr_warn3(WARN, path, ": ", &strerr_sys);
		return (-1);
	}
	/*-
	 * using stat() after a lstat() eliminates broken links
	 */
	if (!opt_s && opt_b && stat(path, &statbuf_t)) {
		if (opt_W) {
			strerr_warn4(WARN, "Broken link ", path, ": ", &strerr_sys);
			return (-1);
		} else
			return 0;
	}
	if (S_ISDIR(statbuf.st_mode))
		is_dir = 1;
	if (lcount)
		*lcount = 0l;
	if (!S_ISREG(statbuf.st_mode)) {
		/*-
		 * dirs, symbolic links, character/block devices,
		 * fifo, unix sockets. See print_perms()
		 */
		if (do_dirs_only && !is_dir)
			return 0;
		crc = special_file_crc(path, &statbuf, statflag, displayhex, tag, ssout);
		if (!opt_R)
			return crc;
		if (!fret && (int) crc == -1)
			fret = -1;
		/*- block to do recursive traversal */
		if (do_dirs_only) {
			if (!stralloc_copys(&tmp2, path)) /*- prevent overlap */
				strerr_die2x(111, FATAL, "out of memory");
			if (!stralloc_copyb(&tmp1, "-", 1) || !stralloc_cat(&tmp1, &tmp2) || !stralloc_0(&tmp1))
				strerr_die2x(111, FATAL, "out of memory");
		} else {
			if (!stralloc_copys(&tmp2, path)) /*- prevent overlap */
				strerr_die2x(111, FATAL, "out of memory");
			if (!stralloc_copy(&tmp1, &tmp2) || !stralloc_0(&tmp1))
				strerr_die2x(111, FATAL, "out of memory");
		}
		tmp1.len--;
		if (is_dir) {
			DIR            *d = opendir(path);
			if (d) {
				struct dirent  *de;
				unsigned long   tlcount;
				int             len;
				while ((de = readdir(d)) != NULL) {
					if (str_diff(de->d_name, ".") == 0 || str_diff(de->d_name, "..") == 0)
						continue;
					len = tmp1.len;
					if (!stralloc_append(&tmp1, "/") || !stralloc_cats(&tmp1, de->d_name) ||
							!stralloc_0(&tmp1))
						strerr_die2x(111, FATAL, "out of memory");
					crc = printcrc(tmp1.s, &tlcount, statflag, displayhex, tag, ssout);
					if (!fret && (int) crc == -1)
						fret = -1;
					tmp1.len = len;
				}
				closedir(d);
			}
		}
		/*-
		 * we come here when opt_r is set and path
		 * is symlink, character/block, fifo, socket
		 */
		return (fret);
	} /*- if (!S_ISREG(statbuf.st_mode)) */
	if (do_dirs_only)
		return 0;
	if (opt_l)
		crc = line_crc(path, lcount, statflag, displayhex, tag, &statbuf, ssout);
	else
		crc = byte_crc(path, lcount, statflag, displayhex, tag, &statbuf, ssout);
	return (opt_R ? (uint32_t) fret : crc);
}

#ifdef MAIN
static void
usage(const char *p, int exitval)
{
	if (substdio_puts(subfderr, "USAGE: ") == -1 ||
			substdio_puts(subfderr, p) == -1 ||
			substdio_puts(subfderr, " [options] path1 path2 ...\n") == -1 ||
			substdio_puts(subfderr, "options\n") == -1 ||
			substdio_puts(subfderr, "      -b (skip broken links)\n") == -1 ||
			substdio_puts(subfderr, "      -d display CRC in base 10 decimal (default is hexadecimal)\n") == -1 ||
			substdio_puts(subfderr, "      -e exclude_file (patterns to be excluded)\n") == -1 ||
			substdio_puts(subfderr, "      -f list_file (use list as input for files)\n") == -1 ||
			substdio_puts(subfderr, "      -i value (initial crc value)\n") == -1 ||
			substdio_puts(subfderr, "      -l (line mode)\n") == -1 ||
			substdio_puts(subfderr, "      -o outfile (redirect stdout to outfile)\n") == -1 ||
			substdio_puts(subfderr, "      -r (use regexp instead of fnmatch for -e option)\n") == -1 ||
			substdio_puts(subfderr, "      -R (recursively scan directories)\n") == -1 ||
			substdio_puts(subfderr, "      -s (follow symlinks)\n") == -1 ||
			substdio_puts(subfderr, "      -t tag (tag entry)\n") == -1 ||
			substdio_puts(subfderr, "      -v (verbose listing)\n") == -1 ||
			substdio_puts(subfderr, "      -w (ignore leading/trailing wsp)\n") == -1 ||
			substdio_flush(subfderr) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stderr: ");
	_exit(exitval);
}

int
main(int argc, char **argv)
{
	int             statflag = 0, displayhex = 1, c, retval,
					fdin, fdout = -1, match, nr;
	const char     *tag_arg = NULL;
	char           *ptr, *input_file = NULL, *tag_in_file, *exclude_file = NULL,
				   *output_file = NULL;
	unsigned long   linecount;
	struct substdio ssout;
	struct substdio *s;
	static char     ssoutbuf[BUFFER_OUTSIZE], ssinbuf[BUFFER_INSIZE];
	substdio        ssin;

#ifdef CHECK_LICENSE
#ifdef HAVE_DKIM
	char          **lic_argv;
#endif
#endif

	setlocale(LC_ALL, "");
#ifdef CHECK_LICENSE
#ifdef HAVE_DKIM
	if (!(lic_argv = makeargs("checklic -p "VIGILPREFIX"/etc/vigil.pub "VIGILPREFIX"/etc/vigil.lic")))
		strerr_die2x(111, FATAL, "out of memory");
	if (check_license(4, lic_argv))
		strerr_die2x(100, FATAL, "invalid vigil license "VIGILPREFIX"/etc/vigil.lic: ");
	free_makeargs(lic_argv);
#else
	strerr_die1x(100, "This program needs to be compiled with libdkim2");
#endif
#endif
	if (argc == 1)
		usage(argv[0], 0);
	/*- process all arguments */
	s = subfdout;
	while ((c = getopt(argc, argv, "bde:f:i:lo:rRst:vwW:")) != opteof) {
		switch (c)
		{
		case 'b':
			opt_b = 1;
			break;
		case 'd':
			displayhex = 0;
			break;
		case 'e':
			opt_e = 1;
			exclude_file = optarg;
			break;
		case 'f':
			input_file = optarg;
			break;
		case 'i':
			scan_long(optarg, &initial_crc);
			break;
		case 'l':
			opt_l = 1;
			break;
		case 'r':
			opt_r = 1;
			break;
		case 'R':
			opt_R = 1;
			break;
		case 's':
			opt_s = 1;
			break;
		case 't':
			tag_arg = optarg;
			break;
		case 'o':
			opt_o = 1;
			output_file = optarg;
			break;
		case 'v':
			statflag = 1;
			break;
		case 'w':
			opt_w = 1;
			break;
		case 'W':
			opt_W = 1;
			break;
		default:
			usage(argv[0], 100);
		}
	} /*- while ((c = getopt(argc, argv, "bde:f:i:lo:rRt:vw")) != opteof) */
	if (opt_e) {
		if ((c = read_exclude_file(&pbuf, exclude_file)) == -1)
			_exit(111);
		if (!c)
			opt_e = 0;
		else
		if (!stralloc_0(&pbuf))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (output_file) {
		if ((fdout = open(output_file, O_CREAT|O_TRUNC|O_WRONLY, 0644)) == -1)
			strerr_die4sys(111, FATAL, "unable to open ", output_file, " for write: ");
		substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, fdout, ssoutbuf, sizeof(ssoutbuf));
		s = &ssout;
	}
	if (input_file) {
		if ((fdin = open_read(input_file)) == -1)
			strerr_die4sys(111, FATAL, "unable to open ", input_file, " for read: ");
		substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fdin, ssinbuf, sizeof(ssinbuf));
		for (retval = 0;;) {
			if (getln(&ssin, &fscan, &match, '\n') == -1) {
				strerr_warn4(WARN, "read: ", input_file, ": ", &strerr_sys);
				close(fdin);
				return -1;
			}
			if (!fscan.len) {
				close(fdin);
				break;
			}
			if (!stralloc_0(&fscan))
				strerr_die2x(111, FATAL, "out of memory");
			fscan.len--;
			nr = fscan.len;
			/*- trim trailing \n, \r, \t, and space characters */
			while (nr > 0 && (fscan.s[nr - 1] == '\n' ||
						fscan.s[nr - 1] == '\r' ||
						fscan.s[nr - 1] == '\t' || fscan.s[nr - 1] == ' ')) {
				nr--;
			}
			fscan.s[nr] = '\0';
			/*- trim leading wsp, \t */
			for (ptr = fscan.s; *ptr == ' ' || *ptr == '\t'; ptr++, nr--);
			if (!opt_R && *ptr == '-') {
				strerr_warn4(WARN, "Entry in ", input_file, " starts with -", 0);
				continue;
			}
			c = str_rchr(fscan.s, ' ');
			tag_in_file = fscan.s[c] ? fscan.s + c + 1 : NULL;
			if (fscan.s[c]) {
				for (;c > 0;) {
					if (fscan.s[c] == ' ' || fscan.s[c] == '\t' || fscan.s[c] == '\r')
						fscan.s[c--] = '\0';
					else
						break;
				}
			}
			if (*ptr == '#')
				continue;
			retval = ((int) printcrc(ptr, &linecount, statflag, displayhex, tag_in_file, s) == -1 || retval) ? 111 : 0;
		} /*- for (retval = 0;;) */
	} else /*- if (input_file) */
		retval = 0;

	for (; optind < argc; optind++) {
		if (!opt_R && argv[optind][0] == '-')
			strerr_warn4(WARN, "Input ", argv[optind], " starts with -", 0);
		retval = ((int) printcrc(argv[optind], &linecount, statflag, displayhex, tag_arg, s) == -1 || retval) ? 111 : 0;
	}
	if (fdout != -1)
		close(fdout);
	return retval;
}
#endif
#endif /*- !CRCTAB */
#else
#error "not compiled with libqmail"
#endif
/*
 * $Log: crc.c,v $
 * Revision 1.10  2026-11-05 10:02:20+05:30  Cprogrammer
 * added multiple new options
 *
 * Revision 1.9  2026-11-01 23:51:18+05:30  Cprogrammer
 * added -b, -e exclude, -f scanfile, -o outfile, -r, -R options
 *
 * Revision 1.8  2025-06-07 18:16:47+05:30  Cprogrammer
 * added -t option to tag files/directories
 *
 * Revision 1.7  2025-05-13 19:58:44+05:30  Cprogrammer
 * fixed gcc14 errors
 *
 * Revision 1.6  2024-07-18 09:22:59+05:30  Cprogrammer
 * removed ignoring errors
 *
 * Revision 1.5  2024-05-02 20:40:01+05:30  Cprogrammer
 * fixed bug with -d option
 * fixed bug with printing group permissions
 *
 * Revision 1.4  2023-05-11 22:43:45+05:30  Cprogrammer
 * crc.c: define SYS_OPEN on the basis of openat is present or not
 *
 * Revision 1.3  2020-10-01 18:23:01+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.2  2019-04-22 23:10:00+05:30  Cprogrammer
 * replaced atoi() with scan_long()
 *
 * Revision 1.1  2019-04-14 20:58:06+05:30  Cprogrammer
 * Initial revision
 *
 */
