/*
 * $Log: lockfile.c,v $
 * Revision 1.1  2019-04-18 08:27:50+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     sccsid[] = "$Id: lockfile.c,v 1.1 2019-04-18 08:27:50+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef FILE_LOCKING
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef USE_SEMAPHORES
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#ifdef HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif

static struct sembuf op_lock[2] = {
	{0, 0, 0}, /*- wait for sem#0 to become zero */
	{0, 1, SEM_UNDO} /*- then increment sem#0 by 1 */
};

static struct sembuf op_unlock[1] = {
	{0, -1, (IPC_NOWAIT | SEM_UNDO)} /*- decrement sem#0 by 1 (sets it to 0) */
};

static void     SigAlarm();

int
lockcreate(char *filename, char proj)
{
	key_t           key;
	int             semid;

	key = ftok(filename, proj);
	for(;;) {
		if ((semid = semget(key, 1, IPC_CREAT | 0666)) == -1) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			return (-1);
		} else
			break;
	}
	return (semid);
}

int
lockremove(int semid)
{
	return (semctl(semid, 0, IPC_RMID, 0));
}

int             GotAlarm;
int
get_write_lock(int semid)
{
	void            (*pstat) ();

	GotAlarm = 0;
	if ((pstat = signal(SIGALRM, SigAlarm)) == SIG_ERR)
		return (-1);
	alarm(1);
	for(;;) {
		if (semop(semid, &op_lock[0], 2) == -1) {
#ifdef ERESTART
			if ((errno == EINTR || errno == ERESTART) && GotAlarm)
#else
			if (errno == EINTR && GotAlarm)
#endif
			{
				(void) signal(SIGALRM, pstat);
				GotAlarm = 0;
				errno = EAGAIN;
				return (-1);
			} else
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			return (-1);
		} else
			break;
	}
	alarm(0);
	return (semid);
}

int
ReleaseLock(int fd)
{
	return (semop(fd, &op_unlock[0], 1));
}

/*- Dummy Function */
int
RemoveLock(char *filename, char proj)
{
	return (0);
}

static void
SigAlarm()
{
	GotAlarm = 1;
	return;
}
#elif defined(USE_LINKLOCK)
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_QMAIL
#include <fmt.h>
#include <strerr.h>
#include <stralloc.h>
#endif

static stralloc lockfile = {0}, tmplock = {0};
char            strnum[FMT_ULONG];

static void
die_nomem()
{
	strerr_warn1("lockcreate: out of memory", 0);
	_exit(111);
}

int
lockcreate(char *filename, char proj)
{
	struct stat     statbuf;
	char            strnum1[FMT_ULONG], strnum2[FMT_ULONG];
	int             fd, tmperrno, i;
	pid_t           pid, mypid;
	time_t          file_age, start_time, secs;
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_WRLCK, SEEK_SET};
#else
	struct flock    fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
#endif

	start_time = time(0);
	strnum1[i = fmt_uint(strnum1, proj)] = 0;
	if (!stralloc_copys(&tmplock, filename) ||
			!stralloc_catb(&tmplock, ".pre.", 5) ||
			!stralloc_catb(&tmplock, strnum1, i) || !stralloc_0(&tmplock))
		die_nomem();
	if ((fd = open(tmplock.s, O_CREAT, 0666)) == -1)
		return (-1);
	close(fd);
	if (!stralloc_copys(&lockfile, filename) ||
			!stralloc_catb(&lockfile, ".pre.", 5) ||
			!stralloc_catb(&lockfile, strnum1, i) || !stralloc_0(&lockfile))
		die_nomem();
	for (mypid = getpid();;) {
		if (!link(tmplock.s, lockfile.s)) {
			if ((fd = open(lockfile.s, O_WRONLY, 0)) == -1) {/*- this should never happen */
				tmperrno = errno;
				secs = time(0) - start_time;
				if (tmperrno == ENOENT)
					continue;
				else {
					errno = tmperrno;
					return (-1);
				}
			}
			secs = time(0) - start_time;
			if (secs) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_ulong(strnum2, secs)] = 0;
				strerr_warn3(strnum1, ": lockfile ", strnum2, 0);
			}
			return (fd);
		} else
		if (errno == EEXIST) {
			if (stat(lockfile.s, &statbuf)) {
				usleep(500);
				continue;
			} else {
				file_age = time(0) - statbuf.st_atime;
				if (file_age > 5) {
					if ((fd = open(lockfile.s, O_RDWR, 0)) == -1)
						return (-1);
					if (read(fd, (char *) &pid, sizeof(pid_t)) == -1) {
						close(fd);
						return (-1);
					}
					if (pid == mypid) {
						close(fd);
						errno = EDEADLK;
						return (-1);
					} else
					if (!pid || kill(pid, 0)) {
						fl.l_pid = getpid();
						for (;;) {
							if (fcntl(fd, F_SETLKW, &fl) == -1) {
#ifdef ERESTART
								if (errno == EINTR || errno == ERESTART)
#else
								if (errno == EINTR)
#endif
									continue;
								secs = time(0) - start_time;
								if (secs) {
									strnum1[fmt_ulong(strnum1, getpid())] = 0;
									strnum2[fmt_ulong(strnum2, secs)] = 0;
									strerr_warn3(strnum, ": lockfile ", strnum2, 0);
								}
								close(fd);
								errno = EAGAIN;
								return (-1);
							}
							break;
						}
						fl.l_type = F_UNLCK;
						if (unlink(lockfile.s)) {
							if (fcntl(fd, F_SETLK, &fl) == -1);
							close(fd);
							return (-1);
						}
						if (fcntl(fd, F_SETLK, &fl) == -1) {
							close(fd);
							return (-1);
						}
						close(fd);
						continue;
					} else { /*- some process still holds lock */
						secs = time(0) - start_time;
						if (secs) {
							strnum1[fmt_ulong(strnum1, getpid())] = 0;
							strnum2[fmt_ulong(strnum2, secs)] = 0;
							strerr_warn3(strnum, ": lockfile ", strnum2, 0);
						}
						close(fd);
						errno = EAGAIN;
						/*- return (-1); -*/
					}
				} /*- if (file_age > 5) */
			} /* if (!stat(lockfile, &statbuf)) */
			usleep(500);
		} else
		if (errno == ENOENT) {
			if ((fd = open(tmplock.s, O_CREAT, 0666)) == -1)
				return (-1);
			close(fd);
		} else	
			return (-1);
	}
}

