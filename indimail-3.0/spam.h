/*
 * $Log: spam.h,v $
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * spam.h
 *
 */
#ifndef SPAM_H
#define SPAM_H

/*
 * What character marks an inverted character class? 
 */
#define NEGATE_CLASS		'^'
#define MAXADDR             "5000"
enum
{
	MULTIPLIER = 31,
	IGNOREHASHTAB = 1,
	SPAMMERHASHTAB = 2
};

/*
 * There are two hash tables:
 * 1. Hash table with addresses from log file
 * 2. The addresses that are to be ignored 
 */
typedef struct maddr maddr;
struct maddr
{
	char           *mail;
	int             cnt;
	maddr          *next;
};

int             loadIgnoreList(char *);
int             spamReport(int, char *);
int             readLogFile(char *, int, int);
int             isIgnored(char *);
maddr          *insertAddr(int, char *);

#endif
