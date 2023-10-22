SUBDIRS=eps-1.2
COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4

DEFS=@DEFS@ -Dlint -I. -I@prefix@/include/qmail -Ieps-1.2 -I@mysql_incdir@

iauth_LTLIBRARIES = iauth.la
iauth_la_SOURCES = iauth.c
iauth_la_LIBADD = libindimail.la
iauth_la_DEPENDENCIES = libindimail.la
iauth_la_LDFLAGS = -L.libs -module -avoid-version -shared -rdynamic -no-undefined -nostartfiles $(LDFLAGS)
iauthdir=@libdir@

tcplookup_LTLIBRARIES = tcplookup.la
tcplookup_la_SOURCES = mytcplookup.c
tcplookup_la_LIBADD = libindimail.la
tcplookup_la_DEPENDENCIES = libindimail.la mytcplookup.lo
tcplookup_la_CFLAGS = -fPIC
tcplookup_la_LDFLAGS = -L.libs -module -avoid-version -shared -rdynamic -no-undefined -nostartfiles $(LDFLAGS)
tcplookupdir=@libdir@

libindimail_LTLIBRARIES = libindimail.la
libindimail_la_SOURCES = $(COMMONSOURCES)
libindimail_la_LIBADD = @LIB_MYSQL@ @LIB_QMAIL@ @LIB_SSL@ @LIB_GSASL@ -lm -ldl
libindimail_la_LDFLAGS = -version-info @LIBVER@ $(LDFLAGS)
libindimail_la_CFLAGS = -fPIC
libindimaildir=@libdir@

indimailbindir=@bindir@
indimailsbindir=@sbindir@
indimaillibexecdir=@libexecdir@
indimailauthlibdir=@libexecdir@/imapmodules

SYSCONFDIR = @sysconfdir@
sysconf_DATA = indimail.settings controlfiles.i headerlist osh.table \
	cronlist.i indimail.perms indimail.mrtg.cfg indimail-release indimail.schema

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA = libindimail.pc

BUILT_SOURCES = indimail_config.h
man_MANS=adminclient.8 chowkidar.8 clearopensmtp.8 crc.1 crcdiff.1 \
	ischema.1 incrypt.1 dbinfo.8 hostcntrl.8 hostsync.8 indimail.7 \
	IndiMail.7 indisrvr.8 initsvc.1 inlookup.8 InLookup.8 inquerytest.8 \
	install_tables.8 ipchange.8 ismaildup.1 login_tasks.3 \
	mgmtpass.8 osh.8 postdel.8 printdir.1 proxyimap.8 proxypop3.8 \
	resetquota.8 updaterules.8 vacation.8 tcplookup.8 qmailmrtg.1 \
	vaddaliasdomain.1 vadddomain.1 vadduser.1 valias.1 vatrn.1 \
	vbulletin.1 vcalias.8 vcaliasrev.8 vcfilter.1 vchkpass.8 vdeldomain.1 \
	vdelivermail.8 vdeloldusers.8 vdeluser.1 vdominfo.1 vfilter.8 \
	vfstab.8 vgroup.1 vhostid.8 vipmap.1 vlimit.1 vmoddomain.1 vmoduser.1 \
	vmoveuserdir.1 vpasswd.1 vpriv.8 vproxy.8 vrenamedomain.1 vrenameuser.1 \
	vreorg.8 vserverinfo.1 vsetpass.8 vsetuserquota.1 vsmtp.8 vtable.1 \
	vuserinfo.1

