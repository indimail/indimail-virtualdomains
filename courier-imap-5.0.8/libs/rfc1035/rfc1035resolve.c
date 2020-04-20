/*
** Copyright 1998 - 2018 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	<stdio.h>
#include	"soxwrap/soxwrap.h"
#include	"rfc1035.h"
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdlib.h>
#include	<string.h>
#include	<idna.h>

static void putqbuf(const char *p, unsigned l, void *q)
{
struct querybuf *qp=(struct querybuf *)q;

	if (qp->qbuflen < sizeof(qp->qbuf) &&
			sizeof(qp->qbuf) - qp->qbuflen >= l)
		memcpy(qp->qbuf+qp->qbuflen, p, l);
	qp->qbuflen += l;
}


static struct rfc1035_reply
*rfc1035_resolve_multiple_idna(struct rfc1035_res *res,
			       int opcode,
			       const struct rfc1035_query *queries,
			       unsigned nqueries);

struct rfc1035_reply
*rfc1035_resolve_multiple(struct rfc1035_res *res,
			  int opcode,
			  const struct rfc1035_query *queries,
			  unsigned nqueries)
{
	struct rfc1035_query *idna_queries=
		(struct rfc1035_query *)malloc(nqueries * sizeof(*queries));
	struct rfc1035_reply *r;
	unsigned n;
	char **buffers;

	/*
	** Translate each label from UTF8 to IDNA.
	*/

	if (!idna_queries)
		return NULL;
	if ((buffers=(char **)malloc(nqueries * sizeof(char *))) == NULL)
	{
		free(idna_queries);
		return NULL;
	}
	for (n=0; n<nqueries; ++n)
	{
		idna_queries[n]=queries[n];
		if (idna_to_ascii_8z(idna_queries[n].name, &buffers[n], 0)
		    != IDNA_SUCCESS)
		{
			errno=EINVAL;
			while (n)
			{
				free(buffers[--n]);
			}
			free(idna_queries);
			free(buffers[n]);
			return NULL;
		}

		idna_queries[n].name=buffers[n];
	}

	r=rfc1035_resolve_multiple_idna(res, opcode, idna_queries, nqueries);
	for (n=0; n<nqueries; ++n)
		free(buffers[n]);
	free(idna_queries);
	free(buffers);
	return r;
}

static struct rfc1035_reply
*rfc1035_resolve_multiple_attempt(struct rfc1035_res *res,
				  int opcode,
				  struct querybuf *qbuf,
				  int udpfd,
				  int af,
				  const RFC1035_ADDR *sin,
				  unsigned current_timeout);

static struct rfc1035_reply
*rfc1035_resolve_multiple_attempt_tcp(struct rfc1035_res *res,
				      int opcode,
				      struct querybuf *qbuf,
				      int af,
				      const RFC1035_ADDR *sin,
				      int isaxfr,
				      unsigned current_timeout);

static struct rfc1035_reply
*rfc1035_resolve_multiple_idna(struct rfc1035_res *res,
			       int opcode,
			       const struct rfc1035_query *queries,
			       unsigned nqueries)
{
	struct	querybuf qbuf;
	int	udpfd;
	int	attempt;
	const RFC1035_ADDR *ns;
	unsigned nscount;
	unsigned current_timeout, timeout_backoff;
	unsigned nbackoff, backoff_num;
	int	af;
	unsigned i;
	int	isaxfr=0;

	struct	rfc1035_reply *rfcreply=0;
	static const char fakereply[]={0, 0, 0, RFC1035_RCODE_SERVFAIL,
				       0, 0,
				       0, 0,
				       0, 0,
				       0, 0};

	nscount=res->rfc1035_nnameservers;
	ns=res->nameservers;

	if (res->rfc1035_good_ns >= nscount)
		res->rfc1035_good_ns=0;

	qbuf.qbuflen=0;
	if ( rfc1035_mkquery(res,
		opcode, queries, nqueries, &putqbuf, &qbuf))
	{
		errno=EINVAL;
		return (0);
	}

	for (i=0; i<nqueries; i++)
		if (queries[i].qtype == RFC1035_TYPE_AXFR)
		{
			isaxfr=1;
			break;
		}

	if (isaxfr && nqueries > 1)
		return rfc1035_replyparse(fakereply, sizeof(fakereply));

	/* Prepare the UDP socket */

	if ((udpfd=rfc1035_open_udp(&af)) < 0)	return (0);

	/* Keep trying until we get an answer from a nameserver */

	current_timeout=res->rfc1035_timeout_initial;
	nbackoff=res->rfc1035_timeout_backoff;
	if (!current_timeout)	current_timeout=RFC1035_DEFAULT_INITIAL_TIMEOUT;
	if (!nbackoff)	nbackoff=RFC1035_DEFAULT_MAXIMUM_BACKOFF;

	timeout_backoff=current_timeout;

	for (backoff_num=0; backoff_num < nbackoff; backoff_num++,
		     current_timeout *= timeout_backoff)
	{
		for ( attempt=0; attempt < nscount; ++attempt)
		{
			const RFC1035_ADDR *sin=
				&ns[(res->rfc1035_good_ns+attempt) % nscount];

			rfcreply=isaxfr ?
				rfc1035_resolve_multiple_attempt_tcp
				(res,
				 opcode,
				 &qbuf,
				 af,
				 sin,
				 1,
				 current_timeout)
				: rfc1035_resolve_multiple_attempt
				(res,
				 opcode,
				 &qbuf,
				 udpfd,
				 af,
				 sin,
				 current_timeout);

			if (rfcreply)
			{
				res->rfc1035_good_ns=
					(res->rfc1035_good_ns + attempt) %
					nscount;
				break;
			}
		}

		if (rfcreply)
			break;
	}

	sox_close(udpfd);

	if (!rfcreply)
		rfcreply=rfc1035_replyparse(fakereply, sizeof(fakereply));

	return (rfcreply);
}

