@SET_MAKE@
SUBDIRS=eps-1.2

COFLAGS=-z+05:30
ACLOCAL_AMFLAGS=-I m4
release := $(shell cat conf-release 2>/dev/null|| echo "1.1")

DEFS=@DEFS@ -Dlint -fno-strict-aliasing -I. -I/usr/include/qmail -Ieps-1.2 @auth_inc@

INSTALL_PROGRAM = ${INSTALL}          -c
install_sh_PROGRAM = $(install_sh)    -c
INSTALL_SCRIPT = ${INSTALL}           -c
install_sh_SCRIPT = $(install_sh)     -c
INSTALL_STRIP_PROGRAM = $(install_sh) -c $(STRIP_OPT)

indimailbindir=@prefix@/bin
indimailsbindir=@prefix@/sbin
indimaillibexecdir=@libexecdir@
indimailauthlibdir=@libexecdir@/imapmodules

libindimail_LTLIBRARIES = libindimail.la
libindimail_la_SOURCES = $(COMMONSOURCES)
libindimail_la_LIBADD = @LIB_MYSQL@ $(LIB_QMAIL) -lm
libindimail_la_LDFLAGS = -version-info @LIBVER@
libindimaildir=@libdir@

iauth_LTLIBRARIES = iauth.la
iauth_la_SOURCES = iauth.c
iauth_la_LIBADD = $(top_builddir)/libindimail.la
iauth_la_DEPENDENCIES = $(top_builddir)/libindimail.la
iauth_la_LDFLAGS = -L.libs -module -avoid-version -shared -no-undefined -rdynamic -nostartfiles
iauthdir=@libdir@

SYSCONFDIR = @sysconfdir@
sysconf_DATA = indimail.settings controlfiles.i headerlist osh.table cronlist.i perm_list.indimail \
			   indimail.mrtg.cfg indimail-release

pkgconfigdir=@pkgconfigdir@
pkgconfig_DATA = libindimail.pc

man_MANS=adminclient.8 chowkidar.8 clearopensmtp.8 crc.1 crcdiff.1 \
		 crypt.1 dbinfo.8 hostcntrl.8 hostsync.8 iecho.1 indimail.7 \
		 IndiMail.7 indisrvr.8 initsvc.1 inlookup.8 InLookup.8 inquerytest.8 \
		 install_tables.8 ipchange.8 ismaildup.1 login_tasks.3 \
		 mgmtpass.8 osh.8 postdel.8 printdir.1 proxyimap.8 proxypop3.8 \
		 resetquota.8 sslerator.1 updaterules.8 vacation.8 \
		 vaddaliasdomain.1 vadddomain.1 vadduser.1 valias.1 vatrn.1 \
		 vbulletin.1 vcalias.8 vcaliasrev.8 vcfilter.1 vchkpass.8 vdeldomain.1 \
		 vdelivermail.8 vdeloldusers.8 vdeluser.1 vdominfo.1 vfilter.8 \
		 vfstab.8 vgroup.1 vhostid.8 vipmap.1 vlimit.1 vmoddomain.1 vmoduser.1 \
		 vmoveuser.1 vpasswd.1 vpriv.8 vproxy.8 vrenamedomain.1 vrenameuser.1 \
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
				  delusercntrl.h deluser.h dir_control.h Dirname.h \
				  disable_mysql_escape.h evaluate.h fappend.h FifoCreate.h filewrt.h \
				  findhost.h findmdahost.h fstabChangeCounters.h getactualpath.h \
				  getAddressBook.h get_assign.h getFreeFS.h getindimail.h \
				  get_indimailuidgid.h getlastauth.h get_local_hostid.h get_local_ip.h \
				  get_localtime.h get_message_size.h get_Mplexdir.h getpeer.h GetPrefix.h \
				  get_real_domain.h GetSMTProute.h getuidgid.h \
				  hostcntrl_select.h host_in_locals.h iadddomain.h iadduser.h iclose.h \
				  in_bsearch.h indimail_compat.h indimail.h inquery.h iopen.h ipasswd.h \
				  ip_map.h is_alias_domain.h is_already_running.h is_distributed_domain.h \
				  islocalif.h ismaildup.h isnum.h is_user_present.h isvalid_domain.h \
				  isvirtualdomain.h layout.h LoadBMF.h LoadDbInfo.h lockfile.h Login_Tasks.h \
				  load_mysql.h lowerit.h maildir_to_domain.h maildir_to_email.h MailQuotaWarn.h \
				  MakeArgs.h makeseekable.h make_user_dir.h mdaMySQLConnect.h \
				  mgmtpassfuncs.h monkey.h MoveFile.h munch_domain.h mysql_stack.h \
				  next_big_dir.h no_of_days.h open_big_dir.h open_master.h open_smtp_relay.h \
				  parseAddress.h parse_email.h parse_quota.h passwd_policy.h pathToFilesystem.h \
				  pipe_exec.h post_handle.h print_control.h ProcessInFifo.h proxylogin.h \
				  purge_files.h pwcomp.h PwdInLookup.h qmail_remote.h recalc_quota.h \
				  RelayInLookup.h relay_select.h RemoteBulkMail.h remove_line.h remove_quotes.h \
				  renameuser.h replacestr.h r_mkdir.h runcmmd.h SendWelcomeMail.h \
				  set_mysql_options.h setuserid.h setuserquota.h skip_relay.h \
				  skip_system_files.h sockwrite.h smtp_port.h spam.h sql_active.h sql_adddomain.h \
				  sql_adduser.h sql_delaliasdomain.h sql_deldomain.h sql_deluser.h sql_getall.h \
				  sql_getflags.h sql_gethostid.h sql_getip.h sql_getpw.h sql_get_realdomain.h \
				  sql_init.h sql_insertaliasdomain.h sqlOpen_user.h sql_passwd.h sql_renamedomain.h \
				  SqlServer.h sql_setpw.h sql_setquota.h sql_updateflag.h storeHeader.h \
				  strToPw.h tcpbind.h tcpopen.h tls.h trashpurge.h udpopen.h update_file.h \
				  update_local_hostid.h update_newu.h update_quota.h update_rules.h \
				  updusercntrl.h userinfo.h UserInLookup.h user_over_quota.h valiasCount.h \
				  valias_delete_domain.h valias_delete.h valiasinfo.h valias_insert.h \
				  valias_select.h valias_update.h variables.h vdelfiles.h vfilter_delete.h \
				  vfilter_display.h vfilter_filterNo.h vfilter_header.h vfilter_insert.h \
				  vfilter_select.h vfilter_update.h vfstab.h vget_lastauth.h vgetpasswd.h \
				  vhostid_delete.h vhostid_insert.h vhostid_select.h vhostid_update.h VlimitInLookup.h \
				  vlimits.h vmake_maildir.h vpriv.h vquota_select.h vset_lastauth.h vset_lastdeliver.h \
				  vsmtp_delete_domain.h vsmtp_delete.h vsmtp_insert.h vsmtp_select.h vsmtp_update.h \
				  vupdate_rules.h wildmat.h indimail_config.h