include_HEADERS = addaliasdomain.h add_control.h add_domain_assign.h addressToken.h \
	add_user_assign.h addusercntrl.h add_vacation.h adminCmmd.h \
	AliasInLookup.h atrn_map.h auth_admin.h autoturn_dir.h \
	backfill.h bulk_mail.h bulletin.h check_group.h Check_Login.h checkPerm.h \
	check_quota.h clear_open_smtp.h close_big_dir.h cntrl_clearaddflag.h \
	cntrl_cleardelflag.h common.h compile_morercpthosts.h CopyEmailFile.h \
	copyPwdStruct.h count_dir.h count_rcpthosts.h count_table.h crc.h \
	CreateDomainDirs.h create_table.h dbinfoAdd.h dbinfoDel.h dbinfoSelect.h \
	dbinfoUpdate.h dbload.h dblock.h del_control.h del_domain_assign.h \
	deldomain.h deliver_mail.h delunreadmails.h del_user_assign.h \
	delusercntrl.h deluser.h dir_control.h \
	disable_mysql_escape.h fappend.h FifoCreate.h filewrt.h \
	findhost.h findmdahost.h fstabChangeCounters.h getactualpath.h \
	getAddressBook.h get_assign.h getFreeFS.h getindimail.h \
	get_indimailuidgid.h getlastauth.h get_local_hostid.h get_local_ip.h \
	get_localtime.h get_message_size.h get_Mplexdir.h getpeer.h GetPrefix.h \
	get_real_domain.h GetSMTProute.h getuidgid.h gsasl_mkpasswd.h \
	hostcntrl_select.h host_in_locals.h iadddomain.h iadduser.h iclose.h \
	in_bsearch.h indimail_compat.h indimail.h inquery.h iopen.h ipasswd.h \
	ip_map.h is_alias_domain.h is_already_running.h is_distributed_domain.h \
	islocalif.h ismaildup.h is_user_present.h isvalid_domain.h \
	isvirtualdomain.h layout.h LoadBMF.h LoadDbInfo.h lockfile.h Login_Tasks.h \
	load_mysql.h lowerit.h maildir_to_domain.h maildir_to_email.h MailQuotaWarn.h \
	make_user_dir.h mdaMySQLConnect.h get_hashmethod.h \
	mgmtpassfuncs.h monkey.h MoveFile.h munch_domain.h mysql_stack.h \
	next_big_dir.h open_big_dir.h open_master.h open_smtp_relay.h \
	parseAddress.h parse_email.h parse_quota.h passwd_policy.h pathToFilesystem.h \
	pipe_exec.h post_handle.h print_control.h ProcessInFifo.h proxylogin.h \
	purge_files.h pwcomp.h PwdInLookup.h qmail_remote.h recalc_quota.h \
	RelayInLookup.h relay_select.h RemoteBulkMail.h remove_line.h remove_quotes.h \
	renameuser.h runcmmd.h SendWelcomeMail.h \
	set_mysql_options.h setuserquota.h skip_relay.h \
	skip_system_files.h sockwrite.h smtp_port.h spam.h sql_active.h sql_adddomain.h \
	sql_adduser.h sql_delaliasdomain.h sql_deldomain.h sql_deluser.h sql_getall.h \
	sql_getflags.h sql_gethostid.h sql_getip.h sql_getpw.h sql_get_realdomain.h \
	sql_init.h sql_insertaliasdomain.h sqlOpen_user.h sql_passwd.h sql_renamedomain.h \
	SqlServer.h sql_setpw.h sql_setquota.h sql_updateflag.h storeHeader.h \
	strToPw.h trashpurge.h update_file.h \
	update_local_hostid.h update_newu.h update_quota.h update_rules.h \
	updusercntrl.h userinfo.h UserInLookup.h user_over_quota.h valiasCount.h \
	valias_delete_domain.h valias_delete.h valiasinfo.h valias_insert.h \
	valias_select.h valias_update.h variables.h vdelfiles.h vfilter_delete.h \
	vfilter_display.h vfilter_filterNo.h vfilter_header.h vfilter_insert.h \
	vfilter_select.h vfilter_update.h vfstab.h vget_lastauth.h vgetpasswd.h \
	vhostid_delete.h vhostid_insert.h vhostid_select.h vhostid_update.h VlimitInLookup.h \
	vlimits.h vmake_maildir.h vpriv.h vquota_select.h vset_default_domain.h \
	vset_lastauth.h vset_lastdeliver.h vsmtp_delete_domain.h vsmtp_delete.h vsmtp_insert.h \
	vsmtp_select.h vsmtp_update.h vupdate_rules.h wildmat.h indimail_config.h
includedir=@prefix@/include/indimail

COMMONSOURCES = addaliasdomain.c add_atrn_map.c add_control.c add_domain_assign.c add_ip_map.c \
	add_user_assign.c addusercntrl.c add_vacation.c atrn_access.c autoturn_dir.c \
	backfill.c bulk_mail.c check_group.c Check_Login.c check_quota.c close_big_dir.c \
	cntrl_clearaddflag.c cntrl_cleardelflag.c common.c compile_morercpthosts.c \
	CopyEmailFile.c copyPwdStruct.c count_dir.c count_rcpthosts.c count_table.c \
	CreateDomainDirs.c create_table.c dbinfoAdd.c dbinfoDel.c dbinfoSelect.c \
	dbinfoUpdate.c dbload.c dec_dir_control.c del_atrn_map.c del_control.c \
	del_domain_assign.c deldomain.c del_ip_map.c del_user_assign.c compat.c \
	deluser.c delusercntrl.c fappend.c \
	FifoCreate.c filewrt.c findhost.c findmdahost.c fstabChangeCounters.c \
	getactualpath.c get_assign.c getDbLock.c \
	get_indimailuidgid.c get_local_hostid.c get_local_ip.c get_localtime.c \
	get_message_size.c get_Mplexdir.c getpeer.c GetPrefix.c get_real_domain.c \
	GetSMTProute.c gsasl_mkpasswd.c hostcntrl_select.c \
	host_in_locals.c iadddomain.c iadduser.c iclose.c inc_dir.c \
	inc_dir_control.c init_dir_control.c inquery.c iopen.c \
	ipasswd.c is_alias_domain.c is_already_running.c is_distributed_domain.c \
	islocalif.c ismaildup.c is_user_present.c isvirtualdomain.c \
	layout.c limits.c LoadDbInfo.c lockfile.c Login_Tasks.c lowerit.c \
	maildir_to_domain.c maildir_to_email.c MailQuotaWarn.c make_user_dir.c \
	mdaMySQLConnect.c mgmtpassfuncs.c MoveFile.c munch_domain.c \
	mysql_query.c mysql_stack.c next_big_dir.c open_big_dir.c \
	open_master.c open_smtp_relay.c parse_email.c parse_quota.c passwd_policy.c \
	pathToFilesystem.c pipe_exec.c post_handle.c print_control.c \
	pwcomp.c recalc_quota.c relay_select.c RemoteBulkMail.c remove_line.c \
	remove_quotes.c renameuser.c runcmmd.c get_hashmethod.c \
	SendWelcomeMail.c set_mysql_options.c setuserquota.c \
	show_atrn_map.c show_ip_map.c skip_relay.c \
	skip_system_files.c smtp_port.c sql_active.c sql_adddomain.c sql_adduser.c \
	sql_delaliasdomain.c sql_deldomain.c sql_deluser.c sql_getall.c \
	sql_getflags.c sql_gethostid.c sql_getip.c sql_getpw.c \
	sql_get_realdomain.c sql_init.c sql_insertaliasdomain.c sqlOpen_user.c \
	sql_passwd.c sql_renamedomain.c SqlServer.c sql_setpw.c sql_setquota.c \
	sql_updateflag.c strToPw.c update_file.c \
	update_local_hostid.c update_newu.c update_quota.c update_rules.c \
	upd_atrn_map.c upd_ip_map.c updusercntrl.c userinfo.c user_over_quota.c \
	valiasCount.c valias_delete.c valias_delete_domain.c valiasinfo.c \
	valias_insert.c valias_select.c valias_update.c variables.c \
	vcreate_dir_control.c vdel_dir_control.c vdelfiles.c vfilter_delete.c \
	vfilter_display.c vfilter_filterNo.c vfilter_header.c vfilter_insert.c \
	vfilter_select.c vfilter_update.c vget_ip_map.c vget_lastauth.c vgetpasswd.c \
	vhostid_delete.c vhostid_insert.c vhostid_select.c vhostid_update.c \
	vmake_maildir.c vpriv_delete.c vpriv_insert.c \
	vpriv_select.c vpriv_update.c vquota_select.c vread_dir_control.c \
	vset_default_domain.c vset_lastauth.c vset_lastdeliver.c vsmtp_delete.c \
	vsmtp_delete_domain.c vsmtp_insert.c vsmtp_select.c vsmtp_update.c \
	vupdate_rules.c vwrite_dir_control.c wildmat.c \
	load_mysql.c

