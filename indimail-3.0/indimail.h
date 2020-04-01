/*
 * $Log: indimail.h,v $
 * Revision 1.3  2019-06-03 06:50:32+05:30  Cprogrammer
 * replaced config.h with indimail_config.h
 *
 * Revision 1.2  2019-05-28 17:39:27+05:30  Cprogrammer
 * added load_mysql.h for mysql interceptor function prototypes
 *
 * Revision 1.1  2019-05-27 20:36:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef INDIMAILH_H
#define INDIMAILH_H

#ifndef	lint
static char     sccsidh[] = "$Id: indimail.h,v 1.3 2019-06-03 06:50:32+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_CONFIG_H
#include "indimail_config.h"
#endif
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
typedef int64_t mdir_t;
typedef uint64_t umdir_t;
#else
typedef long long mdir_t;
typedef unsigned long long umdir_t;
#define PRId64 "lld"
#define PRIu64 "llu"
#define SCNd64 "lld"
#define SCNu64 "llu"
#endif
#ifdef HAVE_MYSQL_H
#include <mysql.h>
#else
#error "mysql.h not found"
#endif
#include "load_mysql.h"
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif

/* Edit to match your set up */
#define MYSQL_HOST              "localhost"
#define MYSQL_USER              "indimail"
#define MYSQL_PASSWD            "ssh-1.5-" /*- control file overrides this */
#define MYSQL_DATABASE          "indimail"
#define MYSQL_DEFAULT_TABLE     "indimail"
#define MYSQL_INACTIVE_TABLE    "indibak"
#define RELAY_DEFAULT_TABLE     "relay"
#define PORT_SMTP                25
#define PORT_QMTP                209
#define PORT_QMQP                628

/* max field sizes */
#define MAX_PW_NAME             40
#define MAX_PW_DOMAIN           67
#define MAX_PW_HOST             32
#define MAX_PW_PASS             128
#define MAX_PW_GECOS            48
#define MAX_PW_DIR              156
#define MAX_PW_QUOTA            30
#define MAX_ALIAS_LINE          156
#define DBINFO_BUFF             128
#define INFIFO                  "infifo"
#define MCDFILE                 "mcdinfo"
#define FS_OFFLINE              0
#define FS_ONLINE               1
#define AVG_USER_QUOTA          "5000000"
#define ON_MASTER               0
#define ON_LOCAL                1

#define DBINFO_TABLE_LAYOUT "\
filename char(128) not null, \
domain   char(64) not null, \
distributed int not null, \
server   char(28) not null, \
mdahost  char(28) not null, \
port     int not null, \
use_ssl  int not null default 0, \
dbname   char(28) not null, \
user     char(28) not null, \
passwd   char(28) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
unique index (filename, domain, server, mdahost, port, dbname, user, passwd), \
index (domain)"

#define MAX_FAIL_ATTEMPTS       5
#define MYSQL_RETRY_INTERVAL    5
struct dbinfo
{
	char            domain[DBINFO_BUFF];   /*- domain name */
	int             distributed;           /*- 1 for distributed, 0 for non-distributed */
	char            mdahost[DBINFO_BUFF];  /*- server for imap, pop3, delivery */
	char            server[DBINFO_BUFF];   /*- mysql server */
	int             port;                  /*- mysql port */
	char           *socket;                /*- mysql_socket */
	char            use_ssl;               /*- set for ssl connection */
	char            database[DBINFO_BUFF]; /*- mysql database */
	char            user[DBINFO_BUFF];     /*- mysql user */
	char            password[DBINFO_BUFF]; /*- mysql passwd */
	int             fd;
	time_t          last_attempted;
	int             failed_attempts;
	char            isLocal;
	char           *last_error;
};
typedef struct dbinfo DBINFO;
extern MYSQL    mysql[2];
extern MYSQL  **MdaMysql;
extern DBINFO **RelayHosts;

#ifdef CLUSTERED_SITE
#define CNTRL_HOST              "localhost"
#define MASTER_HOST             "localhost"
#define CNTRL_USER              "indimail"
#define CNTRL_PASSWD            "ssh-1.5-" /*- control file overrides this */
#define CNTRL_DATABASE          "indimail"
#define CNTRL_DEFAULT_TABLE     "hostcntrl"
#endif /*- #ifdef CLUSTERED_SITE */

/* defaults - no need to change */
#define SMALL_SITE              0
#define LARGE_SITE              1
#define MYSQL_DOT_CHAR          '_'
#define MYSQL_LARGE_USERS_TABLE "users"