includedir=@prefix@/include/indimail

setuid_list = bin/printdir bin/vaddaliasdomain bin/vadddomain bin/vadduser bin/vbulletin \
	bin/vcfilter bin/vdeldomain bin/vdeluser bin/vmoduser bin/vmoveuser bin/vrenamedomain \
	bin/vrenameuser bin/vsetuserquota bin/vdominfo bin/vuserinfo sbin/vreorg \
	libexec/sq_vacation

doc_list= ChangeLog-indimail FAQ.pdf INSTALL-indimail.md \
	README-indimail.md INSTALL-RPM.md Quick-INSTALL.md \
	INSTALL-MYSQL.md README-CLUSTER.md RELEASE-Notes-indimail.md \
	README.ldap indimail_arch.png indimail_queue.png \
	README.vlimits COPYING-indimail

COMMONSOURCES = addaliasdomain.c add_atrn_map.c add_control.c add_domain_assign.c add_ip_map.c \
				add_user_assign.c addusercntrl.c add_vacation.c atrn_access.c autoturn_dir.c \
				backfill.c bulk_mail.c check_group.c Check_Login.c check_quota.c close_big_dir.c \
				cntrl_clearaddflag.c cntrl_cleardelflag.c common.c compile_morercpthosts.c \
				CopyEmailFile.c copyPwdStruct.c count_dir.c count_rcpthosts.c count_table.c \
				CreateDomainDirs.c create_table.c dbinfoAdd.c dbinfoDel.c dbinfoSelect.c \
				dbinfoUpdate.c dbload.c dec_dir_control.c del_atrn_map.c del_control.c \
				del_domain_assign.c deldomain.c del_ip_map.c del_user_assign.c compat.c \
				deluser.c delusercntrl.c Dirname.c evaluate.c fappend.c \
				FifoCreate.c filewrt.c findhost.c findmdahost.c fstabChangeCounters.c \
				getactualpath.c get_assign.c getDbLock.c \
				get_indimailuidgid.c get_local_hostid.c get_local_ip.c get_localtime.c \
				get_message_size.c get_Mplexdir.c getpeer.c GetPrefix.c get_real_domain.c \
				GetSMTProute.c hostcntrl_select.c \
				host_in_locals.c iadddomain.c iadduser.c iclose.c inc_dir.c \
				inc_dir_control.c init_dir_control.c inquery.c iopen.c \
				ipasswd.c is_alias_domain.c is_already_running.c is_distributed_domain.c \
				islocalif.c ismaildup.c isnum.c is_user_present.c isvirtualdomain.c \
				layout.c limits.c LoadDbInfo.c lockfile.c Login_Tasks.c lowerit.c \
				maildir_to_domain.c maildir_to_email.c MailQuotaWarn.c MakeArgs.c \
				makeseekable.c make_user_dir.c \
				mdaMySQLConnect.c mgmtpassfuncs.c MoveFile.c munch_domain.c \
				mysql_query.c mysql_stack.c next_big_dir.c no_of_days.c open_big_dir.c \
				open_master.c open_smtp_relay.c parse_email.c parse_quota.c passwd_policy.c \
				pathToFilesystem.c pipe_exec.c post_handle.c print_control.c \
				pwcomp.c recalc_quota.c relay_select.c RemoteBulkMail.c remove_line.c \
				remove_quotes.c renameuser.c replacestr.c r_mkdir.c runcmmd.c \
				SendWelcomeMail.c set_mysql_options.c setuserid.c setuserquota.c \
				show_atrn_map.c show_ip_map.c skip_relay.c \
				skip_system_files.c smtp_port.c sql_active.c sql_adddomain.c sql_adduser.c \
				sql_delaliasdomain.c sql_deldomain.c sql_deluser.c sql_getall.c \
				sql_getflags.c sql_gethostid.c sql_getip.c sql_getpw.c \
				sql_get_realdomain.c sql_init.c sql_insertaliasdomain.c sqlOpen_user.c \
				sql_passwd.c sql_renamedomain.c SqlServer.c sql_setpw.c sql_setquota.c \
				sql_updateflag.c strToPw.c tcpbind.c tcpopen.c update_file.c \
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
	$(LIBTOOL) --mode=link $(COMPILE) $(MODFLAGS) -o $@ iauth.lo $(iauth_la_LDFLAGS) \
	$(iauth_la_LIBADD) -rpath @libdir@