libindimail.la: $(libindimail_la_OBJECTS) $(libindimail_la_DEPENDENCIES)
	$(LIBTOOL) --mode=link --tag=CC gcc $(libindimail_la_LDFLAGS) \
	-O -o libindimail.la $(libindimail_la_OBJECTS) $(libindimail_la_LIBADD) \
	-rpath @libdir@

iauth.la: $(iauth_la_OBJECTS) $(iauth_la_DEPENDENCIES)
	$(LIBTOOL) --mode=link $(COMPILE) $(MODFLAGS) \
	-o $@ iauth.lo $(iauth_la_LDFLAGS) \
	$(iauth_la_LIBADD) -rpath @libdir@

tcplookup.la: $(tcplookup_la_OBJECTS) $(tcplookup_la_DEPENDENCIES)
	$(LIBTOOL) --mode=link $(COMPILE) $(MODFLAGS) \
	-o $@ mytcplookup.lo $(tcplookup_la_LDFLAGS) \
	$(tcplookup_la_LIBADD) -rpath @libdir@

indimailbin_PROGRAMS = vadddomain vdeldomain vadduser vdeluser vuserinfo \
	vaddaliasdomain vrenamedomain dbinfo vdominfo \
	vmoduser printdir ismaildup valias vcalias hostcntrl \
	vhostid vpasswd vcfilter vsmtp vsetuserquota \
	vacation vatrn vmoveuserdir vrenameuser vipmap vlimit \
	vpriv crc crcdiff incrypt vgroup proxyimap proxypop3 \
	vproxy vbulletin

indimailsbin_PROGRAMS = inlookup ischema tcplookup vdelivermail clearopensmtp \
	updaterules vfilter vmoddomain inquerytest vreorg vtable \
	install_tables hostsync vserverinfo initsvc \
	mgmtpass vchkpass vsetpass resetquota postdel \
	ipchange adminclient indisrvr vdeloldusers vfstab \
	osh chowkidar vpurge

indimaillibexec_PROGRAMS = qmailmrtg updatefile
indimaillibexec_SCRIPTS = mailzipper overquota.sh controlsync cputime \
	myslave bogofilter-qfe iupgrade.sh ilocal_upgrade.sh journal2mrtg

indimailauthlib_PROGRAMS = authindi

indimailauthlib_SCRIPTS = authgeneric

indimailbin_SCRIPTS = vcaliasrev

noinst_SCRIPTS = vadddomain_handle vadduser_handle vdeldomain_handle \
	vaddaliasdomain_handle vrenamedomain_handle vpasswd_handle vmoduser_handle \
	vdeluser_handle vrenameuser_handle

setuid_list = bin/printdir bin/vaddaliasdomain bin/vadddomain bin/vadduser bin/vbulletin \
	bin/vcfilter bin/vdeldomain bin/vdeluser bin/vmoduser bin/vmoveuserdir bin/vrenamedomain \
	bin/vrenameuser bin/vsetuserquota bin/vdominfo bin/vuserinfo sbin/vreorg

noinst_DATA = indimail-permissions.easy indimail-permissions.paranoid \
	indimail-permissions.secure indimail.spec indimail.changes indimail-rpmlintrc \
	PKGBUILD

post_handle_list = vadddomain vadduser vdeldomain vaddaliasdomain vrenamedomain vmoduser vpasswd \
	vdeluser vrenameuser

doc_list= ChangeLog FAQ.md README-indimail.md README-ldap.md  \
	README-vlimits.md COPYING-indimail

MOSTLYCLEANFILES=vcaliasrev myslave controlsync bogofilter-qfe \
	cronlist.i incrypt vdeluser_handle vrenameuser_handle \
	cscope.* *.dSYM mailzipper mytcplookup.c tcplookup.8 vaddaliasdomain.1 \
	indimail.perms dbinfo.8 inlookup.8 vadddomain.1 vdeldomain.1 vadduser.1 \
	vdeluser.1 vdelivermail.8 vrenamedomain.1 vrenameuser.1 vpasswd.1 vmoveuserdir.1 \
	vcfilter.1 indimail.7 indimail.mrtg.cfg overquota.sh \
	vadddomain_handle vadduser_handle vpasswd_handle vdeldomain_handle vaddaliasdomain_handle \
	vrenamedomain_handle login_tasks.3 authgeneric usr.sbin.mysqld usr.sbin.clamd \
	vmoduser_handle vmoduser.1 usr.bin.freshclam indimail.mod indimail.pp \
	indimail_config.h debian/Makefile archpkg*.install