#define VLOG_ERROR_INTERNAL     0  /* logs an internal error these messages only go to syslog if option is on */
#define VLOG_ERROR_LOGON        1  /* bad logon, user does not exist */
#define VLOG_AUTH               2  /* logs a successful authentication */
#define VLOG_ERROR_PASSWD       3  /* password is incorrect or empty*/
#define VLOG_ERROR_ACCESS       4  /* access is denied by 2 in gid */

#ifdef IP_ALIAS_DOMAINS
#define IP_ALIAS_MAP_FILE       "etc/ip_alias_map"
#define IP_ALIAS_TOKENS         " \t\n"
#endif

#define TRUE                 1
#define FALSE                0
#define ABORT               -1

#ifdef VFILTER
extern char    *vfilter_comparision[];
extern char    *vfilter_header[];
extern char    *i_headers[];
extern char    *h_mailinglist[];

#define FILTER_EQUAL                  0
#define FILTER_CONTAIN                1
#define FILTER_DOES_NOT_CONTAIN       2
#define FILTER_STARTS_WITH            3
#define FILTER_ENDS_WITH              4
#define FILTER_NOT_IN_ADDRESS_BOOK    5
#define FILTER_NOT_IN_TO_CC_BCC       6

#endif /*- #ifdef VFILTER */

#ifdef CLUSTERED_SITE
#define CNTRL_TABLE_LAYOUT "\
pw_name char(40) not null, \
pw_domain char(67) not null, \
pw_passwd char(128) not null, \
host char(64) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, \
primary key (pw_name, pw_domain)"

#define HOST_TABLE_LAYOUT "\
host char(64) not null, \
ipaddr char(16) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (host), index ipaddr (ipaddr)"

#define SMTP_TABLE_LAYOUT "\
host char(64) not null, \
src_host char(64) not null, \
domain char(64) not null, port int, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (domain, host, src_host)"

#define ALIASDOMAIN_TABLE_LAYOUT "\
alias char(64) not null, \
domain char(67), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(alias)"

#define SPAM_TABLE_LAYOUT "\
email char(64) not null, \
spam_count int not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
index email (email), index spam_count (spam_count), index timestamp (timestamp)"

#define BADMAILFROM_TABLE_LAYOUT "\
email char(64) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (email)"
#endif /*- #ifdef CLUSTERED_SITE */

typedef struct 
{
	char           *name; /*- User printable name of the function.  */
	char           *doc;  /*- Documentation for this function.  */
} ADMINCOMMAND;
extern ADMINCOMMAND adminCommands[];

typedef struct
{
	int             which;
	char           *table_name;
	char           *template;
} IndiMAILTable;
extern IndiMAILTable IndiMailTable[];

/* small site table layout */
#define SMALL_TABLE_LAYOUT "\
pw_name char(40) not null, \
pw_domain char(67) not null, \
pw_passwd char(128) not null, \
pw_uid int, \
pw_gid int, \
pw_gecos char(48) not null, \
pw_dir char(156), \
pw_shell char(30), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key (pw_name, pw_domain), index pw_gecos (pw_gecos(25)), index pw_uid (pw_uid) "

/* large site table layout */
#define LARGE_TABLE_LAYOUT "\
pw_name char(40) not null, \
pw_passwd char(128) not null, \
pw_uid int, \
pw_gid int, \
pw_gecos char(48), \
pw_dir char(156), \
pw_shell char(30), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key(pw_name)"

#define RELAY_TABLE_LAYOUT "\
email char(96) not null, \
ipaddr char(18) not null, \
timestamp int, \
unique index (email, ipaddr), index(ipaddr), index(timestamp)"

#ifdef IP_ALIAS_DOMAINS
#define IP_ALIAS_TABLE_LAYOUT "\
ipaddr char(18) not null, \
domain char(67), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(ipaddr)"
#endif

#ifdef ENABLE_AUTH_LOGGING
#define FROM_ACTIVE_TO_INACTIVE 0
#define FROM_INACTIVE_TO_ACTIVE 1
/* last auth definitions */
#define AUTH_TIME  1
#define CREAT_TIME 2
#define PASS_TIME  3
#define ACTIV_TIME 4
#define INACT_TIME 5
#define POP3_TIME  6
#define IMAP_TIME  7
#define WEBM_TIME  8

#define LASTAUTH_TABLE_LAYOUT "\
user char(40) not null, \
domain char(67) not null,\
service char(10) not null, \
remote_ip char(16) not null,  \
quota int not null, \
gecos char(48) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, \
primary key (user, domain, service), index gecos (gecos), index quota (quota), \
index timestamp (timestamp)"