iauth.lo: iauth.c
	$(LIBTOOL) --mode=compile gcc $(DEFS) -c $^

all: config.h indimail_config.h
all-local: libindimail.la iauth.la

indimailbin_PROGRAMS = vadddomain vdeldomain vadduser vdeluser vuserinfo \
					   vaddaliasdomain vrenamedomain dbinfo vdominfo \
					   vmoduser printdir ismaildup valias vcalias hostcntrl \
					   vhostid vpasswd vcfilter vsmtp vsetuserquota \
					   vacation vatrn vmoveuser vrenameuser vipmap vlimit \
					   vpriv crc crcdiff iecho crypt vgroup proxyimap proxypop3 \
					   sslerator vproxy vbulletin

indimailsbin_PROGRAMS = inlookup vdelivermail clearopensmtp updaterules \
						vfilter vmoddomain inquerytest vreorg vtable \
						install_tables hostsync vserverinfo initsvc \
						mgmtpass vchkpass vsetpass resetquota \
						ipchange adminclient indisrvr vdeloldusers vfstab \
						osh chowkidar vpurge

indimaillibexec_PROGRAMS = sq_vacation qmailmrtg7 updatefile
indimaillibexec_SCRIPTS = mailzipper overquota.sh controlsync cputime \
						  myslave bogofilter-qfe iupgrade.sh ilocal_upgrade.sh

indimailauthlib_PROGRAMS = authindi

indimailauthlib_SCRIPTS = authgeneric

indimailbin_SCRIPTS = vcaliasrev

noinst_SCRIPTS = vadddomain_handle vadduser_handle vdeldomain_handle \
	vaddaliasdomain_handle vrenamedomain_handle vpasswd_handle vmoduser_handle \
	vdeluser_handle vrenameuser_handle

noinst_DATA = indimail-permissions.easy indimail-permissions.paranoid \
			  indimail-permissions.secure doc/INSTALL-indimail.md doc/INSTALL-RPM.md \
			  doc/INSTALL-MYSQL.md doc/README-CLUSTER.md doc/RELEASE-Notes-indimail.md \
			  doc/Quick-INSTALL.md doc/README-indimail.md indimail.spec indimail-rpmlintrc

post_handle_list = vadddomain vadduser vdeldomain vaddaliasdomain vrenamedomain vmoduser vpasswd \
				   vdeluser_handle vrenameuser_handle

MOSTLYCLEANFILES=vcaliasrev myslave controlsync bogofilter-qfe \
cronlist.i crypt vdeluser_handle vrenameuser_handle \
cscope.* *.dSYM socket.lib mailzipper \
perm_list.indimail dbinfo.8 inlookup.8 vadddomain.1 vdeldomain.1 vadduser.1 \
vdeluser.1 vdelivermail.8 vrenamedomain.1 vrenameuser.1 vpasswd.1 vmoveuser.1 \
vcfilter.1 indimail.7 indimail.mrtg.cfg overquota.sh \
vadddomain_handle vadduser_handle vpasswd_handle vdeldomain_handle vaddaliasdomain_handle \
vrenamedomain_handle login_tasks.3 authgeneric usr.sbin.mysqld usr.sbin.clamd \
vmoduser_handle vmoduser.1 usr.bin.freshclam indimail.mod indimail.pp \
indimail_config.h \
doc/INSTALL-indimail.md doc/Quick-INSTALL.md doc/INSTALL-RPM.md \
doc/INSTALL-MYSQL.md doc/RELEASE-Notes-indimail.md doc/README-CLUSTER.md \
doc/README-indimail.md

#DISTCLEANFILES=.gitignore

vadddomain_SOURCES = vadddomain.c
vadddomain_LDADD = @auth_libs@ isvalid_domain.lo
vadddomain_DEPENDENCIES = isvalid_domain.lo $(top_builddir)/libindimail.la