qmailmrtg_SOURCES = qmailmrtg.c
qmailmrtg_LDADD = libindimail.la $(LIB_QMAIL)
qmailmrtg_DEPENDENCIES = libindimail.la

vadddomain_SOURCES = vadddomain.c
vadddomain_LDADD = isvalid_domain.lo libindimail.la $(LIB_QMAIL)
vadddomain_DEPENDENCIES = isvalid_domain.lo libindimail.la

vaddaliasdomain_SOURCES = vaddaliasdomain.c
vaddaliasdomain_LDADD = libindimail.la $(LIB_QMAIL)
vaddaliasdomain_DEPENDENCIES = libindimail.la

vdeldomain_SOURCES = vdeldomain.c
vdeldomain_LDADD = libindimail.la $(LIB_QMAIL)
vdeldomain_DEPENDENCIES = libindimail.la

vrenamedomain_SOURCES = vrenamedomain.c
vrenamedomain_LDADD = libindimail.la $(LIB_QMAIL)
vrenamedomain_DEPENDENCIES = libindimail.la

vmoddomain_SOURCES = vmoddomain.c
vmoddomain_LDADD = libindimail.la $(LIB_QMAIL)
vmoddomain_DEPENDENCIES = libindimail.la

vadduser_SOURCES = vadduser.c
vadduser_LDADD = libindimail.la $(LIB_QMAIL)
vadduser_DEPENDENCIES = libindimail.la

vgroup_SOURCES = vgroup.c
vgroup_LDADD = libindimail.la $(LIB_QMAIL)
vgroup_DEPENDENCIES = libindimail.la

vpasswd_SOURCES = vpasswd.c
vpasswd_LDADD = libindimail.la $(LIB_QMAIL)
vpasswd_DEPENDENCIES = libindimail.la

vdeluser_SOURCES = vdeluser.c
vdeluser_LDADD = libindimail.la $(LIB_QMAIL)
vdeluser_DEPENDENCIES = libindimail.la

vmoduser_SOURCES = vmoduser.c
vmoduser_LDADD = libindimail.la $(LIB_QMAIL)
vmoduser_DEPENDENCIES = libindimail.la

vuserinfo_SOURCES = vuserinfo.c
vuserinfo_LDADD = libindimail.la $(LIB_QMAIL)
vuserinfo_DEPENDENCIES = libindimail.la

dbinfo_SOURCES = dbinfo.c
dbinfo_LDADD = libindimail.la $(LIB_QMAIL)
dbinfo_DEPENDENCIES = libindimail.la

vdominfo_SOURCES = vdominfo.c
vdominfo_LDADD = libindimail.la $(LIB_QMAIL)
vdominfo_DEPENDENCIES = libindimail.la

hostcntrl_SOURCES = hostcntrl.c
hostcntrl_LDADD = libindimail.la $(LIB_QMAIL)
hostcntrl_DEPENDENCIES = libindimail.la

vhostid_SOURCES = vhostid.c
vhostid_LDADD = libindimail.la $(LIB_QMAIL)
vhostid_DEPENDENCIES = libindimail.la

vsmtp_SOURCES = vsmtp.c
vsmtp_LDADD = libindimail.la $(LIB_QMAIL)
vsmtp_DEPENDENCIES = libindimail.la

inlookup_SOURCES = inlookup.c
inlookup_LDADD = ProcessInFifo.lo AliasInLookup.lo PwdInLookup.lo \
	RelayInLookup.lo UserInLookup.lo VlimitInLookup.lo libindimail.la $(LIB_QMAIL)
inlookup_DEPENDENCIES = ProcessInFifo.lo AliasInLookup.lo PwdInLookup.lo RelayInLookup.lo \
	UserInLookup.lo VlimitInLookup.lo libindimail.la

tcplookup_SOURCES = tcplookup.c
tcplookup_LDADD = libindimail.la $(LIB_QMAIL)
tcplookup_CFLAGS = -DMAIN
tcplookup_DEPENDENCIES = libindimail.la

printdir_SOURCES = printdir.c
printdir_LDADD = libindimail.la $(LIB_QMAIL)
printdir_DEPENDENCIES = libindimail.la

vdelivermail_SOURCES = vdelivermail.c
vdelivermail_LDADD = deliver_mail.lo qmail_remote.lo libindimail.la $(LIB_QMAIL)
vdelivermail_DEPENDENCIES = deliver_mail.lo qmail_remote.lo libindimail.la

vfilter_SOURCES = vfilter.c
vfilter_LDADD = storeHeader.lo parseAddress.lo deliver_mail.lo \
	addressToken.lo getAddressBook.lo \
	eps-1.2/libeps.la libindimail.la $(LIB_QMAIL)
vfilter_DEPENDENCIES=storeHeader.lo parseAddress.lo deliver_mail.lo \
	addressToken.lo getAddressBook.lo libindimail.la

include Makefile.@SYSTEM@

vcfilter_SOURCES = vcfilter.c
vcfilter_LDADD = libindimail.la $(LIB_QMAIL)
vcfilter_DEPENDENCIES = libindimail.la

ismaildup_SOURCES = ismaildup.c
ismaildup_LDADD = libindimail.la $(LIB_QMAIL) @LIB_SSL@
ismaildup_CFLAGS = -DMAIN
ismaildup_DEPENDENCIES = libindimail.la

valias_SOURCES = valias.c
valias_LDADD = libindimail.la $(LIB_QMAIL)
valias_DEPENDENCIES = libindimail.la

vcalias_SOURCES = vcalias.c
vcalias_LDADD = libindimail.la $(LIB_QMAIL)
vcalias_DEPENDENCIES = libindimail.la