#define USERQUOTA_TABLE_LAYOUT "\
user char(40) not null, \
domain char(67) not null,\
quota bigint unsigned not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP NOT NULL, \
primary key(user, domain), index quota (quota)"
#endif

#define DIR_CONTROL_TABLE_LAYOUT "\
domain char(67) not null,\
cur_users int, \
level_cur int, level_max int, \
level_start0 int, level_start1 int, level_start2 int, \
level_end0 int, level_end1 int, level_end2 int, \
level_mod0 int, level_mod1 int, level_mod2 int, \
level_index0 int , level_index1 int, level_index2 int, the_dir char(156), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
unique index (domain)"

#define DIR_CONTROL_SELECT "\
cur_users, \
level_cur, level_max, \
level_start0, level_start1, level_start2, \
level_end0, level_end1, level_end2, \
level_mod0, level_mod1, level_mod2, \
level_index0, level_index1, level_index2, the_dir"

#ifdef VALIAS
#define VALIAS_TABLE_LAYOUT "\
alias  char(40) not null, \
domain char(67) not null, \
valias_line char(190) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
unique index(alias, domain, valias_line), index (alias, domain)"
#endif

#define MGMT_TABLE_LAYOUT "\
user  char(32) not null, \
pass char(128) not null, \
pw_uid int not null, \
pw_gid int not null, \
lastaccess int not null, \
lastupdate int not null, \
day char(2) not null, \
attempts int not null, \
status char(2) not null, \
zztimestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
unique index(user)"

#ifdef ENABLE_MYSQL_LOGGING
#define VLOG_TABLE_LAYOUT "\
id bigint primary key auto_increment, \
user char(40), \
passwd char(28), \
domain char(67), \
logon char(32), \
remoteip char(18), \
message varchar(254), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP, \
error int, \
index user_idx (user), \
index domain_idx (domain), \
index remoteip_idx (remoteip), \
index error_idx (error), \
index message_idx (message)"
#endif

#define BULKMAIL_TABLE_LAYOUT "\
emailid char(107) not null, \
filename char(64) not null, \
zztimestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key(emailid, filename)"

