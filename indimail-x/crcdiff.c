
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
static char     sccsid[] = "$Id: crcdiff.c,v 1.7 2025-06-07 18:18:22+05:30 Cprogrammer Exp mbhangui $";
#endif

static void
usage(int exitval)
{
	fprintf(stderr, "crcdiff [-Cis] [-c critical_list] file1 file2)\n");
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

static char    *fn1, *fn2;

int
main(int argc, char **argv)
{

	char           *new_ptr, *old_ptr, *critical_file_list = NULL;
	FILE           *newfp, *oldfp, *fp = NULL;
	int             match, c, sorted = 0, crit_flag, display_checksum = 0, ignore_corrupted = 0;
	unsigned long   add_count, del_count, corrupt_count, perm_count,
					owner_count, mod_count, crit_l1_count, crit_l2_count, count;
	char            new_line[BUF_SIZE], old_line[BUF_SIZE];

	while ((c = getopt(argc, argv, "c:Cis")) != -1) {
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
		case 'i':
			ignore_corrupted = 1;
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
	fn1 = argv[optind];
	fn2 = argv[optind + 1];
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
			if ((old_ptr = strrchr(old_line, '\n')))
				*old_ptr = '\0';
			fprintf(stderr, "%s: Error in input data1 [%s]\n", fn1, old_line);
			continue;
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
				if ((new_ptr = strrchr(new_line, '\n')))
					*new_ptr = '\0';
				fprintf(stderr, "%s: Error in input data2 [%s]\n", fn2, new_line);
				continue;
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
				if (!old_ptr) {
					fprintf(stderr, "%s: Error in input data1 [%s]\n", fn1, old_line);
					return (1);
				}
				if (!new_ptr) {
					fprintf(stderr, "%s: Error in input data2 [%s]\n", fn2, new_line);
					return (1);
				}
				/*- check crc change */
				if (strncmp(new_line, old_line, 10)) { /* corrupt  - crc changed w/o date change */
					if (!strcmp(new_ptr, old_ptr)) {
						if (!ignore_corrupted) {
							printf("corrupt    %s", new_line + (display_checksum ? 0 : 11));
							if (fp && check_critical_list(fp, new_line)) {
								fprintf(stderr, "WARN L2 >  %s", new_line + 11);
								crit_l2_count++;
							}
							corrupt_count++;
						}
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
					printf("permiss<   %s", old_line + (display_checksum ? 0 : 11));
					printf("permiss>   %s", new_line + (display_checksum ? 0 : 11));
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
					printf("own/grp<   %s", old_line + (display_checksum ? 0 : 11));
					printf("own/grp>   %s", new_line + (display_checksum ? 0 : 11));
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
			return (1);
		}
		if (!(new_ptr = strrchr(new_line, ' '))) {
			if ((new_ptr = strrchr(new_line, '\n')))
				*new_ptr = '\0';
			fprintf(stderr, "%s: Error in input data2 [%s]\n", fn2, new_line);
			continue;
		}
		for (count = match = 0;; count++) {
			if (!fgets(old_line, BUF_SIZE, oldfp)) {
				if (feof(oldfp)) {
					rewind(oldfp);
					break;
				}
				perror("fgets");
				return (1);
			}
			if (!strcmp(old_line, new_line)) {
				match = 1;
				break;
			}
			if (!(old_ptr = strrchr(old_line, ' '))) {
				if ((old_ptr = strrchr(old_line, '\n')))
					*old_ptr = '\0';
				fprintf(stderr, "%s: Error in input data1 [%s]\n", fn1, old_line);
				continue;
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
	if (ignore_corrupted) {
		printf("4. permission  changes : %lu\n", perm_count);
		printf("5. owner/group changes : %lu\n", owner_count);
	} else {
		printf("4. corrupted           : %lu\n", corrupt_count);
		printf("5. permission  changes : %lu\n", perm_count);
		printf("6. owner/group changes : %lu\n", owner_count);
	}
	if (crit_l1_count || crit_l2_count) {
		printf("\n");
		printf("NOTE: Critical Changes\n");
		printf("1. critical L1 changes : %lu\n", crit_l1_count);
		printf("2. critical L2 changes : %lu\n", crit_l2_count);
	}
	printf("--------------------------\n");
	return (1);
}