clearopensmtp_SOURCES = clearopensmtp.c
clearopensmtp_LDADD = clear_open_smtp.lo libindimail.la $(LIB_QMAIL)
clearopensmtp_DEPENDENCIES = clear_open_smtp.lo libindimail.la

updaterules_SOURCES = updaterules.c
updaterules_LDADD = libindimail.la $(LIB_QMAIL)
updaterules_DEPENDENCIES = libindimail.la

inquerytest_SOURCES = inquerytest.c
inquerytest_LDADD = ProcessInFifo.lo  AliasInLookup.lo PwdInLookup.lo \
	RelayInLookup.lo UserInLookup.lo VlimitInLookup.lo libindimail.la $(LIB_QMAIL) -ldl
inquerytest_DEPENDENCIES = ProcessInFifo.lo  AliasInLookup.lo PwdInLookup.lo RelayInLookup.lo \
	UserInLookup.lo VlimitInLookup.lo libindimail.la

vreorg_SOURCES = vreorg.c
vreorg_LDADD = libindimail.la $(LIB_QMAIL)
vreorg_DEPENDENCIES = libindimail.la

install_tables_SOURCES = install_tables.c
install_tables_LDADD = libindimail.la $(LIB_QMAIL)
install_tables_DEPENDENCIES = libindimail.la

vtable_SOURCES = vtable.c
vtable_LDADD = libindimail.la $(LIB_QMAIL)
vtable_DEPENDENCIES = libindimail.la

hostsync_SOURCES = hostsync.c
hostsync_LDADD = libindimail.la $(LIB_QMAIL)
hostsync_DEPENDENCIES = libindimail.la

vserverinfo_SOURCES = vserverinfo.c
vserverinfo_LDADD = libindimail.la $(LIB_QMAIL)
vserverinfo_DEPENDENCIES = libindimail.la

initsvc_SOURCES = initsvc.c
initsvc_LDADD = common.lo fappend.lo $(LIB_QMAIL)
initsvc_DEPENDENCIES = common.lo fappend.lo

mgmtpass_SOURCES = mgmtpass.c
mgmtpass_LDADD = libindimail.la $(LIB_QMAIL)
mgmtpass_DEPENDENCIES = libindimail.la

authindi_SOURCES = authindi.c
authindi_LDADD = libindimail.la $(LIB_QMAIL)
authindi_DEPENDENCIES = libindimail.la

vchkpass_SOURCES = vchkpass.c
vchkpass_LDADD = libindimail.la $(LIB_QMAIL)
vchkpass_DEPENDENCIES = libindimail.la

ischema_SOURCES = ischema.c
ischema_LDADD = libindimail.la $(LIB_QMAIL)
ischema_DEPENDENCIES = libindimail.la

vsetpass_SOURCES = vsetpass.c
vsetpass_LDADD = libindimail.la $(LIB_QMAIL)
vsetpass_DEPENDENCIES = libindimail.la

vsetuserquota_SOURCES = vsetuserquota.c
vsetuserquota_LDADD = libindimail.la $(LIB_QMAIL)
vsetuserquota_DEPENDENCIES = libindimail.la

vacation_SOURCES = vacation.c
vacation_LDADD = libindimail.la $(LIB_QMAIL)
vacation_DEPENDENCIES = libindimail.la

vatrn_SOURCES = vatrn.c
vatrn_LDADD = libindimail.la $(LIB_QMAIL)
vatrn_DEPENDENCIES = libindimail.la

vmoveuserdir_SOURCES = vmoveuserdir.c
vmoveuserdir_LDADD = libindimail.la $(LIB_QMAIL)
vmoveuserdir_DEPENDENCIES = libindimail.la

vrenameuser_SOURCES = vrenameuser.c
vrenameuser_LDADD = libindimail.la $(LIB_QMAIL)
vrenameuser_DEPENDENCIES = libindimail.la

vipmap_SOURCES = vipmap.c
vipmap_LDADD = libindimail.la $(LIB_QMAIL)
vipmap_DEPENDENCIES = libindimail.la

vlimit_SOURCES = vlimit.c
vlimit_LDADD = libindimail.la $(LIB_QMAIL)
vlimit_DEPENDENCIES = libindimail.la

vpriv_SOURCES = vpriv.c
vpriv_LDADD = libindimail.la $(LIB_QMAIL)
vpriv_DEPENDENCIES = libindimail.la

crc_SOURCES = crc.c
crc_LDADD = libindimail.la $(LIB_QMAIL)
crc_DEPENDENCIES = libindimail.la

crcdiff_SOURCES = crcdiff.c

incrypt_SOURCES = incrypt.c
incrypt_LDADD = libindimail.la @LIB_CRYPT@ $(LIB_QMAIL)
incrypt_DEPENDENCIES = libindimail.la

ipchange_SOURCES = ipchange.c
ipchange_LDADD = libindimail.la $(LIB_QMAIL)
ipchange_DEPENDENCIES = libindimail.la

postdel_SOURCES = postdel.c
postdel_LDADD = $(LIB_QMAIL)

resetquota_SOURCES = resetquota.c
resetquota_LDADD = libindimail.la $(LIB_QMAIL)
resetquota_DEPENDENCIES = libindimail.la

proxyimap_SOURCES = proxyimap.c
proxyimap_LDADD = proxylogin.lo auth_admin.lo adminCmmd.lo \
	monkey.lo sockwrite.lo libindimail.la $(LIB_QMAIL) @LIB_SSL@
proxyimap_DEPENDENCIES = proxylogin.lo auth_admin.lo adminCmmd.lo \
	monkey.lo sockwrite.lo libindimail.la

proxypop3_SOURCES = proxypop3.c
proxypop3_LDADD = proxylogin.lo auth_admin.lo adminCmmd.lo \
	monkey.lo sockwrite.lo libindimail.la $(LIB_QMAIL) @LIB_SSL@
