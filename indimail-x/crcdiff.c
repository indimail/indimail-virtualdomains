/*
 * $Log: crcdiff.c,v $
 * Revision 1.5  2024-09-05 19:01:08+05:30  Cprogrammer
 * updated usage string
 *
 * Revision 1.4  2024-07-18 09:17:52+05:30  Cprogrammer
 * added -C option to display checksum
 *
 * Revision 1.3  2024-05-02 20:54:42+05:30  Cprogrammer
 * display L1, L2 changes
 * added -c, -s options
 *
 * Revision 1.2  2023-01-22 10:35:57+05:30  Cprogrammer
 * fixed data type passed to printf
 *
 * Revision 1.1  2019-04-14 20:58:10+05:30  Cprogrammer
 * Initial revision
 *
 * This progam will compare two crc lists and report the differences.
 *
 * By Jon Zeeff (zeeff@b-tech.ann-arbor.mi.us)
 *
 * Permission is granted to use this in any manner provided that
 * 1) the copyright notice is left intact,
 * 2) you don't hold me responsible for any bugs and
 * 3) you mail me any improvements that you make.
 *
 *
 * report:
 *     corrupt  - crc changed w/o date change
 *     replaced - crc + date changed
 *     permiss  - permissions changed
 *     own/grp  - owner or group changed
 *     deleted  -
 *     added    -
 *  Print the info for the new file except for deleted.
 *
 * Use:
 *
 * find / -print | sort | xargs crc -v > crc_file
 *
 * to generate a crc list (crc.c should accompany this source).
 *
 * Assume that no files have tabs or spaces in the name.
 * A crc line is like this
 * 0x5d784e43 dr-xr-xr-x root	root	Jun 11 07:57:46 2024 /usr/bin
 *
 -*/

/*- max size of line */

#define BUF_SIZE 1124
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <getopt.h>
#include <stdlib.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: crcdiff.c,v 1.5 2024-09-05 19:01:08+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
usage(int exitval)
{
	fprintf(stderr, "crcdiff [-Cs] [-c critical_list] file1 file2)\n");
	exit(exitval);
}

int
check_critical_list(FILE *fp, char *fn_line)
{
	char           *ptr;
	char            line[BUF_SIZE];
	int             len;

	if (!fp)
		return 0;
	rewind(fp);
	if (!(ptr = strrchr(fn_line, ' ')))
		return 0;
	ptr++;
	if (!*ptr)
		return 0;
	for (;;) {
		if (!fgets(line, BUF_SIZE, fp)) {
			if (feof(fp))
				break;
			perror("fgets");
			return (1);
		}
		len = strlen(line);
		if (!strncmp(ptr, line, len))
			return 1;
	}
	return 0;
}

