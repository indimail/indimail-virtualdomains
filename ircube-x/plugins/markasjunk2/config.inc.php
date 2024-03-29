<?php

/**
 * MarkAsJunk2 configuration file
 */

// Learning driver
// Use an external process such as sa_learn, sa_blacklist to learn from spam/ham messages. Default: null.
// sa_blacklist also runs the cmd_learn driver
// Please see the README for more information
$config['markasjunk2_learning_driver'] = 'sa_blacklist';

// Ham mailbox
// Mailbox messages should be moved to when they are marked as ham. null = INBOX
// set to FALSE to disable message moving
$config['markasjunk2_ham_mbox'] = null;

// Spam mailbox
// Mailbox messages should be moved to when they are marked as spam.
// null = the mailbox assigned as the spam folder in Roundcube settings
// set to FALSE to disable message moving
$config['markasjunk2_spam_mbox'] = null;

// Mark messages as read when reporting them as spam
$config['markasjunk2_read_spam'] = true;

// Mark messages as unread when reporting them as ham
$config['markasjunk2_unread_ham'] = false;

// Add flag to messages marked as spam (flag will be removed when marking as ham)
// If you do not want to use message flags set this to false
$config['markasjunk2_spam_flag'] = 'Junk';

// Add flag to messages marked as ham (flag will be removed when marking as spam)
// If you do not want to use message flags set this to false
// $config['markasjunk2_ham_flag'] = 'NonJunk';
$config['markasjunk2_ham_flag'] = false;

// Write output from spam/ham commands to the log for debug
$config['markasjunk2_debug'] = false;

// Show icon on toolbar
// The mark as spam/ham icon can either be displayed on the toolbar or as part of the mark messages menu
// 0 - always show in mark message menu
// 1 - always show on toolbar
// 2 - show in toolbar on mailbox screen, show in mark message menu message on screen
// 3 - show in mark message menu on mailbox screen, show in toolbar message on screen
$config['markasjunk2_toolbar'] = 1;

// Learn any message moved to the spam mailbox as spam (not just when the button is pressed)
$config['markasjunk2_move_spam'] = false;

// Learn any message moved from the spam mailbox to the ham mailbox as ham (not just when the button is pressed)
$config['markasjunk2_move_ham'] = false;

// Some drivers create new copies of the target message(s), in this case the original message(s) will be deleted
// Rather than deleting the message(s) (moving to Trash) setting this option true will cause the original message(s)
// to be permanently removed
$config['markasjunk2_permanently_remove'] = false;

// Display only a mark as spam button
$config['markasjunk2_spam_only'] = false;

// Activate MarkAsJunk2 for selected mail hosts only. If this is not set all mail hosts are allowed.
// Example: $config['markasjunk2_allowed_hosts'] = array('mail1.domain.tld', 'mail2.domain.tld');
$config['markasjunk2_allowed_hosts'] = null;

// Load specific config for different mail hosts
// Example: $config['markasjunk2_host_config'] = array(
//    'mail1.domain.tld' => 'mail1_config.inc.php',
//    'mail2.domain.tld' => 'mail2_config.inc.php',
// );
$config['markasjunk2_host_config'] = null;

// cmd_learn Driver options
// ------------------------
// The command used to learn that a message is spam
// The command can contain the following macros that will be expanded as follows:
//      %u is replaced with the username (from the session info)
//      %l is replaced with the local part of the username (if the username is an email address)
//      %d is replaced with the domain part of the username (if the username is an email address or default mail domain if not)
//      %i is replaced with the email address from the user's default identity
//      %s is replaced with the email address the message is from
//      %f is replaced with the path to the message file
//      %h:<header name> is replaced with the content of that header from the message (lower case) eg: %h:x-dspam-signature
//      %m is replaced with the mysql connect string
// If you do not want to run the command set this to null
$config['markasjunk2_spam_cmd'] = '/usr/libexec/indimail/bogo-learn %m %u dummy spam %f';