proxypop3_DEPENDENCIES = proxylogin.lo auth_admin.lo adminCmmd.lo \
	monkey.lo sockwrite.lo libindimail.la

vproxy_SOURCES = vproxy.c
vproxy_LDADD = monkey.lo sockwrite.lo libindimail.la $(LIB_QMAIL)
vproxy_DEPENDENCIES = monkey.lo sockwrite.lo libindimail.la

vbulletin_SOURCES = vbulletin.c
vbulletin_LDADD = bulletin.lo libindimail.la $(LIB_QMAIL)
vbulletin_DEPENDENCIES = bulletin.lo libindimail.la

adminclient_SOURCES = adminclient.c
adminclient_LDADD = auth_admin.lo adminCmmd.lo libindimail.la $(LIB_QMAIL) @LIB_SSL@
adminclient_DEPENDENCIES = auth_admin.lo adminCmmd.lo libindimail.la

indisrvr_SOURCES = indisrvr.c
indisrvr_LDADD = checkPerm.lo libindimail.la @LIB_SOCKET@ $(LIB_QMAIL) @LIB_SSL@
indisrvr_DEPENDENCIES = checkPerm.lo libindimail.la

vdeloldusers_SOURCES = vdeloldusers.c
vdeloldusers_LDADD = delunreadmails.lo getlastauth.lo \
	trashpurge.lo getindimail.lo libindimail.la $(LIB_QMAIL)
vdeloldusers_DEPENDENCIES = delunreadmails.lo getlastauth.lo \
	trashpurge.lo getindimail.lo libindimail.la

vfstab_SOURCES = vfstab.c
vfstab_LDADD = vfstab_select.lo vfstab_insert.lo vfstab_update.lo \
	vfstab_delete.lo vfstabNew.lo vfstab_status.lo getFreeFS.lo libindimail.la $(LIB_QMAIL)
vfstab_DEPENDENCIES = vfstab_select.lo vfstab_insert.lo vfstab_update.lo \
	vfstab_delete.lo vfstabNew.lo vfstab_status.lo getFreeFS.lo \
	libindimail.la

vpurge_SOURCES = vpurge.c
vpurge_LDADD = purge_files.lo libindimail.la $(LIB_QMAIL)
vpurge_DEPENDENCIES = purge_files.lo libindimail.la

chowkidar_SOURCES = chowkidar.c
chowkidar_LDADD = LoadBMF.lo spam.lo libindimail.la $(LIB_QMAIL)
chowkidar_DEPENDENCIES = LoadBMF.lo spam.lo libindimail.la

osh_SOURCES = osh.c
osh_LDADD = @LIB_CURSES@ @LIB_TERMCAP@ exp.lo handlers.lo @LIB_QMAIL@
osh_DEPENDENCIES = exp.lo handlers.lo

updatefile_SOURCES = updatefile.c
updatefile_LDADD = libindimail.la $(LIB_QMAIL)
updatefile_DEPENDENCIES = libindimail.la

edit = sed \
	-e 's}@indimaildir\@}$(indimaildir)}g' \
	-e 's}@qmaildir\@}$(qmaildir)}g' \
	-e 's}@servicedir\@}$(servicedir)}g' \
	-e 's}@controldir\@}$(controldir)}g' \
	-e 's}@sysconfdir\@}$(sysconfdir)}g' \
	-e 's}@mandir\@}$(mandir)}g' \
	-e 's}@libexecdir\@}$(libexecdir)}g' \
	-e 's}@domaindir\@}$(domaindir)}g' \
	-e 's}@mysqlbindir\@}$(mysqlbindir)}g' \
	-e 's}@moduledir\@}$(moduledir)}g' \
	-e 's}@libdir\@}$(libdir)}g' \
	-e 's}@logdir\@}$(logdir)}g' \
	-e 's}@basepath\@}$(basepath)}g' \
	-e 's}@mysql_prefix\@}$(mysql_prefix)}g' \
	-e 's}@mysql_incdir\@}$(mysql_incdir)}g' \
	-e 's}@mysql_libdir\@}$(mysql_libdir)}g' \
	-e 's}@defaultdomain\@}$(defaultdomain)}g' \
	-e 's}@PACKAGE\@}$(PACKAGE)}g' \
	-e 's}@PACKAGE_VERSION\@}$(PACKAGE_VERSION)}g' \
	-e 's}@version\@}$(VERSION)}g' \
	-e "s|@release\@|"`cat conf-release`"|g" \
	-e 's|@email\@|'"`cat conf-email`"'|g' \
	-e 's}@pkgconfigdir\@}$(pkgconfigdir)}g' \
	-e 's}@VERSION\@}$(VERSION)}g' \
	-e 's}@DESTDIR\@}$(DESTDIR)}g' \
	-e 's}@HOST\@}$(host)}g' \
	-e 's}@DATE\@}$(DATE)}g' \
	-e 's}@prefix\@}$(prefix)}g'
editman = sed \
	-e 's}INDIMAILDIR}$(indimaildir)}g' \
	-e 's}@indimaildir\@}$(indimaildir)}g' \
	-e 's}@sysconfdir\@}$(sysconfdir)}g' \
	-e 's}@qmaildir\@}$(qmaildir)}g' \
	-e 's}@controldir\@}$(controldir)}g' \
	-e 's}@libexecdir\@}$(libexecdir)}g' \
	-e 's}@servicedir\@}$(servicedir)}g' \
	-e 's}@logdir\@}$(logdir)}g' \
	-e 's}@prefix\@}$(prefix)}g'

indimail.fc: indimail.fc.in
	$(edit) $@.in > $@
controlsync: controlsync.in
	$(edit) $@.in > $@
