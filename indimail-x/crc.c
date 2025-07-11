/*
 * $Id: crc.c,v 1.8 2025-06-07 18:16:47+05:30 Cprogrammer Exp mbhangui $
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
#ifdef HAVE_QMAIL
#include <subfd.h>
#include <strerr.h>
#include <substdio.h>
#include <stralloc.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <sgetopt.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: crc.c,v 1.8 2025-06-07 18:16:47+05:30 Cprogrammer Exp mbhangui $";
#endif

#define MAXBUF 4096
static char     strnum[FMT_ULONG];
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
	return rem & 0xFFFFFFFF;	/* Make it run on 64-bit machine.  */
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
#else /* !CRCTAB */
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

static int
print_perm(unsigned int perm)
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
	/* owner */
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

	/* group */
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

	/* others */
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
	if (substdio_put(subfdout, " ", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_puts(subfdout, string) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(subfdout, " ", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	return (0);
}

static void
stats(const char *file, struct stat *statptr, const char *tag)
{

	struct passwd  *entry;
	struct group   *group_entry;
	int             i;
	char            a_time[50];
	static int      prev_uid = -9999;
	static int      prev_gid = -9999;

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
	print_perm(statptr->st_mode);
	if (substdio_put(subfdout, owner.s, owner.len) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(subfdout, "\t", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(subfdout, group.s, group.len) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(subfdout, "\t", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_puts(subfdout, a_time + 4) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_put(subfdout, " ", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	else
	if (substdio_puts(subfdout, (char *) file ) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	if (tag) {
		if (substdio_put(subfdout, ",tag=", 5))
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		else
		if (substdio_puts(subfdout, tag))
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	}
	if (substdio_put(subfdout, "\n", 1) == -1)
		strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	return;
}

/*
 * Calculate and print the checksum and length in bytes
 * of file FILE, or of the standard input if FILE is "-".
 * If PRINT_NAME is nonzero, print FILE next to the checksum and size.
 * TEXT indicates whether file is text or binary.
 * Return 0 if successful, -1 if an error occurs.
 */
int
printcrc(const char *file, unsigned long *lcount,
		int statflag, int displayhex, const char *tag)
{
	int             fd;
	int             nr;
	char           *ptr;
	char            buf[MAXBUF + 1];
	unsigned long   crc;
	unsigned long   length = 0, bytes_read;
	struct stat     statbuf, statbuf_t;

	if (stat(file, &statbuf)) {
		strerr_warn3(WARN, file, ": ", &strerr_sys);
		return (-1);
	}
	if (lcount)
		*lcount = 0l;
	if (!S_ISREG(statbuf.st_mode)) {
		memset((char *) &statbuf_t, 0, sizeof (struct stat));
		statbuf_t.st_mode = statbuf.st_mode;
		statbuf_t.st_ino = statbuf.st_ino;
		statbuf_t.st_dev = statbuf.st_dev;
		statbuf_t.st_rdev = statbuf.st_rdev;
		statbuf_t.st_size = statbuf.st_size;
#if 0
		statbuf_t.st_atime = statbuf.st_atime;
		statbuf_t.st_mtime = statbuf.st_mtime;
		statbuf_t.st_ctime = statbuf.st_ctime;
#endif
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
				if (substdio_put(subfdout, "0x", 2) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
				for (nr = 8 - nr; nr; nr--) {
					if (substdio_put(subfdout, "0", 1) == -1)
						strerr_die2sys(111, FATAL, "unable to write to stdout: ");
				}
			}
			if (substdio_puts(subfdout, strnum) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			if (statflag)
				stats(file, &statbuf, tag);
			else
			if (substdio_put(subfdout, "\n", 1) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			if (substdio_flush(subfdout) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		}
		return (crc);
	}
	/*- open the file and do a silent crc on it */
#if defined(LINUX) || defined(FREEBSD)
	/*- prevent hooking the open system call */
#if defined(SYS_openat) && defined(AT_FDCWD)
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_openat,AT_FDCWD,FILE,FLAG,MODE)
#else
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_open,FILE,FLAG,MODE)
#endif
	if ((fd = SYS_OPEN(file, O_RDONLY, 0)) == -1) {
#else
	if ((fd = open(file, O_RDONLY)) == -1) {
#endif
		strerr_warn3(WARN, file, ": ", &strerr_sys);
		return (-1);
	}
	for (crc = initial_crc;;) {
		if ((nr = read(fd, (char *) buf, MAXBUF)) == -1) {
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
		buf[nr] = 0;
		length += nr;
		for (ptr = buf; nr > 0; ptr++, nr--) {
			if (lcount && *ptr == '\n')
				(*lcount)++;
			crc = (crc << 8) ^ crctab[((crc >> 24) ^ *ptr) & 0xFF];
		}
	}
	bytes_read = length;
	while (bytes_read > 0) {
		crc = (crc << 8) ^ crctab[((crc >> 24) ^ bytes_read) & 0xFF];
		bytes_read >>= 8;
	}
	crc = ~crc & 0xFFFFFFFF;
	(void) close(fd);
	if (statflag != -1) {
		strnum[nr = (displayhex ? fmt_xlong : fmt_ulong) (strnum, crc)] = 0;
		if (displayhex) {
			if (substdio_put(subfdout, "0x", 2) == -1)
				strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			for (nr = 8 - nr; nr; nr--) {
				if (substdio_put(subfdout, "0", 1) == -1)
					strerr_die2sys(111, FATAL, "unable to write to stdout: ");
			}
		}
		if (substdio_puts(subfdout, strnum) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (statflag)
			stats(file, &statbuf, tag);
		else
		if (substdio_put(subfdout, "\n", 1) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
		if (substdio_flush(subfdout) == -1)
			strerr_die2sys(111, FATAL, "unable to write to stdout: ");
	}
	return (crc);
}

#ifdef MAIN

static void
usage(int exitval)
{
	if (substdio_puts(subfderr, "crc:  -v (verbose listing)\n") == -1)
		strerr_die2sys(111, FATAL, "unable to write to stderr: ");
	else
	if (substdio_puts(subfderr, "      -d display CRC in base 10 decimal (default is hexadecimal)\n") == -1)
		strerr_die2sys(111, FATAL, "unable to write to stderr: ");
	else
	if (substdio_putsflush(subfderr, "      -i value (initial crc value)\n") == -1)
		strerr_die2sys(111, FATAL, "unable to write to stderr: ");
	_exit(exitval);
}
int
main(int argc, char **argv)
{
	int             statflag = 0, displayhex = 1, c, retval;
	const char     *tag = NULL;
	unsigned long   linecount;

	if (argc == 1)
		usage(0);
	/*- process all arguments */
	while ((c = getopt(argc, argv, "vdi:t:")) != opteof) {
		switch (c)
		{
		case 'd':
			displayhex = 0;
			break;
		case 'v':
			statflag = 1;
			break;
		case 'i':
			scan_long(optarg, &initial_crc);
			break;
		case 't':
			tag = optarg;
			break;
		default:
			usage(100);
		}
	}
	for (retval = 0; optind < argc; optind++)
		retval = (printcrc(argv[optind], &linecount, statflag, displayhex, tag) == -1 || retval ? 100 : 0);
	return retval;
}
#endif
#endif /* !CRCTAB */
/*
 * $Log: crc.c,v $
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