vaddaliasdomain_SOURCES = vaddaliasdomain.c
vaddaliasdomain_LDADD = @auth_libs@
vaddaliasdomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vdeldomain_SOURCES = vdeldomain.c
vdeldomain_LDADD = @auth_libs@
vdeldomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vrenamedomain_SOURCES = vrenamedomain.c
vrenamedomain_LDADD = @auth_libs@
vrenamedomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vmoddomain_SOURCES=vmoddomain.c
vmoddomain_LDADD= @auth_libs@
vmoddomain_DEPENDENCIES = $(top_builddir)/libindimail.la

vadduser_SOURCES = vadduser.c
vadduser_LDADD = @auth_libs@
vadduser_DEPENDENCIES = $(top_builddir)/libindimail.la

vgroup_SOURCES = vgroup.c
vgroup_LDADD = @auth_libs@
vgroup_DEPENDENCIES = $(top_builddir)/libindimail.la

vpasswd_SOURCES = vpasswd.c
vpasswd_LDADD = @auth_libs@
vpasswd_DEPENDENCIES = $(top_builddir)/libindimail.la

vdeluser_SOURCES = vdeluser.c
vdeluser_LDADD = @auth_libs@
vdeluser_DEPENDENCIES = $(top_builddir)/libindimail.la

vmoduser_SOURCES = vmoduser.c
vmoduser_LDADD = @auth_libs@
vmoduser_DEPENDENCIES = $(top_builddir)/libindimail.la

vuserinfo_SOURCES = vuserinfo.c
vuserinfo_LDADD = @auth_libs@
vuserinfo_DEPENDENCIES = $(top_builddir)/libindimail.la

dbinfo_SOURCES = dbinfo.c
dbinfo_LDADD = @auth_libs@
dbinfo.lo = LoadDbInfo.h iclose.h variables.h common.h \
	get_indimailuidgid.h writemcdinfo.h LoadDbInfo.h runcmmd.h
dbinfo_DEPENDENCIES = $(top_builddir)/libindimail.la

vdominfo_SOURCES = vdominfo.c
vdominfo_LDADD = @auth_libs@
vdominfo.lo = get_assign.h common.h setuserid.h check_group.h iclose.h \
	variables.h get_indimailuidgid.h get_real_domain.h print_control.h \
	common.h isvirtualdomain.h get_local_hostid.h \
	get_local_ip.h
vdominfo_DEPENDENCIES = $(top_builddir)/libindimail.la

hostcntrl_SOURCES = hostcntrl.c
hostcntrl_LDADD = @auth_libs@
hostcntrl_DEPENDENCIES = $(top_builddir)/libindimail.la

vhostid_SOURCES = vhostid.c
vhostid_LDADD = @auth_libs@
vhostid_DEPENDENCIES = $(top_builddir)/libindimail.la

vsmtp_SOURCES = vsmtp.c
vsmtp_LDADD = @auth_libs@
vsmtp_DEPENDENCIES = $(top_builddir)/libindimail.la

inlookup_SOURCES = inlookup.c
inlookup_LDADD = @auth_libs@ ProcessInFifo.lo AliasInLookup.lo PwdInLookup.lo \
	RelayInLookup.lo UserInLookup.lo VlimitInLookup.lo libindimail.la -ldl
inlookup_DEPENDENCIES = ProcessInFifo.lo AliasInLookup.lo PwdInLookup.lo RelayInLookup.lo \
	UserInLookup.lo VlimitInLookup.lo $(top_builddir)/libindimail.la

printdir_SOURCES = printdir.c
printdir_LDADD = @auth_libs@
printdir_DEPENDENCIES = $(top_builddir)/libindimail.la

vdelivermail_SOURCES = vdelivermail.c
vdelivermail_LDADD = @auth_libs@ deliver_mail.lo qmail_remote.lo \
					 udpopen.lo @LIB_SSL@
vdelivermail_DEPENDENCIES = deliver_mail.lo qmail_remote.lo udpopen.lo $(top_builddir)/libindimail.la

vfilter_SOURCES=vfilter.c
vfilter_LDADD= storeHeader.lo parseAddress.lo deliver_mail.lo \
			   addressToken.lo getAddressBook.lo udpopen.lo \
			   $(top_builddir)/eps-1.2/libeps.la @auth_libs@
vfilter_DEPENDENCIES=storeHeader.lo parseAddress.lo deliver_mail.lo \
					 addressToken.lo getAddressBook.lo udpopen.lo $(top_builddir)/libindimail.la

vcfilter_SOURCES=vcfilter.c
vcfilter_LDADD= @auth_libs@
vcfilter_DEPENDENCIES = $(top_builddir)/libindimail.la

ismaildup_SOURCES = ismaildup.c
ismaildup_LDADD = @auth_libs@
ismaildup_CFLAGS = -DMAIN
ismaildup_DEPENDENCIES = $(top_builddir)/libindimail.la

valias_SOURCES = valias.c
valias_LDADD = @auth_libs@
valias_DEPENDENCIES = $(top_builddir)/libindimail.la

vcalias_SOURCES = vcalias.c
vcalias_LDADD = @auth_libs@
vcalias_DEPENDENCIES = $(top_builddir)/libindimail.la

