/*
** Copyright 1998 - 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#undef	PACKAGE
#undef	VERSION
#include	"config.h"
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<ctype.h>
#include	<fcntl.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"waitlib/waitlib.h"
#include	"authlib/authmod.h"
#include	"authlib/authsasl.h"
#include	"tcpd/spipe.h"
#include	"authlib/debug.h"

#ifndef lint
static const char rcsid[] = "$Id: pop3login.c,v 1.11 2004/09/12 23:25:56 mrsam Exp $";
#endif

extern void     pop3dcapa();
extern void pop3dlang(const char *);
extern int      have_starttls();
extern int      tls_required();
static unsigned long bytes_sent_count = 0;
static unsigned long bytes_received_count = 0;

char            tcpremoteip[28];

static char getline_buf[1024];
static size_t getline_buf_size=0;
static size_t getline_i=0;

static int safe_getc()
{
	if (getline_i >= getline_buf_size)
	{
		int n=read(0, getline_buf, sizeof(getline_buf));

		if (n < 0)
			n=0;

		if (n == 0)
			return -1;

		getline_buf_size=n;
		getline_i=0;
	}

	return (int)(unsigned char)getline_buf[getline_i++];
}

static void safe_fflush()
{
	getline_i=getline_buf_size=0;
}

static char *safe_fgets(char *buf, size_t buf_size)
{
	size_t i;

	for (i=0; i+1 < buf_size; ++i)
	{
		int ch=safe_getc();

		if (ch < 0)
		{
			if (i == 0)
				return NULL;
			break;
		}

		buf[i]=ch;
		if (ch == '\n')
		{
			++i;
			break;
		}
	}

	if (i < buf_size)
		buf[i]=0;

	return buf;
}

static void
printed(int cnt)
{
	if (cnt > 0)
		bytes_sent_count += cnt;
}

static int
starttls()
{
	int             pipefd[2];
	pid_t           p;
	int             waitstat;

	if (libmail_streampipe(pipefd))
	{
		printf("-ERR libmail_streampipe() failed.\r\n");
		return (-1);
	}
	if ((p = fork()) == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		printf("-ERR fork() failed.\r\n");
		return (-1);
	}
	if (p == 0)
	{
		char            buf1[100];
		char            dummy;

		/*
		 * Fork once more, and let the parent exit,
		 * so that courieresmtpd doesn't have this
		 * child process.
		 */

		p = fork();
		if (p == -1)
		{
			perror("pop3login: fork");
			exit(1);
		}
		if (p)
			exit(0);
		close(pipefd[0]);
		sprintf(buf1, "-localfd=%d", (int) pipefd[1]);
		if (read(pipefd[1], &dummy, 1) != 1)
			exit(0);

		/*
		 * couriertls will have the socket on fd 0,
		 * and dup stderr on fd 1 
		 */

		close(1);
		if (dup(2) == -1) {
			perror("pop3login: dup");
			exit (1);
		}
		execl(getenv("COURIERTLS"), "couriertls", buf1, "-tcpd", "-server", (char *) 0);
		perror("pop3login: exec: couriertls");
		exit (1);
	}
	printed(printf("+OK Begin SSL/TLS negotiation now.\r\n"));
	fflush(stdout);
	safe_fflush();
	close(pipefd[1]);
	close(0);
	close(1);
	if (dup(pipefd[0]) != 0 || dup(pipefd[0]) != 1)
	{
		perror("pop3login: dup");
		exit(1);
	}
	close(pipefd[0]);
	if (write(1, "", 1) == -1)
		; /*- child - exec OK now */
	while (wait(&waitstat) != p)
		;
	putenv("POP3_STARTTLS=NO");
	putenv("POP3_TLS_REQUIRED=0");
	putenv("POP3_TLS=1");
	return (0);
}

static char    *
authresp(const char *s)
{
	char           *p;
	char            buf[BUFSIZ];

	printed(printf("+ %s\r\n", s));
	fflush(stdout);
	if (safe_fgets(buf, sizeof(buf)) == 0)
		return (0);
	if ((p = strchr(buf, '\n')) == 0)
		return (0);
	if (p > buf && p[-1] == '\r')
		--p;
	*p = 0;

	p = strdup(buf);
	if (!p)
	{
		perror("pop3login: malloc");
		return (0);
	}
	return (p);
}