mailzipper: mailzipper.in
	$(edit) $@.in > $@
vcaliasrev: vcaliasrev.in
	$(edit) $@.in > $@
myslave: myslave.in
	$(edit) $@.in > $@
indimail.mrtg.cfg: indimail.mrtg.cfg.in
	$(edit) $@.in > $@
overquota.sh: overquota.sh.in
	$(edit) $@.in > $@
ilocal_upgrade.sh: ilocal_upgrade.in
	$(edit) ilocal_upgrade.in > $@
journal2mrtg: journal2mrtg.in
	$(edit) journal2mrtg.in > $@
authgeneric: authgeneric.in
	$(edit) $@.in > $@
bogofilter-qfe: bogofilter-qfe.in
	$(edit) $@.in > $@
cronlist.i: cronlist.in
	(echo "# DO NOT EDIT THIS FILE - edit the master and reinstall."; $(edit) cronlist.in) > $@
indimail-permissions.easy: indimail-permissions.easy.in
	$(edit) $@.in > $@
indimail-permissions.secure: indimail-permissions.secure.in
	$(edit) $@.in > $@
indimail-permissions.paranoid: indimail-permissions.paranoid.in
	$(edit) $@.in > $@
indimail.perms: perm_list.in
	$(edit) perm_list.in > $@
usr.sbin.mysqld: usr.sbin.mysqld.in
	$(edit) $@.in > $@
usr.sbin.clamd: usr.sbin.clamd.in
	$(edit) $@.in > $@
usr.bin.freshclam: usr.bin.freshclam.in
	$(edit) $@.in > $@
indimail_config.h: config.h
	grep -E -v "#define PACKAGE|#define VERSION" config.h > indimail_config.h

debian/Makefile: debian/Makefile.in configure.ac conf-version
	$(edit) $@.in > $@;

post_handle:
	mkdir -p post_handle
vadddomain_handle: vadddomain_handle.in post_handle
	$(edit) $@.in > post_handle/vadddomain; ln -sf post_handle/vadddomain $@
vaddaliasdomain_handle: vaddaliasdomain_handle.in post_handle
	$(edit) $@.in > post_handle/vaddaliasdomain; ln -sf post_handle/vaddaliasdomain $@
vrenamedomain_handle: vrenamedomain_handle.in post_handle
	$(edit) $@.in > post_handle/vrenamedomain; ln -sf post_handle/vrenamedomain $@
vdeldomain_handle: vdeldomain_handle.in post_handle
	$(edit) $@.in > post_handle/vdeldomain; ln -sf post_handle/vdeldomain $@
vadduser_handle: vadduser_handle.in post_handle
	$(edit) $@.in > post_handle/vadduser; ln -sf post_handle/vadduser $@
vpasswd_handle: vpasswd_handle.in post_handle
	$(edit) $@.in > post_handle/vpasswd; ln -sf post_handle/vpasswd $@
vmoduser_handle: vmoduser_handle.in post_handle
	$(edit) $@.in > post_handle/vmoduser; ln -sf post_handle/vmoduser $@
vdeluser_handle: vdeluser_handle.in post_handle
	$(edit) $@.in > post_handle/vdeluser; ln -sf post_handle/vdeluser $@
vrenameuser_handle: vrenameuser_handle.in post_handle
	$(edit) $@.in > post_handle/vrenameuser; ln -sf post_handle/vrenameuser $@

conf-release:
	if [ ! -f conf-release ] ; then \
		echo 1.1 > conf-release; \
	fi
indimail.spec: indimail.spec.in doc/ChangeLog conf-version \
conf-release conf-email
	(cat $@.in;./catChangeLog --spec doc/ChangeLog) | $(edit) > $@
indimail.changes: doc/ChangeLog conf-version conf-release conf-email
	./catChangeLog --changes doc/ChangeLog > $@
PKGBUILD: PKGBUILD.in conf-version conf-release conf-email \
indimail.changes
	cat $@.in | $(edit) > $@
indimail-rpmlintrc: indimail-rpmlintrc.in
	$(edit) indimail-rpmlintrc.in > $@
login_tasks.3: login_tasks.9
	$(editman) login_tasks.9 > $@
dbinfo.8: dbinfo.9
	cat dbinfo.9 | sed s}INDIMAIL}$(indimaildir)}g > $@
inlookup.8: inlookup.9
	$(editman) inlookup.9 > $@
tcplookup.8: tcplookup.9
	$(editman) tcplookup.9 > $@
vcfilter.1: vcfilter.9
	$(editman) vcfilter.9 > $@
vaddaliasdomain.1: vaddaliasdomain.9
	$(editman) vaddaliasdomain.9 > $@
vadddomain.1: vadddomain.9
	$(editman) vadddomain.9 > $@
vdeldomain.1: vdeldomain.9
	$(editman) vdeldomain.9 > $@
vadduser.1: vadduser.9
	$(editman) vadduser.9 > $@
vdeluser.1: vdeluser.9
	$(editman) vdeluser.9 > $@
vrenamedomain.1: vrenamedomain.9
	$(editman) vrenamedomain.9 > $@
vrenameuser.1: vrenameuser.9
	$(editman) vrenameuser.9 > $@
vpasswd.1: vpasswd.9
	$(editman) vpasswd.9 > $@
vmoduser.1: vmoduser.9
	$(editman) vmoduser.9 > $@
vmoveuserdir.1: vmoveuserdir.9
	$(editman) vmoveuserdir.9 > $@
vdelivermail.8: vdelivermail.9
	$(editman) vdelivermail.9 > $@
ischema.1: ischema.9
	$(editman) ischema.9 > $@
indimail.7: indimail.9
	$(editman) indimail.9 > $@
