/*
 * $Id: $
 */
#ifndef OSH_H
#define OSH_H
/* Don't change this! */

#undef COMPILE_TABLE

/*-
 * Define this if you want to have the command table built into the shell.
 * If you undefine this, TABLE_NAME will be read for the available commands
 * whenever the shell is started, otherwise, the commands are hardcoded
 * into struct.h with the define 'NUMENTRY' in main.c giving the number
 * of entries in that table.
 */

#define TABLE_NAME "/root/osh/table"

/*-
 * This is the table from which we load the command listing for the user
 * executing osh. It should be owned by root, and be unreadable by all
 * but root.
 * If you append '{}' to the end of the filename, {} will be substituted by
 * the hostname of the machine on which osh is being run. In other words,
 * if you have "table.hosta" and "table.hostb", and your TABLE_NAME is
 * "table.{}", Osh will use "table.hosta" if it is being run from the machine
 * 'hosta'. *NOTE* if you decide to use this, and a machine does NOT have
 * an associated table, Osh will bomb.
 */

#define LOGGING

/* Define this if you want to have osh executions and commands logged */

#undef SYSLOG
/*-
 * If logging is defined, and this is defined, log to the syslog rather than
 * to a file.
 */

#define SYSLOG_PRIORITY LOG_INFO

/*-
 * If SYSLOG is defined, define this to be the priority of messages logged
 * by syslog calls (see the syslog man page for more inforamtion).
 *
 * If LOGGING is defined, and SYSLOG isn't defined, this is the file to
 * which log entries will be sent
 */

#define CHECK_ACCESS

/*-
 * This define forces the user who's files are being looked at to have
 * a file (name by ACCESS_FILE) in their home directory. A file or directory
 * can ONLY be read if that file exists if this is defineds. If OPER_OVERRIDE
 * is defined, group OPER_GROUP is allowed to read any file.
 */

#define ACCESS_FILE ".consult"

/*-
 * This is the file who's existance is checked for in the file owner's
 * home directory before that file can be read
 */

/* See description for CHECK_ACCESS */
#undef OPER_OVERRIDE

/* Name of group to allow OPER_OVERRIDE status to */
#define OPER_GROUP "operator"

/* Handler prototypes */
int i_alias(int, char **);
int i_more(int, char **);
int i_cd(int, char **); 
int i_pwd(int, char **);
void i_done(int, char **);
int i_cp(int, char **);
int i_rm(int, char **);
int i_vi(int, char **);
int i_help(int, char **);
int i_ldcache(int, char **);
int execute(int, char **);
int i_mount(int, char **);
int i_test(int, char **);
int i_exit(int, char **);
void fatal(char *);

struct entry {
   char *prog_name;	/* Name which parser will accept */
   int (*handler)(int, char **);    /* Routine to call if internal */
   char *path;		/* Path to program-we don't want to use execvp
			   because of search path insecurity */
};

struct hand {
   char *prog_name;
   int (*handler)(int, char **);
};

struct alias {
   char *cmd;
   char *alias;
};


#ifdef COMPILE_TABLE
static struct entry Table[]= {
  { "help"    , i_help  , NULL                    },
  { "cd"      , i_cd    , NULL                    },
  { "more"    , i_more  , NULL                    },
  { "alias"   , i_alias , NULL                    },
  { "test"    , i_test  , NULL                    },
  { "logout"  , i_exit  , NULL                    },
  { "exit"    , i_exit  , NULL                    },
  { "wc"      , execute , "/bin/wc"               },
  { "ls"      , execute , "/bin/ls"               },
  { "rm"      , i_rm    , "/bin/rm"               },
  { "cp"      , i_cp    , "/bin/cp"               },
  { "cat"     , execute , "/bin/cat"              },
  { "who"     , execute , "/bin/who"              },
  { "finger"  , execute , "/usr/ucb/finger"       },
  { "printenv", execute , "/usr/ucb/printenv"     },
  { "grep"    , execute , "/bin/grep"             },
  {  NULL, NULL, NULL }
};
static int NUMENTRY=16; /* Number of entries listed in the compiled table */
#endif

#define NUMINT  12

#define MAXTABLESIZE 256

#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif
#ifndef BADSIG
# define BADSIG	(void (*)())-1
#endif


#define MAXARG 1024  /* To account for wildcards (ie. scc accounts) */
#define MAXFNAME 32
#define MAXWORD 20
#define BADFD (-1)

#define lowbyte(w) ((w) & 0377)
#define highbyte(w) lowbyte((w)>>8)
#ifndef OSH_H
# define MIN(a,b)   ((a<b) ? a:b)
#endif

typedef enum {TWORD,TPIPE,TAMP,TDOLLAR,TSEMI,TGT,TGTGT,TLT,TNL,TEOF} TOKEN;

extern int writeable(char *);
extern int instring(char *, char *);
extern char *expand(char *argv[MAXARG + 1], int *, char *);
extern int check_access(int, char **);
extern int NUMENTRY;
extern struct entry Table[];
extern struct alias AliasList[];
extern int AliasCounter;
extern char *FileList[];
extern char **environ;
extern void logit(char);
void logout();
#endif
/*
 * $Log: osh.h,v $
 * Revision 1.3  2021-09-01 18:41:48+05:30  Cprogrammer
 * reverted return type of i_exit to int
 *
 * Revision 1.2  2021-09-01 18:29:59+05:30  Cprogrammer
 * updated function prototyps
 *
 * Revision 1.1  2019-04-13 23:39:27+05:30  Cprogrammer
 * osh.h
 *
 * Revision 2.1  2004-09-05 00:53:13+05:30  Cprogrammer
 * operator shell header file
 *
 */