clearopensmtp_SOURCES = clearopensmtp.c
clearopensmtp_LDADD = @auth_libs@ clear_open_smtp.lo
clearopensmtp_DEPENDENCIES = clear_open_smtp.lo $(top_builddir)/libindimail.la

updaterules_SOURCES = updaterules.c
updaterules_LDADD = @auth_libs@
updaterules_DEPENDENCIES = $(top_builddir)/libindimail.la

inquerytest_SOURCES = inquerytest.c
inquerytest_LDADD = @auth_libs@ ProcessInFifo.lo  AliasInLookup.lo PwdInLookup.lo \
	RelayInLookup.lo UserInLookup.lo VlimitInLookup.lo -ldl
inquerytest_DEPENDENCIES = ProcessInFifo.lo  AliasInLookup.lo PwdInLookup.lo RelayInLookup.lo \
	UserInLookup.lo VlimitInLookup.lo $(top_builddir)/libindimail.la

vreorg_SOURCES = vreorg.c
vreorg_LDADD = @auth_libs@
vreorg_DEPENDENCIES = $(top_builddir)/libindimail.la

install_tables_SOURCES = install_tables.c
install_tables_LDADD = @auth_libs@
install_tables_DEPENDENCIES = $(top_builddir)/libindimail.la

vtable_SOURCES = vtable.c
vtable_LDADD = @auth_libs@
vtable_DEPENDENCIES = $(top_builddir)/libindimail.la

hostsync_SOURCES = hostsync.c
hostsync_LDADD = @auth_libs@
hostsync_DEPENDENCIES = $(top_builddir)/libindimail.la

vserverinfo_SOURCES = vserverinfo.c
vserverinfo_LDADD = @auth_libs@
vserverinfo_DEPENDENCIES = $(top_builddir)/libindimail.la

initsvc_SOURCES = initsvc.c
initsvc_LDADD = common.lo fappend.lo $(LIB_QMAIL)
initsvc_DEPENDENCIES = common.lo fappend.lo

mgmtpass_SOURCES = mgmtpass.c
mgmtpass_LDADD = @auth_libs@
mgmtpass_DEPENDENCIES = $(top_builddir)/libindimail.la

authindi_SOURCES = authindi.c
authindi_LDADD = @auth_libs@
authindi_DEPENDENCIES = $(top_builddir)/libindimail.la

vchkpass_SOURCES = vchkpass.c
vchkpass_LDADD = @auth_libs@
vchkpass_DEPENDENCIES = $(top_builddir)/libindimail.la

vsetpass_SOURCES = vsetpass.c
vsetpass_LDADD = @auth_libs@
vsetpass_DEPENDENCIES = $(top_builddir)/libindimail.la

vsetuserquota_SOURCES = vsetuserquota.c
vsetuserquota_LDADD = @auth_libs@
vsetuserquota_DEPENDENCIES = $(top_builddir)/libindimail.la

vacation_SOURCES = vacation.c
vacation_LDADD = @auth_libs@
vacation_DEPENDENCIES = $(top_builddir)/libindimail.la

sq_vacation_SOURCES = sq_vacation.c
sq_vacation_LDADD = @auth_libs@
sq_vacation_DEPENDENCIES = $(top_builddir)/libindimail.la

vatrn_SOURCES = vatrn.c
vatrn_LDADD = @auth_libs@
vatrn_DEPENDENCIES = $(top_builddir)/libindimail.la

vmoveuser_SOURCES = vmoveuser.c
vmoveuser_LDADD = @auth_libs@
vmoveuser_DEPENDENCIES = $(top_builddir)/libindimail.la

vrenameuser_SOURCES = vrenameuser.c
vrenameuser_LDADD = @auth_libs@
vrenameuser_DEPENDENCIES = $(top_builddir)/libindimail.la

vipmap_SOURCES = vipmap.c
vipmap_LDADD = @auth_libs@
vipmap_DEPENDENCIES = $(top_builddir)/libindimail.la

vlimit_SOURCES = vlimit.c
vlimit_LDADD = @auth_libs@
vlimit_DEPENDENCIES = $(top_builddir)/libindimail.la

vpriv_SOURCES = vpriv.c
vpriv_LDADD = @auth_libs@
vpriv_DEPENDENCIES = $(top_builddir)/libindimail.la

crc_SOURCES=crc.c
crc_LDADD = @auth_libs@
crc_DEPENDENCIES = $(top_builddir)/libindimail.la

crcdiff_SOURCES=crcdiff.c
crcdiff_LDADD=

iecho_SOURCES=iecho.c
iecho_LDADD=

crypt_SOURCES = crypt.c
crypt_LDADD = @auth_libs@ @LIB_CRYPT@
crypt_DEPENDENCIES = $(top_builddir)/libindimail.la

ipchange_SOURCES = ipchange.c
ipchange_LDADD = @auth_libs@
ipchange_DEPENDENCIES = $(top_builddir)/libindimail.la

resetquota_SOURCES = resetquota.c
resetquota_LDADD = @auth_libs@
resetquota_DEPENDENCIES = $(top_builddir)/libindimail.la