#define SMALL_INSERT "insert low_priority into  %s \
( pw_name, pw_domain, pw_passwd, pw_uid, pw_gid, pw_gecos, pw_dir, pw_shell ) \
values \
( \"%s\", \"%s\", \"%s\", %d, 0, \"%s\", \"%s\", \"%s\" )"

#define SMALL_GETALL "select high_priority pw_name, pw_passwd, pw_uid, pw_gid, \
pw_gecos, pw_dir, pw_shell from %s where pw_domain = \"%s\""

#define SMALL_SETPW "update low_priority %s set pw_passwd = \"%s\", \
pw_uid = %d, pw_gid = %d, pw_gecos = \"%s\", pw_dir = \"%s\", \
pw_shell = \"%s\" where pw_name = \"%s\" and pw_domain = \"%s\""

#define FSTAB_TABLE_LAYOUT "\
filesystem char(64) not null, \
host char(64) not null, \
status int not null, \
max_users bigint not null, \
cur_users bigint not null, \
max_size bigint not null, \
cur_size bigint not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key (filesystem, host), index status (status)"

#ifdef VFILTER
#define FILTER_TABLE_LAYOUT "\
emailid char(107) not null, \
filter_no smallint not null, \
filter_name char(32) not null, \
header_name smallint not null, \
comparision tinyint not null, \
keyword char(64) not null, \
destination char(156) not null, \
bounce_action char(64) not null, \
mailing_list tinyint not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(emailid, filter_no), unique index (emailid, header_name, comparision, keyword, destination)"
#endif

#define MAILING_LIST_TABLE_LAYOUT "\
emailid char(107) not null, \
filter_no smallint not null, \
mailing_list char(64) not null, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(emailid, mailing_list), index emailid (emailid, filter_no)"

#ifdef ENABLE_DOMAIN_LIMITS
#define LIMITS_TABLE_LAYOUT " \
domain                   CHAR(67) NOT NULL, \
domain_expiry            INT(10) NOT NULL DEFAULT -1, \
passwd_expiry            INT(10) NOT NULL DEFAULT -1, \
maxpopaccounts           INT(10) NOT NULL DEFAULT -1, \
maxaliases               INT(10) NOT NULL DEFAULT -1, \
maxforwards              INT(10) NOT NULL DEFAULT -1, \
maxautoresponders        INT(10) NOT NULL DEFAULT -1, \
maxmailinglists          INT(10) NOT NULL DEFAULT -1, \
diskquota                BIGINT UNSIGNED NOT NULL DEFAULT 0, \
maxmsgcount              BIGINT UNSIGNED NOT NULL DEFAULT 0, \
defaultquota             BIGINT NOT NULL DEFAULT 0, \
defaultmaxmsgcount       BIGINT UNSIGNED NOT NULL DEFAULT 0, \
disable_pop              TINYINT(1) NOT NULL DEFAULT 0, \
disable_imap             TINYINT(1) NOT NULL DEFAULT 0, \
disable_dialup           TINYINT(1) NOT NULL DEFAULT 0, \
disable_passwordchanging TINYINT(1) NOT NULL DEFAULT 0, \
disable_webmail          TINYINT(1) NOT NULL DEFAULT 0, \
disable_relay            TINYINT(1) NOT NULL DEFAULT 0, \
disable_smtp             TINYINT(1) NOT NULL DEFAULT 0, \
perm_account             TINYINT(2) NOT NULL DEFAULT 0, \
perm_alias               TINYINT(2) NOT NULL DEFAULT 0, \
perm_forward             TINYINT(2) NOT NULL DEFAULT 0, \
perm_autoresponder       TINYINT(2) NOT NULL DEFAULT 0, \
perm_maillist            TINYINT(4) NOT NULL DEFAULT 0, \
perm_quota               TINYINT(2) NOT NULL DEFAULT 0, \
perm_defaultquota        TINYINT(2) NOT NULL DEFAULT 0, \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP not null, \
primary key(domain)"
#endif

#define ATRN_MAP_LAYOUT "\
pw_name char(40) not null, \
pw_domain char(67) not null, \
domain_list char(67), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
unique index atrnmap (pw_name, pw_domain, domain_list)"

#define PRIV_CMD_LAYOUT "\
user        char(32) not null, \
program     char(64) not null, \
cmdswitches char(128), \
timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL, \
primary key(user, program)"

#define ATCHARS                 "@%:"
#define MSG_BUF_SIZE            8192
#define USE_POP                 0x00
#define USE_APOP                0x01
#define ADD_FLAG                2
#define PWD_FLAG                3
#define DEL_FLAG                4
#define MAX_DOMAINNAME          100

#define BOUNCE_ALL              "bounce-no-mailbox"
#define DELETE_ALL              "delete"

/* gid flags */
#define NO_PASSWD_CHNG          0x01
#define NO_POP                  0x02
#define NO_WEBMAIL              0x04
#define NO_IMAP                 0x08
#define BOUNCE_MAIL             0x10
#define NO_RELAY                0x20
#define NO_DIALUP               0x40
#define QA_ADMIN                0x80
#define V_OVERRIDE              0x100
#define NO_SMTP                 0x200
#define V_USER0                 0x400
#define V_USER1                 0x800
#define V_USER2                 0x1000
#define V_USER3                 0x2000

/* modes for indimail dirs, files and qmail files */
#define OVERQUOTA_MAILSIZE      1000
#define MAILCOUNT_LIMIT         1500 /*- Allow 1500 incoming mails */
#define MAILSIZE_LIMIT          10485760 /*- Allow 10MB incoming mails */
#define INDIMAIL_TCPRULES_UMASK 0022
#define INDIMAIL_DIR_MODE       0750
#define INDIMAIL_QMAIL_MODE     0644
#define BULK_MAILDIR            "bulk_mail"
#define WELCOMEMAIL             "1welcome.txt,all"
#define ACTIVATEMAIL            "activate.txt"
#define MIGRATEUSER             "/usr/local/bin/migrateuser"
#define MIGRATEFLAG             "Qmail.txt"
#define TOKENS                  " \t"

/* error return codes */
#define VA_SUCCESS              0
#define SOCKBUF                 32768 /*- Buffer size used in Monkey service -*/
#define MAXSLEEP                0
#define MAXNOBUFRETRY           60 /*- Defines maximum number of ENOBUF retries -*/
#define SELECTTIMEOUT           30 /*- secs after which select will timeout -*/
#if !defined(INADDR_NONE) && defined(sun)
#define INADDR_NONE             0xffffffff /*- should be in <netinet/in.h> -*/
#endif
#define NULL_REMOTE_IP          "0.0.0.0"

/*- Fifo Server Definitions */
#define USER_QUERY   1
#define RELAY_QUERY  2
#define PWD_QUERY    3
#ifdef CLUSTERED_SITE
#define HOST_QUERY   4
#endif
#define ALIAS_QUERY  5
#define LIMIT_QUERY  6
#define DOMAIN_QUERY 7

#define MAX_LINK_COUNT 32000 /*- max links to create for bulk emails */

#endif /*- #ifdef INDIMAILH_H */
