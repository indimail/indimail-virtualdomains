# HTML Templates & Macro substitution

Contributed 12/2003 by Rick Widmer.

When iwebadmin encounters ## while parsing a template file, it looks at the following character and replaces ## and the character with the information listed below.

The templates which use each value are listed below each item.

1. Tag values w is not defined in the source code.
2. Tag values b, f, h, l, m, r, u, w, N, O, T are not used in a current template. Except for w, they are defined in template.c. It may best to remove them.
3. Tag value H is not defined in template.c but is used in header.html.

To see which templates use a particular tag:

grep -c '##A' * | grep -v ':0' | less

Change '##A' to the tag of interest. To see which templates do NOT use a particular tag, leave '-v' out of the second grep. The script `find_tags` does this.

## Tags

\##~ Current selected language (e.g., "en"). Could be useful in selecting alternate graphics for alternate languages.  
\##A Send the user name currently being acted upon. (ActionUser) If postmaster was editing someone this would be someone. Used in add\_listdig, add\_listmod, add\_listuser, del\_autorespond\_confirm, del\_forward\_confirm, del\_listdig, del\_listmod, del\_listuser, del\_mailinglist\_confirm, del\_user\_confirm, mod\_autorespond, mod\_dotqmail, mod-mailinglist-idx, mod-user, show\_digest\_subscribers, show\_moderators, show\_subscribers  
\##a Send the Alias of a mailing list. (Alias). Used in mod\_mailinglist  
\##B Show number of pop accounts. If MaxPopAccounts is > -1 then it shows (MaxPopAccounts) else it shows string 229 [unlimited]. Used in show\_users  
\##b Show the lines inside a alias table. Calls function show\_dowqmail\_lines. Not used in any current template files.  
\##C Send the CGIPATH. This is frequently used as part of the Action= in forms, and in building URLs for links. (CGIPATH). Used in ALL templates. Not used in header, footer or colortable.  
\##c Show the lines inside a mailing list table. Calls the function show\_mailing\_list\_line2. Used in add\_user  
\##D Send the domain name. (Domain). Used in all templates except show\_login.  
\##d Show the lines inside a forward table. Calls the function show\_dotqmail\_lines. Used in show\_forwards  
\##E This will be used to parse mod\_mailinglist-idx.html. Calls the function show\_current\_list\_values. Used in add\_mailinglist-idx, mod\_mailinglist-idx  
\##e calls show\_mailing\_list\_line. Displays a block of mailing lists. Used in show\_mailinglist  
\##F Display the contents of the autoresponder's message file. This is a fairly long block of inline code, with a note it should be moved to another file. Used in mod\_autorespond  
\##f Show the forwards. Calls show\_forwards. Not used in any template files.  
\##G Show the mailing list Digest Subscribers. This calls function show\_list\_group\_now(2). Used in show\_digest\_subscribers  
\##g Show the lines inside a autorespond table. Calls the function show\_autorespond\_line. Used in show\_autorespond  
\##H not used header.html  
\##h Show the counts, not sure which ones... Calls function show\_counts. Not used in any current templates.  
\##I No comment in source code. Calls function show\_dotqmail\_file. Used in mod\_dotqmail  
\##i? Check for user forward and forward/store vacation. Calls function check\_user\_forward\_vacation. This accepts a number after the letter in place of the ?. Values are:   
  0 return 'checked ' if there is no .qmail file (Standard delivery)
  1 return 'checked ' if this is a forward
  2 return address forwarded to
  3 return 'checked ' if local copy is to be saved
  4 return 'checked ' if vacation message
  5 return subject of vacation message
  6 return body of vacation message
  7 return gecos (real name) field from sql\_getpw
  8 return 'checked ' if this is a blackhole
  9 return 'checked ' if spam checking is enabled mod\_user (10 times).

Note: check\_user\_forward\_vacation is terribly inefficient. This needs to check for the first use of any of its request values and do all its file
reading ONE TIME, storing values in static variables for the rest of the calls. (May need to make sure we are still looking at the same user as the one data is buffered for.)  
\##J Show mailbox flag status. Calls function check\_mailbox\_flags. Used in mod\_user (8 times)  
\##j Show number of mailing lists. If (MaxMailingLists) > -1 this returns (CurrMailingLists)/(MaxMailingLists) else it returns (CurrMailingLists) followed by text string 229 [unlimited]. Used in show\_mailinglist  
\##K Show number of autoresponders. If (MaxForwards) > -1 this returns (CurrAutoResponders)/(MaxAutoResponders) else it returns (CurrAutoResponders) followed by text string 229 [unlimited]. Used in show\_autorespond  
\##k Show number of forwards. If (MaxForwards) > -1 this returns (CurForwards)/(MaxForwards) else it returns (CurForwards) followed by text string 229 [unlimited]. Used in show\_forwards  
\##L Login username. If (Username) is set it is shown, elseif "user=" is set in the Get parms it is used, else the string 'postmaster' is sent. If postmaster was editing someone this would be postmaster. Used in show\_login  
\##l Show the aliases stuff. This calls function show\_aliases. Not used in any current templates.  
\##M Show the mailing list subscribers. The function show\_list\_group\_now is called. Used in show\_subscribers  
\##m Show the mailing lists. The function show\_mailing\_lists is called. Not used in any current templates.  
\##N Parse include files. This is a fairly long section of inline code that either displays a template file doing ## substitution or sometimes displays text string 144. [file permission error]. Not used in any current templates.  
\##n not used. Not used in any current templates.  
\##O Build a pulldown menu of all POP/IMAP users. This calls function sql\_getall then builds an option tag for each entry returned.  
 <option value=(pw\_name)>(pw\_name)</option>
 not used in any current templates.