sslerator_SOURCES = sslerator.c
sslerator_LDADD = @auth_libs@ sockwrite.lo  @LIB_SSL@$(shell cat socket.lib 2>/dev/null)
sslerator_DEPENDENCIES = sockwrite.lo $(top_builddir)/libindimail.la

proxyimap_SOURCES = proxyimap.c
proxyimap_LDADD = @auth_libs@ proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo \
				  monkey.lo sockwrite.lo
proxyimap_DEPENDENCIES = proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo \
						 monkey.lo sockwrite.lo $(top_builddir)/libindimail.la

proxypop3_SOURCES = proxypop3.c
proxypop3_LDADD = @auth_libs@ proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo \
				  monkey.lo sockwrite.lo
proxypop3_DEPENDENCIES = proxylogin.lo tls.lo auth_admin.lo adminCmmd.lo \
						 monkey.lo sockwrite.lo $(top_builddir)/libindimail.la

vproxy_SOURCES = vproxy.c
vproxy_LDADD = monkey.lo sockwrite.lo @auth_libs@
vproxy_DEPENDENCIES = monkey.lo sockwrite.lo $(top_builddir)/libindimail.la

vbulletin_SOURCES = vbulletin.c
vbulletin_LDADD = bulletin.lo @auth_libs@
vbulletin_DEPENDENCIES = bulletin.lo $(top_builddir)/libindimail.la

adminclient_SOURCES = adminclient.c
adminclient_LDADD = @auth_libs@ tls.lo auth_admin.lo adminCmmd.lo @LIB_SSL@
adminclient_DEPENDENCIES = tls.lo auth_admin.lo adminCmmd.lo $(top_builddir)/libindimail.la

indisrvr_SOURCES = indisrvr.c
indisrvr_LDADD = @auth_libs@ sockwrite.lo checkPerm.lo @LIB_SSL@ $(shell cat socket.lib 2>/dev/null)
indisrvr_DEPENDENCIES = sockwrite.lo checkPerm.lo $(top_builddir)/libindimail.la

vdeloldusers_SOURCES = vdeloldusers.c
vdeloldusers_LDADD = @auth_libs@ delunreadmails.lo getlastauth.lo \
					 trashpurge.lo getindimail.lo
vdeloldusers_DEPENDENCIES = delunreadmails.lo getlastauth.lo \
							trashpurge.lo getindimail.lo $(top_builddir)/libindimail.la

vfstab_SOURCES = vfstab.c
vfstab_LDADD = @auth_libs@ vfstab_select.lo vfstab_insert.lo vfstab_update.lo \
			   vfstab_delete.lo vfstabNew.lo vfstab_status.lo getFreeFS.lo
vfstab_DEPENDENCIES = vfstab_select.lo vfstab_insert.lo vfstab_update.lo \
					  vfstab_delete.lo vfstabNew.lo vfstab_status.lo getFreeFS.lo \
					  $(top_builddir)/libindimail.la

vpurge_SOURCES = vpurge.c
vpurge_LDADD = @auth_libs@ purge_files.lo
vpurge_DEPENDENCIES = purge_files.lo $(top_builddir)/libindimail.la

chowkidar_SOURCES = chowkidar.c
chowkidar_LDADD = @auth_libs@ LoadBMF.lo spam.lo
chowkidar_DEPENDENCIES = LoadBMF.lo spam.lo $(top_builddir)/libindimail.la

osh_SOURCES = osh.c
osh_LDADD = @LIB_CURSES@ @LIB_TERMCAP@ exp.lo handlers.lo @LIB_QMAIL@
osh_DEPENDENCIES = exp.lo handlers.lo

updatefile_SOURCES=updatefile.c
updatefile_LDADD= @auth_libs@
updatefile_DEPENDENCIES = $(top_builddir)/libindimail.la

addaliasdomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
add_atrn_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
add_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
add_domain_assign.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
add_ip_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
add_user_assign.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
addusercntrl.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
add_vacation.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
atrn_access.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
autoturn_dir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
backfill.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
bulk_mail.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
check_group.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
Check_Login.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
check_quota.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
close_big_dir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
cntrl_clearaddflag.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
cntrl_cleardelflag.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
common.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
compile_morercpthosts.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
CopyEmailFile.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
copyPwdStruct.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
count_dir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
count_rcpthosts.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
count_table.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
CreateDomainDirs.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
create_table.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
dbinfoAdd.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
dbinfoDel.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
dbinfoSelect.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
dbinfoUpdate.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
dbload.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
dec_dir_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
del_atrn_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
del_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
del_domain_assign.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
deldomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
del_ip_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
del_user_assign.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
deluser.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
delusercntrl.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
Dirname.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
evaluate.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
fappend.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
FifoCreate.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
filewrt.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
findhost.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
findmdahost.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
fstabChangeCounters.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
getactualpath.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_assign.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
getDbLock.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_indimailuidgid.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_local_hostid.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_local_ip.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_localtime.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_message_size.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_Mplexdir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
getpeer.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
GetPrefix.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_real_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
GetSMTProute.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
get_smtp_service_port.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
hostcntrl_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
host_in_locals.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
iadddomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
iadduser.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
iclose.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
inc_dir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
inc_dir_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
init_dir_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h dir_control.h
inquery.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
iopen.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
ipasswd.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
is_alias_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
is_already_running.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
is_distributed_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
islocalif.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
ismaildup.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
isnum.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
is_user_present.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
isvirtualdomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
layout.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
limits.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h vlimits.h
LoadDbInfo.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
lockfile.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
Login_Tasks.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
lowerit.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
maildir_to_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
maildir_to_email.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
MailQuotaWarn.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
MakeArgs.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
makeseekable.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
make_user_dir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
mdaMySQLConnect.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
mgmtpassfuncs.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
MoveFile.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
munch_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
mysql_query.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
mysql_stack.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
next_big_dir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
no_of_days.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
open_big_dir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
open_master.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
parse_email.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
parse_quota.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
passwd_policy.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
pathToFilesystem.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
pipe_exec.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
post_handle.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
print_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
pwcomp.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
recalc_quota.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
relay_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
RemoteBulkMail.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
remove_line.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
remove_quotes.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
renameuser.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
replacestr.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
r_mkdir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
runcmmd.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
SendWelcomeMail.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
set_mysql_options.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
setuserid.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
setuserquota.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
show_atrn_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
show_ip_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
skip_relay.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
skip_system_files.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_active.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_adddomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_adduser.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_delaliasdomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_deldomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h open_master.h
sql_deluser.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_getall.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_getflags.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_gethostid.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_getip.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_getpw.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_get_realdomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_init.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_insertaliasdomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sqlOpen_user.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_passwd.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_renamedomain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
SqlServer.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_setpw.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_setquota.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
sql_updateflag.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
strToPw.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
tcpbind.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
tcpopen.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
update_file.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
update_local_hostid.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
update_newu.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
update_quota.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
update_rules.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
upd_atrn_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
upd_ip_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
updusercntrl.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
userinfo.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
user_over_quota.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
valiasCount.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
valias_delete.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
valias_delete_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
valiasinfo.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
valias_insert.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
valias_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
valias_update.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
variables.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vcreate_dir_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vdel_dir_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vdelfiles.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vfilter_delete.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vfilter_display.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vfilter_filterNo.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vfilter_header.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vfilter_insert.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vfilter_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vfilter_update.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vget_ip_map.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vget_lastauth.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vgetpasswd.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vhostid_delete.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vhostid_insert.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vhostid_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vhostid_update.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vmake_maildir.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vopen_smtp_relay.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vpriv_delete.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vpriv_insert.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vpriv_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vpriv_update.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vquota_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vread_dir_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vset_default_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vset_lastauth.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vset_lastdeliver.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vsmtp_delete.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vsmtp_delete_domain.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vsmtp_insert.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vsmtp_select.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vsmtp_update.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vupdate_rules.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
vwrite_dir_control.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h
wildmat.lo: config.h variables.h valias_insert.h isvalid_domain.h vgetpasswd.h check_group.h get_assign.h r_mkdir.h iclose.h post_handle.h is_user_present.h get_local_ip.h get_local_hostid.h get_local_ip.h is_distributed_domain.h LoadDbInfo.h vgetpasswd.h SqlServer.h iadddomain.h deldomain.h iadduser.h setuserquota.h vhostid_select.h vhostid_insert.h dbinfoAdd.h get_indimailuidgid.h

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
	-e 's}@auth_inc\@}$(auth_inc)}g' \
	-e 's}@auth_libs\@}$(auth_libs)}g' \
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
	-e 's|@release\@|$(release)|g' \
	-e 's}@pkgconfigdir\@}$(pkgconfigdir)}g' \
	-e 's}@VERSION\@}$(VERSION)}g' \
	-e 's}@DESTDIR\@}$(DESTDIR)}g' \
	-e 's}@HOST\@}$(host)}g' \
	-e 's}@DATE\@}$(DATE)}g' \
	-e 's}@prefix\@}$(prefix)}g'
editman = sed \
	-e 's}INDIMAILDIR}$(indimaildir)}g' \
	-e 's}@sysconfdir\@}$(sysconfdir)}g' \
	-e 's}@qmaildir\@}$(qmaildir)}g' \
	-e 's}CONTROLDIR}$(controldir)}g' \
	-e 's}@servicedir\@}$(servicedir)}g' \
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
perm_list.indimail: perm_list.in
	$(edit) perm_list.in > $@
usr.sbin.mysqld: usr.sbin.mysqld.in
	$(edit) $@.in > $@
usr.sbin.clamd: usr.sbin.clamd.in
	$(edit) $@.in > $@
usr.bin.freshclam: usr.bin.freshclam.in
	$(edit) $@.in > $@
indimail_config.h: config.h
	egrep -v "#define PACKAGE|#define VERSION" config.h > indimail_config.h

debian/Makefile: debian/Makefile.in configure.ac
	$(edit) $@.in > $@;

doc/INSTALL-indimail.md: INSTALL.in configure.ac
	$(edit) INSTALL.in > $@
doc/Quick-INSTALL.md: Quick-INSTALL.in configure.ac
	$(edit) Quick-INSTALL.in > $@