// The command used to learn that a message is ham
// The command can contain the following macros that will be expanded as follows:
//      %u is replaced with the username (from the session info)
//      %l is replaced with the local part of the username (if the username is an email address)
//      %d is replaced with the domain part of the username (if the username is an email address or default mail domain if not)
//      %i is replaced with the email address from the user's default identity
//      %s is replaced with the email address the message is from
//      %f is replaced with the path to the message file
//      %h:<header name> is replaced with the content of that header from the message (lower case) eg: %h:x-dspam-signature
//      %m is replaced with the mysql connect string
// If you do not want to run the command set this to null
$config['markasjunk2_ham_cmd'] = '/usr/libexec/indimail/bogo-learn %m %u dummy ham %f';

// dir_learn Driver options
// ------------------------
// The full path of the directory used to store spam (must be writable by webserver)
$config['markasjunk2_spam_dir'] = null;

// The full path of the directory used to store ham (must be writable by webserver)
$config['markasjunk2_ham_dir'] = null;

// The filename prefix
// The filename can contain the following macros that will be expanded as follows:
//      %u is replaced with the username (from the session info)
//      %l is replaced with the local part of the username (if the username is an email address)
//      %d is replaced with the domain part of the username (if the username is an email address or default mail domain if not)
//      %t is replaced with the type of message (spam/ham)
$config['markasjunk2_filename'] = null;

// email_learn Driver options
// --------------------------
// The email address that spam messages will be sent to
// The address can contain the following macros that will be expanded as follows:
//      %u is replaced with the username (from the session info)
//      %l is replaced with the local part of the username (if the username is an email address)
//      %d is replaced with the domain part of the username (if the username is an email address or default mail domain if not)
//      %i is replaced with the email address from the user's default identity
// If you do not want to send an email set this to null
$config['markasjunk2_email_spam'] = null;

// The email address that ham messages will be sent to
// The address can contain the following macros that will be expanded as follows:
//      %u is replaced with the username (from the session info)
//      %l is replaced with the local part of the username (if the username is an email address)
//      %d is replaced with the domain part of the username (if the username is an email address or default mail domain if not)
//      %i is replaced with the email address from the user's default identity
// If you do not want to send an email set this to null
$config['markasjunk2_email_ham'] = null;

// Should the spam/ham message be sent as an attachment
$config['markasjunk2_email_attach'] = true;

// The email subject (when sending as attachment)
// The subject can contain the following macros that will be expanded as follows:
//      %u is replaced with the username (from the session info)
//      %l is replaced with the local part of the username (if the username is an email address)
//      %d is replaced with the domain part of the username (if the username is an email address or default mail domain if not)
//      %t is replaced with the type of message (spam/ham)
$config['markasjunk2_email_subject'] = 'learn this message as %t';

// sa_blacklist Driver options
// ---------------------------
// Path to SAUserPrefs config file
$config['markasjunk2_sauserprefs_config'] = '../sauserprefs/config.inc.php';

// amavis_blacklist Driver options
// ---------------------------
// Path to amacube config file
$config['markasjunk2_amacube_config'] = '../amacube/config.inc.php';

// edit_headers Driver options
// ---------------------------
// Patterns to match and replace headers for spam messages
// Replacement method uses preg_replace - http://www.php.net/manual/function.preg-replace.php
// WARNING: Be sure to match the entire header line, including the name of the header, also use ^ and $ and the 'm' flag
// see the README for an example
// TEST CAREFULLY BEFORE USE ON REAL MESSAGES
$config['markasjunk2_spam_patterns'] = array(
                                                'patterns' => array(),
                                                'replacements' => array()
                                                );

// Patterns to match and replace headers for spam messages
// Replacement method uses preg_replace - http://www.php.net/manual/function.preg-replace.php
// WARNING: Be sure to match the entire header line, including the name of the header, also use ^ and $ and the 'm' flag
// see the README for an example
// TEST CAREFULLY BEFORE USE ON REAL MESSAGES
$config['markasjunk2_ham_patterns'] = array(
                                                'patterns' => array(),
                                                'replacements' => array()
                                                );
