/*
 * $Log: variables.c,v $
 * Revision 1.3  2022-08-05 21:19:04+05:30  Cprogrammer
 * added ischema table
 * removed encrypt_flag
 *
 * Revision 1.2  2019-04-15 21:58:18+05:30  Cprogrammer
 * added dir_control.h for vdir struct definition
 *
 * Revision 1.1  2019-04-15 12:04:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_QMAIL
#include <stralloc.h>
#endif
#include "indimail.h"
#include "dir_control.h"

#ifndef	lint
static char     sccsid[] = "$Id: variables.c,v 1.3 2022-08-05 21:19:04+05:30 Cprogrammer Exp mbhangui $";
#endif

vdir_type       vdir;
char            dirlist[MAX_DIR_LIST] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int             create_flag;
int             site_size = SITE_SIZE;
int             userNotFound = 0;
int             is_open = 0;
stralloc        mysql_host = {0};
char           *indi_port, *default_table, *inactive_table, *cntrl_table;
mdir_t          CurBytes, CurCount;
uid_t           indimailuid = -1;
gid_t           indimailgid = -1;
int             OptimizeAddDomain;
int             is_inactive;
int             is_overquota;
int             verbose = 0;
int             use_etrn;
#ifdef VFILTER
int             use_vfilter;
#endif
char           *rfc_ids[] = {
	"postmaster",
	"abuse",
	"mailer-daemon",
	"prefilt",
	"postfilt",
	"spam",
	"nonspam",
	"register-spam",
	"register-nonspam",
	0
};

/*
 * 0 Hostcntrl Slave/Master
 * 1 Vpopmail
 */
MYSQL           mysql[2];
MYSQL         **MdaMysql;
DBINFO        **RelayHosts;
#ifdef VFILTER
char           *vfilter_comparision[] = {
	"Equals",
	"Contains",
	"Does not contain",
	"Starts with",
	"Ends with",
	"Sender Not in Address Book",
	"My id not in To, CC, Bcc",
	"Numerical Logical Expression",
#ifdef HAVE_FNMATCH
	"RegExp",
#endif
	0
};

char           *vfilter_header[] = {
	"Return-Path",
	"From",
	"Subject",
	"To",
	"Cc",
	"Bcc",
	"Reply-To",
	"Date",
	"Sender",
	"User-Agent",
	"Message-Id",
	"MIME-Version",
	"Content-Type",
	"Content-Transfer-Encoding",
	"Precedence",
	"Organization",
	"Errors-To",
	"List-Id",
	"Mailing-List",
	"X-Sender",
	"X-Mailing-List",
	"X-ML-Name",
	"X-List",
	"X-Loop",
	"X-BeenThere",
	"X-Sequence",
	"X-Mailer",
	"Importance",
	"X-Priority",
	"X-Spam-Status",
	"X-Spam-Rating",
	"Received",
	0
};

char           *i_headers[] = {
	"To",
	"Return-Path",
	"Delivered-To",
	"Cc",
	"Bcc",
	"Reply-To",
	"From",
	"Sender",
	"Reply-To",
	"Errors-To",
	"Disposition-Notification-To",
	0
};

char           *h_mailinglist[] = {
	"Precedence"
	"List-Id"
	"List-Post"
	"List-Help"
	"List-Unsubscribe"
	"List-Subscribe",
	"Mailing-List",
	"X-Mailing-List",
	"X-ML-Name",
	"X-List",
	"X-Loop",
	"X-BeenThere",
	"X-Sequence",
	0
};
#endif
#ifdef CLUSTERED_SITE
int             isopen_cntrl = 0;
int             delayed_insert = 0;
int             isopen_vauthinit[2] = {0, 0};
stralloc        cntrl_host = {0};
char           *cntrl_port;
#endif /*- #ifdef CLUSTERED_SITE */
ADMINCOMMAND adminCommands[] = {
	{PREFIX"/bin/vadduser", "Add a user to a Virtual Domain"},
	{PREFIX"/bin/vpasswd", "Change Password for a Mail User"},
	{PREFIX"/bin/vdeluser", "Delete Mail User from a Virtual Domain"},
	{PREFIX"/bin/vsetuserquota", "Set Filesystem Quota for a Mail user"},
	{PREFIX"/bin/vbulletin", "Send out Bulletins"},
	{PREFIX"/bin/vmoduser", "Modify Mail User Characteristics"},
	{PREFIX"/bin/valias", "Add an Alias"},
	{PREFIX"/bin/vuserinfo", "Mail User Information"},
	{PREFIX"/bin/vipmap", "Add/Modify/Delete IP Maps"},
	{PREFIX"/bin/vacation", "Add Mail Vacation Autoresponder"},
	{PREFIX"/bin/vmoveuser", "Move a user between different file systems"},
	{PREFIX"/bin/vrenameuser", "Rename a user"},
	{PREFIX"/bin/crc", "Calculate Checksums of files/directories"},
	{PREFIX"/bin/vcfilter", "Create Filters"},
	{PREFIX"/bin/vsmtp", "Add/Modify/Delete SMTP Routes"},
	{PREFIX"/bin/dbinfo", "Add/Modify/Delete Mail Control Defination"},
	{PREFIX"/bin/vhostid", "Add/Modify/Delete Host IDs"},
	{PREFIX"/bin/printdir", "Print Mail Hash Directory Info"},
	{PREFIX"/bin/svstat", "Get Service Status for IndiMail Services"},
	{PREFIX"/bin/vaddaliasdomain", "Add an Alias Domain"},
	{PREFIX"/bin/vadddomain", "Add a Virtual Domain"},
	{PREFIX"/bin/vcalias", "Convert .qmail files to valias format"},
	{PREFIX"/bin/vcaliasrev", "Convert Alias to .qmail format"},
	{PREFIX"/bin/vdeldomain", "Delete a Virtual Domain"},
	{PREFIX"/bin/vrenamedomain", "Rename a Virtual Domain"},
	{PREFIX"/bin/vdominfo", "Domain Information"},
	{PREFIX"/bin/vgroup", "Add/Modify/Delete Groups "},
	{PREFIX"/bin/vatrn", "Add ATRN Maps for ODMR"},
	{PREFIX"/bin/vpriv", "Add Privileges to Program for IndiSrvr"},
	{PREFIX"/bin/vlimit", "Administer Domain Wide Limits"},
	{PREFIX"/bin/hostcntrl", "Administer Hostcntrl Entries"},
	{PREFIX"/sbin/resetquota", "Reset/Correct quota for a Maildir"},
	{PREFIX"/sbin/vreorg", "Reorganize Mail Database"},
	{PREFIX"/sbin/vdeloldusers", "Delete Old Mail Users"},
	{PREFIX"/sbin/ipchange", "Change/Update IP Address changes in IndiMail"},
	{PREFIX"/sbin/svctool", "Service Configuration tool"},
	{PREFIX"/sbin/clearopensmtp", "Clear Open SMTP session"},
	{PREFIX"/sbin/hostsync", "Sync Hostcntrl Information"},
	{PREFIX"/sbin/inquerytest", "Test inlookup queries"},
	{PREFIX"/sbin/vmoddomain", "Modify Domain Information"},
	{PREFIX"/sbin/vserverinfo", "Mail Server Information"},
	{PREFIX"/sbin/mgmtpass", "Manage Admin Client Passwords"},
	{PREFIX"/sbin/vfstab", "Add/Modify/Delete Filesystem Balancing for Mail filesystems"},
	{LIBEXECDIR"/updatefile", "Update Control Files"},
	{(char *) NULL, (char *) NULL}
};

IndiMAILTable IndiMailTable[] = {
	{ON_LOCAL, "ischema",      SCHEMA_TABLE_LAYOUT},
	{ON_LOCAL, "atrn_map",     ATRN_MAP_LAYOUT},
	{ON_LOCAL, "bulkmail",     BULKMAIL_TABLE_LAYOUT},
	{ON_LOCAL, "fstab",        FSTAB_TABLE_LAYOUT},
#ifdef IP_ALIAS_DOMAINS
	{ON_LOCAL, "ip_alias_map", IP_ALIAS_TABLE_LAYOUT},
#endif
#ifdef ENABLE_AUTH_LOGGING
	{ON_LOCAL, "lastauth",     LASTAUTH_TABLE_LAYOUT},
	{ON_LOCAL, "userquota",    USERQUOTA_TABLE_LAYOUT},
#endif
#ifdef VALIAS
	{ON_LOCAL, "valias",       VALIAS_TABLE_LAYOUT},
#endif
#ifdef VFILTER
	{ON_LOCAL, "vfilter",      FILTER_TABLE_LAYOUT},
#endif
#ifdef ENABLE_DOMAIN_LIMITS
	{ON_LOCAL, "vlimits",      LIMITS_TABLE_LAYOUT},
#endif
#ifdef ENABLE_MYSQL_LOGGING
	{ON_LOCAL, "vlog",         VLOG_TABLE_LAYOUT},
#endif
#ifdef CLUSTERED_SITE
#ifdef VALIAS
	{ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT},
#endif
	{ON_MASTER, "dbinfo",      DBINFO_TABLE_LAYOUT},
	{ON_MASTER, "fstab",       FSTAB_TABLE_LAYOUT},
	{ON_MASTER, "host_table",  HOST_TABLE_LAYOUT},
	{ON_MASTER, "mgmtaccess",  MGMT_TABLE_LAYOUT},
	{ON_MASTER, "smtp_port",   SMTP_TABLE_LAYOUT},
	{ON_MASTER, "vpriv",       PRIV_CMD_LAYOUT},
	{ON_MASTER, "spam",        SPAM_TABLE_LAYOUT},
	{ON_MASTER, "badmailfrom", BADMAILFROM_TABLE_LAYOUT},
	{ON_MASTER, "badrcptto",   BADMAILFROM_TABLE_LAYOUT},
	{ON_MASTER, "spamdb",      BADMAILFROM_TABLE_LAYOUT},
#endif
	{0,         (char *) NULL, (char *) NULL}
};