int
main(int argc, char **argv)
{
	int             c, disable_pass, utf8_enabled=0;
	char            buf[BUFSIZ], authservice[40];
	char           *user = 0, *p, *ptr, *q;
	const char     *ip = getenv("TCPREMOTEIP");
	const char     *port = getenv("TCPREMOTEPORT");

	disable_pass = (getenv("DISABLE_PASS") ? 1 : 0);
#ifdef HAVE_SETVBUF_IOLBF
	setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
#endif
	if (!ip || !*ip)
	{
		fprintf(stderr, "pop3login: %d: ERR: No IP address\n", getpid());
		fflush(stderr);
		exit(1);
	}
	if (!port || !*port)
	{
		fprintf(stderr, "pop3login: %d: ERR: No TCPREMOTEPORT\n", getpid());
		fflush(stderr);
		exit(1);
	}

	auth_debug_login_init();
	putenv("PROTOCOL=POP3");
	if (authmoduser(argc, argv, 60, 5))
	{
		fprintf(stderr, "pop3login: %d: INFO: Connection, ip=[%s], port=[%s]\n", getpid(), ip, port);
		printed(printf("+OK POP3 Server Ready.\r\n"));
	} else
	{
		char *p;
		if ((p = getenv("UNAUTHENTICATED")))
			fprintf(stderr, "pop3login: %d: ERR LOGIN FAILED, ip=[%s], port=[%s], user=[%s]\n", getpid(), ip, port, p);
		else
			fprintf(stderr, "pop3login: %d: ERR LOGIN FAILED, ip=[%s], port=[%s]\n", getpid(), ip, port);
		printed(printf("-ERR Login failed.\r\n"));
	}
	fflush(stdout);
	fflush(stderr);
	while (safe_fgets(buf, sizeof(buf)))
	{
		bytes_received_count += strlen(buf);
		c = 1;
		for (p = buf; *p; p++)
		{
			if (*p == '\n')
				break;

			if (*p == ' ' || *p == '\t')
				c = 0;
			if (c)
				*p = toupper((int) (unsigned char) *p);
		}

		if (*p)
			*p = 0;
		else
			while ((c = getchar()) != EOF && c != '\n')
				bytes_received_count++;
		p = strtok(buf, " \t\r");
		if (p)
		{
			auth_debug_login(1, "command=%s", p);
			if (strcmp(p, "QUIT") == 0)
			{
				printed(printf("+OK Phir Kab Miloge?\r\n"));
				fflush(stdout);
				fprintf(stderr, "pop3login: %d: INFO: LOGOUT, ip=[%s], port=[%s], rcvd=%lu, sent=%lu\n",
						getpid(), ip, port, bytes_received_count,
						bytes_sent_count);
				fflush(stderr);
				break;
			}
			if (strcmp(p, "UTF8") == 0)
			{
				printf("+OK UTF8 enabled\r\n");
				fflush(stdout);
				utf8_enabled=1;
				continue;
			}
			if (!disable_pass && strcmp(p, "USER") == 0)
			{
				if (tls_required())
				{
					printed(printf("-ERR TLS required to log in.\r\n"));
					fflush(stdout);
					continue;
				}

				p = strtok(0, "\r\n");
				if (p)
				{
					if (user)
						free(user);
					if ((user = malloc(strlen(p) + 1)) == 0)
					{
						printed(printf("-ERR Server out of memory, aborting connection.\r\n"));
						fflush(stdout);
						perror("pop3login: malloc");
						exit(1);
					}
					strcpy(user, p);
					if ((ptr = strchr(user, ':')) != (char *) 0)
					{
						*ptr = 0;
						snprintf(tcpremoteip, 28, "TCPREMOTEIP=%s", ptr + 1);
						putenv(tcpremoteip);
					}
					printed(printf("+OK Password required.\r\n"));
					fflush(stdout);
					continue;
				}
			} else
			if (strcmp(p, "CAPA") == 0)
			{
				pop3dcapa();
				continue;
			} else
			if (strcmp(p, "LANG") == 0)
			{
				pop3dlang(strtok(0, "\r\n"));
				continue;
			} else
			if (strcmp(p, "STLS") == 0)
			{
				if (!have_starttls())
				{
					printed(printf("-ERR TLS support not available.\r\n"));
					fflush(stdout);
					continue;
				}
				starttls();
				fflush(stdout);
				continue;
			} else
			if (strcmp(p, "AUTH") == 0)
			{
				char           *authtype, *authdata;
				char           *method = strtok(0, " \t\r");

				if (method)
				{
					char           *initreply = strtok(0, " \t\r");
					int             rc = authsasl(method, initreply,
												  authresp, &authtype, &authdata);
					if (tls_required())
					{
						printed(printf("-ERR TLS required to log in.\r\n"));
						fflush(stdout);
						continue;
					}
					if (rc == 0)
					{
						strcat(strcpy(authservice, "AUTHSERVICE"), getenv("TCPLOCALPORT"));
						q = getenv(authservice);
						if (!q || !*q)
							q = "pop3";
						if (utf8_enabled)
							putenv("UTF8=1");
						else
							putenv("UTF8=0");
						authmod(argc - 1, argv + 1, q, authtype, authdata);
					}
					if (rc == AUTHSASL_ABORTED)
						printed(printf("-ERR Authentication aborted.\r\n"));
					else
						printed(printf("-ERR Authentication failed.\r\n"));

					fflush(stdout);
					continue;
				} else
				{
					pop3dcapa();
					continue;
				}
			} else
			if (!disable_pass && strcmp(p, "PASS") == 0)
			{
				p = strtok(0, "\r\n");

				if (!user || p == 0)
				{
					printed(printf("-ERR USER/PASS required.\r\n"));
					fflush(stdout);
					continue;
				}

				strcat(strcpy(authservice, "AUTHSERVICE"), getenv("TCPLOCALPORT"));
				q = getenv(authservice);
				if (!q || !*q)
					q = "pop3";

				authmod_login(argc - 1, argv + 1, q, user, p);
			}
		}
		printed(printf("-ERR Invalid command.\r\n"));
		fflush(stdout);
	}
	exit(0);
	return (0);
}