doc/INSTALL-RPM.md: INSTALL-RPM.in configure.ac
	$(edit) INSTALL-RPM.in > $@
doc/INSTALL-MYSQL.md: INSTALL-MYSQL.in configure.ac
	$(edit) INSTALL-MYSQL.in > $@
doc/RELEASE-Notes-indimail.md: RELEASE-Notes.in configure.ac
	(cat RELEASE-Notes.in; ./catChangeLog) | $(edit) > $@
doc/README-CLUSTER.md: README-CLUSTER.in configure.ac
	$(edit) README-CLUSTER.in > $@
doc/README-indimail.md: README.in configure.ac
	$(edit) README.in > $@

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
indimail.spec: indimail.spec.in catChangeLog doc/ChangeLog-indimail conf-version conf-release
	(cat $@.in;./catChangeLog) | $(edit) > $@
indimail-rpmlintrc: indimail-rpmlintrc.in
	$(edit) indimail-rpmlintrc.in > $@
login_tasks.3: login_tasks.9
	$(editman) login_tasks.9 > $@
dbinfo.8: dbinfo.9
	cat $^ | sed s}INDIMAIL}$(indimaildir)}g > $@
inlookup.8: inlookup.9
	$(editman) inlookup.9 > $@
vcfilter.1: vcfilter.9
	cat $^ | sed s}SYSCONFDIR}$(sysconfdir)}g > $@
vadddomain.1:vadddomain.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g \
		| sed s}INDIMAILDIR}$(indimaildir)}g > $@
vdeldomain.1:vdeldomain.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vadduser.1:vadduser.9
	cat $^ | sed s}INDIMAIL}$(indimaildir)}g \
		| sed s}LIBEXECDIR}$(libexecdir)}g > $@
vdeluser.1:vdeluser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vrenamedomain.1:vrenamedomain.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vrenameuser.1:vrenameuser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vpasswd.1:vpasswd.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vmoduser.1:vmoduser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vmoveuser.1:vmoveuser.9
	cat $^ | sed s}LIBEXECDIR}$(libexecdir)}g > $@
vdelivermail.8:vdelivermail.9
	cat $^ | sed s}INDIMAIL}$(indimaildir)}g \
		| sed s}LIBEXECDIR}$(libexecdir)}g > $@
indimail.7:indimail.9
	$(editman) indimail.9 > $@

crc.$(OBJEXT): crc.c
	$(COMPILE) $(DEFS) -DMAIN -c crc.c
crc$(EXEEXT): $(crc_DEPENDENCIES) $(crc_OBJECTS)
	$(LINK) $@.$(OBJEXT) $(crc_LDADD)

iecho.$(OBJEXT): iecho.c
	$(COMPILE) $(DEFS) -c iecho.c
iecho$(EXEEXT): $(iecho_DEPENDENCIES) $(iecho_OBJECTS)
	$(LINK) $@.$(OBJEXT) $(iecho_LDADD)

install-data-local: initsvc libindimail.la indimail-rpmlintrc \
indimail.spec iauth.la indimail.te indimail.fc usr.sbin.mysqld usr.sbin.clamd usr.bin.freshclam
	if [ " $(DESTDIR)" = " " ] ; then \
		if test `@idcommand@` != "0" ; then \
			echo "you are not root"; \
			exit 1; \
		fi ; \
		echo "Shutting down IndiMail"; \
		./initsvc -off || exit 0; \
	fi;

# This is the last step in indimail installation. Post this installation will start in qmail
# Hence we still cannot start svscan.
install-data-hook:
	$(mkinstalldirs) $(DESTDIR)@prefix@/lib/indimail/modules || exit 1
	$(INSTALL_DATA) $(DESTDIR)@libdir@/iauth.so  $(DESTDIR)@prefix@/lib/indimail/modules || exit 1
	/bin/rm -f $(DESTDIR)@libdir@/iauth.* || exit 1
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
		$(INSTALL_DATA) doc/$$f $(DESTDIR)@datarootdir@/doc/$$f || exit 1 ;\
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
	if test " $(DISTRIB_ID)" = " Ubuntu" -o " $(DISTRIB_ID)" = " Debian" ; then \
		$(mkinstalldirs) $(DESTDIR)@sysconfdir@/apparmor.d/local || exit 1; \
		$(INSTALL_DATA) usr.sbin.mysqld $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.sbin.mysqld  || exit 1; \
		$(INSTALL_DATA) usr.sbin.clamd $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.sbin.clamd  || exit 1; \
		$(INSTALL_DATA) usr.bin.freshclam $(DESTDIR)@sysconfdir@/apparmor.d/local/usr.bin.freshclam  || exit 1; \
	fi
	if test " $(DISTRIB_ID)" != " Ubuntu" -a " $(DISTRIB_ID)" != " Debian" ; then \
		$(INSTALL_DATA) indimail.te $(DESTDIR)@sysconfdir@/indimail.te  || exit 1; \
		$(INSTALL_DATA) indimail.fc $(DESTDIR)@sysconfdir@/indimail.fc  || exit 1; \
	fi

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AUTOMAKE_OPTIONS = foreign no-dependencies