\##o Show the mailing list moderators. This calls function show\_list\_groupnow. Used in show\_moderators  
\##P Display mailing list prefix. This calls function get\_mailinglist\_prefix.  
 mod\_mailinglist-idx
\##p Show POP/IMAP users. This calls function show\_user\_lines. Used in show\_users  
\##Q not used. not used in any current templates.  
\##q Display user's quota. This prints the quota in megabytes, or if the user is a DOMAIN\_ADMIN the string 'NOQUOTA', else text string 229 [unlimited]. Used in mod\_user  
\##R not used. Not used in any current templates.  
\##r Show the autoresponder stuff. This calls function show\_autoresponders. not used in any current templates.  
\##S Send the status message parameter. (StatusMessage) used in ALL templates  
\##s Show the catchall name. Calls the function get\_catchall. Used in show\_users  
\##T Send the time parameter. (Mytime) used in every template except show\_login. (2-3 times in most)  
\##t? Contitionally Transmit a block of text. This calls function transmit\_block. It requires one character parameter to replace the ?. This character defines a condition, and the text between the beginning block and the end marker ##tt will only be sent if the condition is met. The possible condition values are:  
  a Administrative commands. Unless the user is an administrator, the text within a ##ta block will not be sent.
  h Help. This text will only be sent if --enable-help was set in ./configure.
  m MySQL. This text will only be sent if --enable-exmlm-mysql was set in ./configure.
  q Modify Quota. This text will only be sent if --enable-modify-quota was set in ./configure.
  s Modify Spam. This text will only be sent if --enable-modify-spam was set in ./configure.
  t This is the end tag. All text after this will be sent normally.
  u User. Not administrator. This is the opposite of the 'a' tag, and can be used to provide an alternate value for non-administrators.
 add\_user (4 times), mod\_user (18 times), show\_login (2 times)

Rather than using a/u it might be a good idea to use a/A. A lower case value is displayed when the thing is set, and the upper case is displayed when the thing is not set. That way if someone needs to do something on not modify-spam they can use S. Maybe it will never happen, but it happened with a/u so why not the others...  
\##U Send the username parameter. (Username) used in every template except show\_login. (2-3 times in most)  
\##u Show the users. This calls function show\_users not used in any current template.  
\##V Show version number. This is a link to the github link for iwebadmin.  QA\_PACKAGE is appended to the URL, and QA\_VERSION is the string that is displayed as the link. Used in show\_login  
\##v Display the main menu. This is a fairly long section of inline code, with a note that it should be moved to a function. It checks various things to decide which items should appear in the main menu. Quite a few text strings are conditionally displayed.  
  001 [iso-8859-1], 061 [Email Accounts], 077 [Mail Robots], 080 [List], 111 [Modify User], 122 [Forwards], 124 [Quick Links], 125 [New Email Account], 127 [New Forward], 128 [New Mail Robot], 129 [New Mailing List], 229 [unlimited], 249 [Quota], 253 [Limit:], 254 [Used:]. Used in main\_menu
\##W not used. Not used in any current templates.  
\##w not used. Not used in any current templates.  
\##X??? This tag gets a block of text from the dictionary and displays it in the template. This tag is always followed by three digits, which are used as an index into the translation dictionary. Used in every template, plus header.  
\##x Exit / logout link/text. This builds a link with the url returned by get\_session\_val("returnhttp and the link is get\_session\_val("returntext"). Used in every template except show\_login.  
\##Y Return text. This calls get\_session\_val("returntext="). Used in show\_login  
\##y Return http. This calls get\_session\_val("returnhttp="). Used in  show\_login  
\##Z Send the image URL directory. Used to create the path for <IMG> tags. (IMAGEURL). Used in mail\_menu, show\_login  
\##z Send the domain name for the show\_login page. If Domain is set it is used, else if "dom=" is set in the incoming URL its value is used, else if DOMAIN\_AUTOFILL is set the domain name being viewed, else nothing is sent. Used in show\_login  

# TESTING
iwebadmin can be tested by running at shell prompt. You just need to pass PATH\_INFO, REQUEST\_METHOD, QUERY\_STRING environment variables

## Login

Ensure that the parameters in QUERY\_STRING are uriencode
```
env PATH_INFO="" REQUEST_METHOD=POST \
	QUERY_STRING="username=postmaster&domain=example.com&password=xxxx&time=$(date +%s)" \
	/var/www/cgi-bin/iwebadmin
```

# Logout
```
sudo env PATH_INFO="/com/logout" REQUEST_METHOD=POST \
	QUERY_STRING="user=postmaster&dom=example.com&time=1663337051" \
	/var/www/cgi-bin/iwebadmin
```

## Other Items

```
LIST="showmenu quick showusers showaliases showforwards showmailinglists showautoresponders \
	adduser addusernow setdefault bounceall deleteall setremotecatchall setremotecatchallnow \
	addlistmodnow dellistmod dellistmodnow addlistmod showlistmod addlistdig addlistdignow \
	dellistdig dellistdignow showlistdig moduser modusernow deluser delusernow moddotqmail \
	moddotqmailnow deldotqmail deldotqmailnow adddotqmail adddotqmailnow addmailinglist \
	delmailinglist delmailinglistnow addlistusernow dellistuser dellistusernow addlistuser \
	addmailinglistnow modmailinglist modmailinglistnow modautorespond addautorespond \
	addautorespondnow modautorespondnow showlistusers delautorespond delautorespondnow \
	logout showcounts"

for i in $LIST
do
	sudo env PATH_INFO=/com/$i REQUEST_METHOD=GET \
		QUERY_STRING="user=postmaster&dom=example.com&time=1663330058" \
		/var/www/cgi-bin/iwebadmin >/tmp/a.html
done
```