static struct rfc1035_reply
*rfc1035_resolve_multiple_attempt(struct rfc1035_res *res,
				  int opcode,
				  struct querybuf *qbuf,
				  int udpfd,
				  int af,
				  const RFC1035_ADDR *sin,
				  unsigned current_timeout)
{
	int	nbytes;
	char	*reply;
	struct	rfc1035_reply *rfcreply=0;

	int	sin_len=sizeof(*sin);

	int	dotcp=0;


		if (!dotcp)
		{
		/* Send the query via UDP */
		RFC1035_NETADDR	addrbuf;
		const struct sockaddr *addrptr;
		int	addrptrlen;

			if (rfc1035_mkaddress(af, &addrbuf,
				sin, htons(53),
				&addrptr, &addrptrlen))
				return NULL;

			if ((reply=rfc1035_query_udp(res, udpfd, addrptr,
				addrptrlen, qbuf->qbuf, qbuf->qbuflen, &nbytes,
					current_timeout)) == 0)
				return NULL;

		/* Parse the reply */

			rfcreply=rfc1035_replyparse(reply, nbytes);
			if (!rfcreply)
			{
				free(reply);
				return NULL;
			/* Bad response from the server, try the next one. */
			}
			rfcreply->mallocedbuf=reply;
		/*
		** If the reply came back with the truncated bit set,
		** retry the query via TCP.
		*/

			if (rfcreply->tc)
			{
				dotcp=1;
				rfc1035_replyfree(rfcreply);
			}
		}

		if (dotcp)
		{
			rfcreply=rfc1035_resolve_multiple_attempt_tcp
				(res, opcode, qbuf, af, sin, 0,
				 current_timeout);
			if (!rfcreply)
				return NULL;
		}

		memcpy(&rfcreply->server_addr, sin, sin_len);
		return (rfcreply);
}

static struct rfc1035_reply
*rfc1035_resolve_multiple_attempt_tcp(struct rfc1035_res *res,
				      int opcode,
				      struct querybuf *qbuf,
				      int af,
				      const RFC1035_ADDR *sin,
				      int isaxfr,
				      unsigned current_timeout)
{
	int nbytes;
	char *reply;
	struct rfc1035_reply *rfcreply;
	unsigned i;

	/*
	** First record in axfr will be an SOA. Start searching for the
	** trailing SOA starting with record 1. In case of multiple responses
	** we'll start looking with element 0, again.
	*/
	unsigned check_soa=1;

		int	tcpfd;
		struct	rfc1035_reply *firstreply=0, *lastreply=0;

			if ((tcpfd=rfc1035_open_tcp(res, sin)) < 0)
				return NULL;	/*
						** Can't connect via TCP,
						** try the next server.
						*/

			reply=rfc1035_query_tcp(res, tcpfd, qbuf->qbuf,
				qbuf->qbuflen, &nbytes, current_timeout);

			if (!reply)
			{
				sox_close(tcpfd);
				return NULL;
			}

			rfcreply=rfc1035_replyparse(reply, nbytes);
			if (!rfcreply)
			{
				free(reply);
				sox_close(tcpfd);
				return NULL;
			}
			rfcreply->mallocedbuf=reply;
			firstreply=lastreply=rfcreply;
			while (isaxfr && rfcreply->rcode == 0)
			{
				for (i=check_soa; i<rfcreply->ancount; ++i)
				{
					if (rfcreply->anptr[i].rrtype ==
					    RFC1035_TYPE_SOA)
						break;
				}

				if (i < rfcreply->ancount)
					break; /* Found trailing SOA */

				check_soa=0;

				if ((reply=rfc1035_recv_tcp(res,
					tcpfd, &nbytes, current_timeout))==0)
					break;

				rfcreply=rfc1035_replyparse(reply, nbytes);
				if (!rfcreply)
				{
					free(reply);
					rfc1035_replyfree(firstreply);
					firstreply=0;
					break;
				}
				rfcreply->mallocedbuf=reply;
				lastreply->next=rfcreply;
				lastreply=rfcreply;
			}
			sox_close(tcpfd);

			return firstreply;
}

struct rfc1035_reply *rfc1035_resolve(
	struct rfc1035_res *res,
	int opcode,
	const char *name,
	unsigned qtype,
	unsigned qclass)
{
struct rfc1035_query q;

	q.name=name;
	q.qtype=qtype;
	q.qclass=qclass;
	return (rfc1035_resolve_multiple(res, opcode, &q, 1));
}
