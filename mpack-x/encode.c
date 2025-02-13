/*
 * $Id: encode.c,v 1.3 2025-01-22 15:46:06+05:30 Cprogrammer Exp mbhangui $
 *
 * (C) Copyright 1993,1994 by Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Carnegie
 * Mellon University not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  Carnegie Mellon University makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

extern char    *magic_look(FILE * infile);
extern char    *os_genid(void);
extern FILE    *os_createnewfile(char *fname);
extern char    *md5digest(FILE * infile, long int *len);
extern int      to64(FILE *, FILE *, long);

#define NUMREFERENCES 4

/*-
 * Encode a file into one or more MIME messages, each
 * no larger than 'maxsize'.  A 'maxsize' of zero means no size limit.
 * If 'applefile' is non-null, it is the first part of a multipart/appledouble
 * pair.
 */
int
encode(FILE * infile, FILE * applefile, char *fname, FILE * descfile, char *subject, char *headers, long int maxsize,
	   char *typeoverride, char *outfname)
{
	char           *type;
	FILE           *outfile;
	char           *cleanfname, *p;
	char           *digest, *appledigest = (char *) 0;
	long            filesize, l, written;
	int             thispart, numparts = 1;
	int             wrotefiletype = 0;
	char            boundary[128];
	char           *multipartid, *msgid, *referenceid[NUMREFERENCES];
	char            buf[1024];
	int             i;

	/*
	 * Clean up fname for printing 
	 */
	snprintf(boundary, sizeof(boundary), "----=_NextPart_%ld_%d", time(0), (int) getpid());
	cleanfname = fname;
#ifdef __riscos
	/*
	 * This filename-cleaning knowledge will probably be moved to the os layer in a future version.  
	 */
	if ((p = strrchr(cleanfname, '.')))
		cleanfname = p + 1;
#else
	if ((p = strrchr(cleanfname, '/')))
		cleanfname = p + 1;
	if ((p = strrchr(cleanfname, '\\')))
		cleanfname = p + 1;
#endif
	if ((p = strrchr(cleanfname, ':')))
		cleanfname = p + 1;
	/*
	 * Find file type 
	 */
	if (typeoverride)
		type = typeoverride;
	else
		type = magic_look(infile);
	/*
	 * Compute MD5 digests 
	 */
	digest = md5digest(infile, &filesize);
	if (applefile)
	{
		appledigest = md5digest(applefile, &l);
		filesize += l;
	}
	/*
	 * See if we have to do multipart 
	 */
	if (maxsize)
	{
		filesize = (filesize / 54) * 73;	/*- Allow for base64 expansion */
		/*
		 * Add in size of desc file 
		 */
		if (descfile)
		{
			free(md5digest(descfile, &l));	/*- XXX */
			filesize += l;
		}
		numparts = (filesize - 1000) / maxsize + 1;
		if (numparts < 1)
			numparts = 1;
	}
	multipartid = os_genid();
	for (i = 0; i < NUMREFERENCES; i++)
		referenceid[i] = 0;
	for (thispart = 1; thispart <= numparts; thispart++)
	{
		written = 0;
		/*
		 * Open output file 
		 */
		if (numparts == 1)
		{
			outfile = os_createnewfile(outfname);
		} else
		{
#ifdef __riscos
			/*
			 * Arrgh, riscos uses '.' as directory separator 
			 */
			sprintf(buf, "%s/%02d", outfname, thispart);
#else
			sprintf(buf, "%s.%02d", outfname, thispart);
#endif
			outfile = os_createnewfile(buf);
		}
		if (!outfile)
		{
			perror(buf);
			return 1;
		}
		msgid = os_genid();
		fprintf(outfile, "Message-ID: <%s>\n", msgid);
		fprintf(outfile, "Mime-Version: 1.0\n");
		if (headers)
			fputs(headers, outfile);
		if (numparts > 1)
		{
			fprintf(outfile, "Subject: %s (%02d/%02d)\n", subject, thispart, numparts);
			if (thispart == 1)
				referenceid[0] = msgid;
			else
			{
				/*
				 * Put out References: header pointing to previous parts 
				 */
				fprintf(outfile, "References: <%s>\n", referenceid[0]);
				for (i = 1; i < NUMREFERENCES; i++)
				{
					if (referenceid[i])
						fprintf(outfile, "\t <%s>\n", referenceid[i]);
				}
				for (i = 2; i < NUMREFERENCES; i++)
				{
					referenceid[i - 1] = referenceid[i];
				}
				referenceid[NUMREFERENCES - 1] = msgid;
			}
			fprintf(outfile, "Content-Type: message/partial; number=%d; total=%d;\n", thispart, numparts);
			fprintf(outfile, "\t id=\"%s\"\n", multipartid);
			fprintf(outfile, "\n");
		}
		if (thispart == 1)
		{
			if (numparts > 1)
			{
				fprintf(outfile, "Message-ID: <%s>\n", multipartid);
				fprintf(outfile, "MIME-Version: 1.0\n");
				written += (33 + strlen(multipartid));
			}
			fprintf(outfile, "Subject: %s\n", subject);
			fprintf(outfile, "Content-Type: multipart/mixed; boundary=\"%s\"\n\n", boundary);
			fprintf(outfile, "This is a multi-part message in MIME format.\n\n");
			written = (101 + strlen(subject) + strlen(boundary));
			/*
			 * Spit out description section 
			 */
			if (descfile)
			{
				fprintf(outfile, "--%s\n\n", boundary);
				while (fgets(buf, sizeof(buf), descfile))
				{
					/*
					 * Strip multiple leading dashes as they may become MIME boundaries 
					 */
					p = buf;
					if (*p == '-')
					{
						while (p[1] == '-')
							p++;
					}
					fputs(p, outfile);
					written += strlen(p);
				}
				fprintf(outfile, "\n");
			}
			fprintf(outfile, "--%s\n", boundary);
			if (applefile)
			{
				fprintf(outfile, "Content-Type: multipart/appledouble; boundary=\"=\"; name=\"%s\"\n", cleanfname);
				fprintf(outfile, "Content-Disposition: inline; filename=\"%s\"\n", cleanfname);
				fprintf(outfile, "\n\n--=\n");
				fprintf(outfile, "Content-Type: application/applefile\n");
				fprintf(outfile, "Content-Transfer-Encoding: base64\n");
				fprintf(outfile, "Content-MD5: %s\n\n", appledigest);
				free(appledigest);
				written += 100;
			}
		}
		if (applefile && !feof(applefile))
		{
			if (written == maxsize)
				written--;		/*- avoid a nasty fencepost error */
			written += to64(applefile, outfile, (thispart == numparts) ? 0 : (maxsize - written));
			if (!feof(applefile))
			{
				fclose(outfile);
				continue;
			}

			fprintf(outfile, "\n--=\n");
		}
		if (!wrotefiletype++)
		{
			fprintf(outfile, "Content-Type: %s; name=\"%s\"\n", type, cleanfname);
			fprintf(outfile, "Content-Transfer-Encoding: base64\n");
			fprintf(outfile, "Content-Disposition: inline; filename=\"%s\"\n", cleanfname);
			fprintf(outfile, "Content-MD5: %s\n\n", digest);
			written += (114 + strlen(type) + 2 * strlen(cleanfname) + strlen(digest));
			free(digest);
		}
		if (written == maxsize)
			written--;			/*- avoid a nasty fencepost error */
		written += to64(infile, outfile, (thispart == numparts) ? 0 : (maxsize - written));
		if (thispart == numparts)
		{
			if (applefile)
				fprintf(outfile, "\n--=--\n");
			fprintf(outfile, "\n--%s--\n", boundary);
		}
		fclose(outfile);
	}
	return 0;
}
/*-
 * $Log: encode.c,v $
 * Revision 1.3  2025-01-22 15:46:06+05:30  Cprogrammer
 * fix gcc14 errors
 *
 * Revision 1.2  2004-01-06 14:55:01+05:30  Manny
 * fixed compilation warnings
 *
 * Revision 1.1  2004-01-06 12:44:12+05:30  Manny
 * Initial revision
 *
 */