inquerytest.8: inquerytest.9
	$(editman) inquerytest.9 > $@
qmailmrtg.1: qmailmrtg.9
	$(editman) qmailmrtg.9 > $@
ismaildup.1: ismaildup.9
	$(editman) ismaildup.9 > $@

mytcplookup.c: tcplookup.c
	cp tcplookup.c mytcplookup.c

crc.$(OBJEXT): crc.c
	$(COMPILE) $(DEFS) -DMAIN -c crc.c
crc$(EXEEXT): $(crc_DEPENDENCIES) $(crc_OBJECTS)
	$(LINK) $@.$(OBJEXT) $(crc_LDADD)

install-data-local: initsvc
	if [ " $(DESTDIR)" = " " ] ; then \
		if test `@idcommand@` != "0" ; then \
			echo "you are not root"; \
			exit 1; \
		fi ; \
		echo "Shutting down IndiMail"; \
		./initsvc -off || exit 0; \
	fi;

# This is the last step in indimail installation.
install-data-hook: iauth.la indimail.te roundcubemail.te indimail.fc usr.sbin.mysqld usr.sbin.clamd usr.bin.freshclam
	$(mkinstalldirs) $(DESTDIR)@domaindir@ || exit 1
	$(mkinstalldirs) $(DESTDIR)@prefix@/lib/indimail/modules || exit 1
	$(INSTALL_DATA) $(DESTDIR)@libdir@/iauth.so  $(DESTDIR)@prefix@/lib/indimail/modules || exit 1
	/bin/rm -f $(DESTDIR)@libdir@/iauth.* || exit 1
	$(INSTALL_DATA) $(DESTDIR)@libdir@/tcplookup.so  $(DESTDIR)@prefix@/lib/indimail/modules || exit 1
	/bin/rm -f $(DESTDIR)@libdir@/tcplookup.* || exit 1
	$(mkinstalldirs) $(DESTDIR)@libexecdir@ || exit 1
	$(INSTALL_DATA) post_handle/* $(DESTDIR)@libexecdir@ || exit 1
	for f in $(post_handle_list); do \
		if [ -f $(DESTDIR)@libexecdir@/$$f ] ; then \
			chmod 755 $(DESTDIR)@libexecdir@/$$f; \
			echo "chmod 755 $(DESTDIR)@libexecdir@/$$f"; \
		fi;\
	done
	$(mkinstalldirs) $(DESTDIR)@datarootdir@/doc || exit 1
	for f in $(doc_list); do \
		if [ "$$f" = "ChangeLog" ] ; then \
			$(INSTALL_DATA) doc/$$f $(DESTDIR)@datarootdir@/doc/$$f-indimail || exit 1 ;\
		else \
			$(INSTALL_DATA) doc/$$f $(DESTDIR)@datarootdir@/doc/$$f || exit 1 ;\
		fi; \
	done
	if test `@idcommand@` = "0" ; then \
		for f in $(setuid_list); do \
			if [ -f $(DESTDIR)@prefix@/$$f ] ; then \
				echo "chown root $(DESTDIR)@prefix@/$$f"; \
				chown root $(DESTDIR)@prefix@/$$f; \
				echo "chmod 4555 $(DESTDIR)@prefix@/$$f"; \
				chmod 4555 $(DESTDIR)@prefix@/$$f; \
			fi;  \
		done; \
	fi
	if test -n "$(DISTRIB_ID)" ; then \
		if test " $(DISTRIB_ID)" = " Ubuntu" -o " $(DISTRIB_ID)" = " Debian" ; then \
			$(mkinstalldirs) $(DESTDIR)@sysconfdir@/apparmor.d/local || exit 1; \
			$(INSTALL_DATA) usr.sbin.mysqld $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.sbin.mysqld  || exit 1; \
			$(INSTALL_DATA) usr.sbin.clamd $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.sbin.clamd  || exit 1; \
			$(INSTALL_DATA) usr.bin.freshclam $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.bin.freshclam  || exit 1; \
		fi; \
		if test " $(DISTRIB_ID)" != " Ubuntu" -a " $(DISTRIB_ID)" != " Debian" ; then \
			$(INSTALL_DATA) indimail.te $(DESTDIR)@sysconfdir@/indimail.te  || exit 1; \
			$(INSTALL_DATA) roundcubemail.te $(DESTDIR)@sysconfdir@/roundcubemail.te  || exit 1; \
			$(INSTALL_DATA) indimail.fc $(DESTDIR)@sysconfdir@/indimail.fc  || exit 1; \
		fi; \
	else \
		$(INSTALL_DATA) indimail.te $(DESTDIR)@sysconfdir@/indimail.te  || exit 1; \
		$(INSTALL_DATA) roundcubemail.te $(DESTDIR)@sysconfdir@/roundcubemail.te  || exit 1; \
		$(INSTALL_DATA) indimail.fc $(DESTDIR)@sysconfdir@/indimail.fc  || exit 1; \
	fi

uninstall-hook:
	/bin/rm -f $(DESTDIR)@prefix@/lib/indimail/modules/iauth.so
	/bin/rmdir $(DESTDIR)@prefix@/lib/indimail/modules || true
	for f in $(post_handle_list); do \
		if [ -f $(DESTDIR)@libexecdir@/$$f ] ; then \
			/bin/rm -f $(DESTDIR)@libexecdir@/$$f; \
		fi;\
	done
	/bin/rmdir $(DESTDIR)@prefix@/lib/indimail/modules || true
	for f in $(doc_list); do \
		/bin/rm -f $(DESTDIR)@datarootdir@/doc/$$f || exit 1 ;\
	done
	/bin/rmdir $(DESTDIR)@datarootdir@/doc || true


LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck
