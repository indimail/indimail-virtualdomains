/*
 * $Id: $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef	lint
static char     rcsid[] = "$Id: $";
#endif

#ifdef HAVE_QMAIL
#include <substdio.h>
#include <subfd.h>
#endif
#include <vlimits.h>
#include "common.h"
#include "print_limits.h"

void
print_limits(struct vlimits *lmt)
{
	if (lmt->limit_type == 1) {
		subprintfe(subfdout, "vlimits", "Domain Expiry   Date : %s",
			lmt->domain_expiry == -1 ? "Never Expires\n" : ctime(&lmt->domain_expiry));
		subprintfe(subfdout, "vlimits", "Password Expiry Date : %s",
			lmt->passwd_expiry == -1 ? "Never Expires\n" : ctime(&lmt->passwd_expiry));
		if (!lmt->diskquota)
			subprintfe(subfdout, "vlimits", "Max Domain Quota     :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Max Domain Quota     : %13lu\n", lmt->diskquota);
		if (!lmt->maxmsgcount)
			subprintfe(subfdout, "vlimits", "Max Domain Messages  :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Max Domain Messages  : %13lu\n", lmt->maxmsgcount);
		if (!lmt->defaultquota)
			subprintfe(subfdout, "vlimits", "Default User Quota   :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Default User Quota   : %13lu\n", lmt->defaultquota);
		if (!lmt->defaultmaxmsgcount)
			subprintfe(subfdout, "vlimits", "Default User Messages:     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Default User Messages: %13lu\n", lmt->defaultmaxmsgcount);
		if (lmt->maxpopaccounts == -1)
			subprintfe(subfdout, "vlimits", "Max Pop Accounts     :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Max Pop Accounts     : %13ld\n", lmt->maxpopaccounts);
		if (lmt->maxaliases == -1)
			subprintfe(subfdout, "vlimits", "Max Aliases          :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Max Aliases          : %13ld\n", lmt->maxaliases);
		if (lmt->maxforwards == -1)
			subprintfe(subfdout, "vlimits", "Max Forwards         :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Max Forwards         : %13ld\n", lmt->maxforwards);
		if (lmt->maxautoresponders == -1)
			subprintfe(subfdout, "vlimits", "Max Autoresponders   :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Max Autoresponders   : %13ld\n", lmt->maxautoresponders);
		if (lmt->maxmailinglists == -1)
			subprintfe(subfdout, "vlimits", "Max Mailinglists     :     unlimited\n");
		else
			subprintfe(subfdout, "vlimits", "Max Mailinglists     : %13ld\n", lmt->maxmailinglists);
		out("vlimits", "\n");
		subprintfe(subfdout, "vlimits", "GID Flags:\n");
		subprintfe(subfdout, "vlimits", "  %s\n", lmt->disable_dialup ? "NO_DIALUP" : "DIALUP");
		subprintfe(subfdout, "vlimits", "  %s\n", lmt->disable_passwordchanging ? "NO_PASSWD_CHNG" : "PASSWD CHNG");
		subprintfe(subfdout, "vlimits", "  %s\n", lmt->disable_smtp ? "NO_SMTP" : "SMTP");
		subprintfe(subfdout, "vlimits", "  %s\n", lmt->disable_imap ? "NO_IMAP" : "IMAP");
		subprintfe(subfdout, "vlimits", "  %s\n", lmt->disable_pop ? "NO_POP" : "POP3");
		subprintfe(subfdout, "vlimits", "  %s\n", lmt->disable_webmail ? "NO_WEBMAIL" : "WEBMAIL");
		subprintfe(subfdout, "vlimits", "  %s\n", lmt->disable_relay ? "NO_RELAY" : "RELAY");
	} else {
		subprintfe(subfdout, "vlimits", "Flags for non postmaster accounts:\n");
		subprintfe(subfdout, "vlimits", "  pop account           : %12s %12s %12s\n",
			lmt->perm_account & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_account & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_account & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  alias                 : %12s %12s %12s\n",
			lmt->perm_alias & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_alias & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_alias & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  forward               : %12s %12s %12s\n",
			lmt->perm_forward & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_forward & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_forward & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  autoresponder         : %12s %12s %12s\n",
			lmt->perm_autoresponder & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_autoresponder & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_autoresponder & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  mailinglist           : %12s %12s %12s\n",
			lmt->perm_maillist & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_maillist & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_maillist & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  mailinglist users     : %12s %12s %12s\n",
			lmt->perm_maillist_users & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_maillist_users & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_maillist_users & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  mailinglist moderators: %12s %12s %12s\n",
			lmt->perm_maillist_moderators & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_maillist_moderators & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_maillist_moderators & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  domain quota          : %12s %12s %12s\n",
			lmt->perm_quota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_quota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY",
			lmt->perm_quota & VLIMIT_DISABLE_DELETE ? "DENY_DELETE" : "ALLOW_DELETE");
		subprintfe(subfdout, "vlimits", "  default quota         : %12s %12s\n",
			lmt->perm_defaultquota & VLIMIT_DISABLE_CREATE ? "DENY_CREATE" : "ALLOW_CREATE",
			lmt->perm_defaultquota & VLIMIT_DISABLE_MODIFY ? "DENY_MODIFY" : "ALLOW_MODIFY");
	}
	flush("vlimits");
	return;
}

/*
 * $Log: $
 */