int
main(int argc, char **argv)
{

	char           *new_ptr, *old_ptr, *critical_file_list = NULL;
	FILE           *newfp, *oldfp, *fp = NULL;
	int             match, c, sorted = 0, crit_flag, display_checksum = 0;
	unsigned long   add_count, del_count, corrupt_count, perm_count,
					owner_count, mod_count, crit_l1_count, crit_l2_count, count;
	char            new_line[BUF_SIZE], old_line[BUF_SIZE];

	while ((c = getopt(argc, argv, "c:Cs")) != -1) {
		switch (c)
		{
		case 'C':
			display_checksum = 1;
			break;
		case 'c':
			critical_file_list = optarg;
			if (!(fp = fopen(critical_file_list, "r"))) {
				fprintf(stderr, "%s: %s\n", critical_file_list, strerror(errno));
				return 1;
			}
			break;
		case 's':
			sorted = 1;
			break;
		default:
			usage(100);
		}
	}
	/*-
       	If line =, read new line from each file
       else
		If date/perm/crc change, report and read new line from each file
       else
        If old_line < new_line, report file deleted, read old line
       else
        report new line as added
              read new_line
        loop
      -*/

	if ((argc - optind) != 2)
		usage(1);
	if (!(newfp = fopen(argv[optind + 1], "r"))) {
		perror(argv[optind + 1]);
		return (1);
	}
	if (!(oldfp = fopen(argv[optind], "r"))) {
		perror(argv[optind]);
		return (1);
	}
	add_count = del_count = corrupt_count = perm_count = owner_count = crit_l1_count = crit_l2_count = mod_count = 0l;
	for (;;) {
		if (!fgets(old_line, BUF_SIZE, oldfp)) {
			if (feof(oldfp))
				break;
			perror("fgets");
			return (1);
		}
		/*- old filename */
		if (!(old_ptr = strrchr(old_line, ' '))) {
			fprintf(stderr, "Error in input data\n");
			exit(1);
		}
		for (count = match = 0;; count++) {
			if (!fgets(new_line, BUF_SIZE, newfp)) {
				if (feof(newfp)) {
					rewind(newfp);
					break;
				}
				perror("fgets");
				return (1);
			}

			if (!strcmp(old_line, new_line)) {
				match = 1;
				break;
			}
			/*- new filename */
			if (!(new_ptr = strrchr(new_line, ' '))) {
				fprintf(stderr, "Error in input data\n");
				exit(1);
			}
			/*- Compare just the file names */
			c = strcmp(old_ptr, new_ptr);
			if (sorted && c < 0) {
				rewind(newfp);
				printf("deleted    %s", old_line + (display_checksum ? 0 : 11));
				fflush(stdout);
				if (fp && check_critical_list(fp, old_line)) {
					fprintf(stderr, "WARN L2 <  %s", old_line + 11);
					crit_l2_count++;
				}
				del_count++;
				break;
			}
			if (!c) {
				match = 1;
				new_ptr = strrchr(new_line, '\t'); /*- timestamp */
				old_ptr = strrchr(old_line, '\t'); /*- timestamp */
				if (!new_ptr || !old_ptr) {
					fprintf(stderr, "Error in input data\n");
					return (1);
				}
				/*- check crc change */
				if (strncmp(new_line, old_line, 10)) {
					if (!strcmp(new_ptr, old_ptr)) {
						printf("corrupt    %s", new_line + (display_checksum ? 0 : 11));
						if (fp && check_critical_list(fp, new_line)) {
							fprintf(stderr, "WARN L2 >  %s", new_line + 11);
							crit_l2_count++;
						}
						corrupt_count++;
					} else {
						printf("replaced   %s", new_line + (display_checksum ? 0 : 11));
						fflush(stdout);
						if (fp && check_critical_list(fp, new_line)) {
							fprintf(stderr, "WARN L2 >  %s", new_line + 11);
							crit_l2_count++;
						}
						mod_count++;
					}
				}
				/* 0xb63052e4 -rw-r--r-- root\troot\tNov 13 11:06:01 2018 /etc/hosts */
				/*- check permission change */
				crit_flag = 0;
				if (strncmp(new_line + 11, old_line + 11, 10)) {
					printf("permiss <  %s", old_line + (display_checksum ? 0 : 11));
					printf("permiss >  %s", new_line + (display_checksum ? 0 : 11));
					fflush(stdout);
					/* setuid, setgid bit changed */
					if (*(old_line + 14) == 's' || *(old_line + 17) == 's') {
						fprintf(stderr, "WARN L1 <  %s", old_line + 11);
						if (!crit_flag) {
							crit_l1_count++;
							crit_flag = 1;
						}
					}
					if (*(new_line + 14) == 's' || *(new_line + 17) == 's') {
						fprintf(stderr, "WARN L1 >  %s", new_line + 11);
						if (!crit_flag) {
							crit_l1_count++;
							crit_flag = 1;
						}
					}
					perm_count++;
					fflush(stdout);
				}
				/*- check  owner/group */
				if (strncmp(new_line + 22, old_line + 22, new_ptr - new_line - 21)) {
					printf("own/grp <  %s", old_line + (display_checksum ? 0 : 11));
					printf("own/grp >  %s", new_line + (display_checksum ? 0 : 11));
					fflush(stdout);
					/* owner/group changes to setuid, setgid files */
					if (*(old_line + 14) == 's' || *(old_line + 17) == 's') {
						fprintf(stderr, "WARN L1 <  %s", old_line + 11);
						if (!crit_flag) {
							crit_l1_count++;
							crit_flag = 1;
						}
					}
					if (*(new_line + 14) == 's' || *(new_line + 17) == 's') {
						fprintf(stderr, "WARN L1 >  %s", new_line + 11);
						if (!crit_flag) {
							crit_l1_count++;
							crit_flag = 1;
						}
					}
					owner_count++;
					fflush(stdout);
				}
				break;
			} else
			if (!count)
				rewind(newfp);
		} /* for (count = match = 0;; count++) */
		if (!sorted && !match) {
			printf("deleted    %s", old_line + (display_checksum ? 0 : 11));
			fflush(stdout);
			if (fp && check_critical_list(fp, new_line)) {
				fprintf(stderr, "WARN L2 <  %s", old_line + 11);
				crit_l2_count++;
			}
			del_count++;
		}
	}
	rewind(newfp);
	rewind(oldfp);
	for (;;) {
		if (!fgets(new_line, BUF_SIZE, newfp)) {
			if (feof(newfp))
				break;
			perror("fgets");
			exit (1);
		}
		if (!(new_ptr = strrchr(new_line, ' '))) {
			fprintf(stderr, "Error in input data\n");
			exit(1);
		}
		for (count = match = 0;; count++) {
			if (!fgets(old_line, BUF_SIZE, oldfp)) {
				if (feof(oldfp)) {
					rewind(oldfp);
					break;
				}
				perror("fgets");
				exit (1);
			}
			if (!strcmp(old_line, new_line)) {
				match = 1;
				break;
			}
			if (!(old_ptr = strrchr(old_line, ' '))) {
				fprintf(stderr, "Error in input data\n");
				exit(1);
			}
			c = strcmp(new_ptr, old_ptr);
			if (sorted && c < 0) {
				printf("added      %s", new_line + (display_checksum ? 0 : 11));
				fflush(stdout);
				if (fp && check_critical_list(fp, new_line)) {
					fprintf(stderr, "WARN L2 >  %s", new_line + 11);
					crit_l2_count++;
				}
				rewind(oldfp);
				add_count++;
				break;
			}
			if (!c) {
				match = 1;
				break;
			} else
			if (!count)
				rewind(oldfp);
		} /*- end of for(match = 0;;) */
		if (!sorted && !match) {
			printf("added      %s", new_line + (display_checksum ? 0 : 11));
			fflush(stdout);
			if (fp && check_critical_list(fp, new_line)) {
				fprintf(stderr, "WARN L2 >  %s", new_line + 11);
				crit_l2_count++;
			}
			add_count++;
		}
	}
	if (!add_count && !del_count && !corrupt_count && !perm_count && !owner_count && !mod_count) {
		printf("0 changes detected\n");
		return (0);
	}
	count = add_count + del_count + corrupt_count + perm_count + owner_count + mod_count;
	printf("\n%lu Changes detected\n", count);
	printf("--------------------------\n");
	printf("1. added               : %lu\n", add_count);
	printf("2. deleted             : %lu\n", del_count);
	printf("3. replaced            : %lu\n", mod_count);
	printf("4. corrupted           : %lu\n", corrupt_count);
	printf("5. permission  changes : %lu\n", perm_count);
	printf("6. owner/group changes : %lu\n", owner_count);
	if (crit_l1_count || crit_l2_count) {
		printf("\n");
		printf("NOTE: Critical Changes\n");
		printf("1. critical L1 changes : %lu\n", crit_l1_count);
		printf("2. critical L2 changes : %lu\n", crit_l2_count);
	}
	printf("--------------------------\n");
	return (1);
}