int
get_write_lock(int fd)
{
	pid_t           pid;

	pid = getpid();
	if (write(fd, (char *) &pid, sizeof(pid_t)) == -1) {
		close(fd);
		return (-1);
	}
	return (fd);
}

int
ReleaseLock(int fd)
{
	return (close(fd));
}

int
RemoveLock(char *filename, char proj)
{
	int             i;

	strnum[i = fmt_uint(strnum, proj)] = 0;
	if (!stralloc_copys(&lockfile, filename) ||
			!stralloc_catb(&lockfile, ".pre.", 5) ||
			!stralloc_catb(&lockfile, strnum, i) || !stralloc_0(&lockfile))
		die_nomem();
	return (access(lockfile.s, F_OK) ? 0 : unlink(lockfile.s));
}
#elif defined(USE_FCNTLLOCK)
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#include <fmt.h>
#include <strerr.h>
#endif

static stralloc tmplock = {0};
char            strnum[FMT_ULONG];

static void
die_nomem()
{
	strerr_warn1("lockcreate: out of memory", 0);
	_exit(111);
}

int
lockcreate(char *filename, char proj)
{
	int             fd, i;
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_WRLCK, SEEK_SET};
#else
	struct flock    fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
#endif

	strnum[i = fmt_uint(strnum, proj)] = 0;
	if (!stralloc_copys(&tmplock, filename) ||
			!stralloc_catb(&tmplock, ".pre.", 5) ||
			!stralloc_catb(&tmplock, strnum, i) || !stralloc_0(&tmplock))
		die_nomem();
	if ((fd = open(tmplock.s, O_CREAT|O_WRONLY, 0644)) == -1)
		return (-1);
	fl.l_pid = getpid();
	for (;;) {
		if (fcntl(fd, F_SETLKW, &fl) == -1) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			return (-1);
		}
		break;
	}
	return (fd);
}

int
get_write_lock(int fd)
{
	return (fd);
}

int
ReleaseLock(int fd)
{
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_UNLCK, SEEK_SET};
#else
	struct flock    fl = {F_UNLCK, SEEK_SET, 0, 0, 0};
#endif

	if (fd == -1)
		return (-1);
	fl.l_pid = getpid();
	if (fcntl(fd, F_SETLK, &fl) == -1);
	return (close(fd));
}

int
RemoveLock(char *filename, char proj)
{
	int             i;

	strnum[i = fmt_uint(strnum, proj)] = 0;
	if (!stralloc_copys(&tmplock, filename) ||
			!stralloc_catb(&tmplock, ".pre.", 5) ||
			!stralloc_catb(&tmplock, strnum, i) || !stralloc_0(&tmplock))
		die_nomem();
	if (!access(tmplock.s, F_OK))
		return (unlink(tmplock.s));
	else
		return (0);
}
#elif defined(USE_FLOCK)
int
lockcreate(char *filename, char proj)
{
	int             fd, i;

	strnum[i = fmt_uint(strnum, proj)] = 0;
	if (!stralloc_copys(&tmplock, filename) ||
			!stralloc_catb(&tmplock, ".pre.", 5) ||
			!stralloc_catb(&tmplock, strnum, i) || !stralloc_0(&tmplock))
		die_nomem();
	if ((fd = open(tmplock.s, O_CREAT|O_WRONLY, 0644)) == -1)
		return (-1);
	if (lockf(fd, F_LOCK, 0)) {
		close(fd);
		return (-1);
	}
	return (fd);
}

int
get_write_lock(int fd)
{
	return (fd);
}

int
ReleaseLock(int fd)
{
	if (fd == -1)
		return (-1);
	if (!lockf(fd, F_ULOCK, 0))
		return (close(fd));
	return (close(fd));
}

int
RemoveLock(char *filename, char proj)
{
	int             i;

	strnum[i = fmt_uint(strnum, proj)] = 0;
	if (!stralloc_copys(&tmplock, filename) ||
			!stralloc_catb(&tmplock, ".pre.", 5) ||
			!stralloc_catb(&tmplock, strnum, i) || !stralloc_0(&tmplock))
		die_nomem();
	if (!access(tmplock.s, F_OK))
		return (unlink(tmplock.s));
	else
		return (0);
}
#endif /*- #ifdef USE_FLOCK */
#endif /*- #ifdef FILE_LOCKING */
