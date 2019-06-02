/*
 * $Log: set_mysql_options.c,v $
 * Revision 1.3  2019-05-29 09:24:33+05:30  Cprogrammer
 * use mysql_options() when using dlopen of libmysqlclient
 *
 * Revision 1.2  2019-04-22 23:15:01+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.1  2019-04-14 21:04:41+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <stralloc.h>
#include <strerr.h>
#include "getEnvConfig.h"

#ifndef	lint
static char     sccsid[] = "$Id: set_mysql_options.c,v 1.3 2019-05-29 09:24:33+05:30 Cprogrammer Exp mbhangui $";
#endif

#define max_mysql_option_err_num 21
static char *mysql_option_err[] = {
	"No Error",
	"MYSQL_INIT_COMMAND",
	"MYSQL_READ_DEFAULT_FILE",
	"MYSQL_READ_DEFAULT_GROUP",
	"MYSQL_OPT_CONNECT_TIMEOUT",
	"MYSQL_OPT_READ_TIMEOUT",
	"MYSQL_OPT_WRITE_TIMEOUT",
	"MYSQL_SET_CLIENT_IP",
	"MYSQL_OPT_RECONNECT",
	"MYSQL_OPT_PROTOCOL",
	"MYSQL_OPT_SSL_CA",
	"MYSQL_OPT_SSL_CAPATH",
	"MYSQL_OPT_SSL_CERT",
	"MYSQL_OPT_SSL_CIPHER",
	"MYSQL_OPT_SSL_CRL",
	"MYSQL_OPT_SSL_CRLPATH",
	"MYSQL_OPT_SSL_ENFORCE",
	"MYSQL_OPT_SSL_VERIFY_SERVER_CERT",
	"MYSQL_OPT_SSL_MODE",
	"MYSQL_OPT_SSL_KEY",
	"MYSQL_OPT_TLS_VERSION",
	0
};

int
int_mysql_options(MYSQL *mysql, enum mysql_option option, const void *arg)
{
#ifdef HAVE_MYSQL_OPTIONSV
	/*- mysql_optionsv() used to dump core dump earlier. */
#ifdef DLOPEN_LIBMYSQLCLIENT
	return (in_mysql_options(mysql, option, arg));
#else
	return (in_mysql_optionsv(mysql, option, arg, 0));
#endif
#else
	return (in_mysql_options(mysql, option, arg));
#endif
}

char *
error_mysql_options_str(unsigned int errnum)
{
	return ((errnum > max_mysql_option_err_num) ? 0 : mysql_option_err[errnum]);
}

int
set_mysql_options(MYSQL *mysql, char *file, char *group, unsigned int *flags)
{
	char           *default_file, *default_group, *c_timeout, 
				   *r_timeout, *w_timeout, *init_cmd,
				   *opt_reconnect, *opt_protocol;
#ifdef MYSQL_SET_CLIENT_IP
	char           *set_client_ip;
#endif
#if defined(HAVE_MYSQL_OPT_SSL_CA) || defined(HAVE_MYSQL_OPT_SSL_CAPATH) \
		|| defined(HAVE_MYSQL_OPT_SSL_CERT) || defined(HAVE_MYSQL_OPT_SSL_CIPHER) \
		|| defined(HAVE_MYSQL_OPT_SSL_CRL) || defined(HAVE_MYSQL_OPT_SSL_CRLPATH) \
		|| defined(HAVE_MYSQL_OPT_SSL_ENFORCE) || defined(HAVE_MYSQL_OPT_SSL_MODE) \
		|| defined(HAVE_MYSQL_OPT_SSL_VERIFY_SERVER_CERT) || defined(HAVE_MYSQL_OPT_SSL_KEY) \
		|| defined(HAVE_MYSQL_OPT_TLS_VERSION)
	char           *ptr; 
#endif
	char            temp[4];
	char            o_reconnect, use_ssl = 0;
#ifdef HAVE_MYSQL_OPT_SSL_VERIFY_SERVER_CERT
	char            tmpv_c;
#endif
	unsigned int    protocol, connect_timeout, read_timeout, write_timeout;
#if defined(LIBMARIADB) || defined(MARIADB_BASE_VERSION)
	static stralloc fpath = {0};
	char           *sysconfdir;
#endif
#if defined(HAVE_MYSQL_OPT_SSL_MODE) || defined(HAVE_MYSQL_OPT_SSL_ENFORCE)
	unsigned int    ssl_mode;
#endif
	int             t;
	char           *cipher;

	use_ssl = (*flags == 1 ? 1 : 0);
	*flags = 0;
	if (env_get("CLIENT_COMPRESS"))
		*flags += CLIENT_COMPRESS;
	if (env_get("CLIENT_INTERACTIVE"))
		*flags += CLIENT_INTERACTIVE;
	getEnvConfigStr(&init_cmd, "MYSQL_INIT_COMMAND", 0);
	if (file) {
#if defined(LIBMARIADB) || defined(MARIADB_BASE_VERSION) /*- full path required in mariadb */
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (!stralloc_copys(&fpath, sysconfdir))
			strerr_die1sys(111, "out of memory: ");
		else
		if (!stralloc_append(&fpath, "/"))
			strerr_die1sys(111, "out of memory: ");
		else
		if (!stralloc_cats(&fpath, file))
			strerr_die1sys(111, "out of memory: ");
		else
		if (!stralloc_0(&fpath))
			strerr_die1sys(111, "out of memory: ");
		getEnvConfigStr(&default_file, "MYSQL_READ_DEFAULT_FILE", fpath.s);
#else
		getEnvConfigStr(&default_file, "MYSQL_READ_DEFAULT_FILE", file);
#endif
	}
	if (file && group)
		getEnvConfigStr(&default_group, "MYSQL_READ_DEFAULT_GROUP", group);
	getEnvConfigStr(&c_timeout, "MYSQL_OPT_CONNECT_TIMEOUT", "120");
	getEnvConfigStr(&r_timeout, "MYSQL_OPT_READ_TIMEOUT", "20");
	getEnvConfigStr(&w_timeout, "MYSQL_OPT_WRITE_TIMEOUT", "20");
#ifdef MYSQL_SET_CLIENT_IP
	getEnvConfigStr(&set_client_ip, "MYSQL_SET_CLIENT_IP", 0);
#endif
	getEnvConfigStr(&opt_reconnect, "MYSQL_OPT_RECONNECT", "0");
	temp[fmt_uint(temp, MYSQL_PROTOCOL_DEFAULT)] = 0;
	getEnvConfigStr(&opt_protocol, "MYSQL_OPT_PROTOCOL", temp);
	scan_uint(opt_protocol, &protocol);
	scan_int(opt_reconnect, &t);
	o_reconnect = t ? 1 : 0;
	scan_uint(c_timeout, &connect_timeout);
	scan_uint(r_timeout, &read_timeout);
	scan_uint(w_timeout, &write_timeout);
	if (init_cmd && int_mysql_options(mysql, MYSQL_INIT_COMMAND, init_cmd))
		return (1);
	if (file && int_mysql_options(mysql, MYSQL_READ_DEFAULT_FILE, default_file))
		return (2);
	if (file && group && int_mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, default_group))
		return (3);
	if (int_mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *) &connect_timeout))
		return (4);
	if (int_mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, (char *) &read_timeout))
		return (5);
	if (int_mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT, (char *) &write_timeout))
		return (6);
#ifdef MYSQL_SET_CLIENT_IP
	if (env_get("MYSQL_SET_CLIENT_IP") && 
			int_mysql_options(mysql, MYSQL_SET_CLIENT_IP, set_client_ip))
		return (7);
#endif
	if (env_get("MYSQL_OPT_RECONNECT") &&
			int_mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *) &o_reconnect))
		return (8);
	/*-
	 * enum mysql_protocol_type 
	 * MYSQL_PROTOCOL_DEFAULT, MYSQL_PROTOCOL_TCP, MYSQL_PROTOCOL_SOCKET,
	 * MYSQL_PROTOCOL_PIPE, MYSQL_PROTOCOL_MEMORY
	 */
	if (int_mysql_options(mysql, MYSQL_OPT_PROTOCOL, (char *) &protocol))
		return (9);

	/*- SSL options */
	if (use_ssl) {
		getEnvConfigStr(&cipher, "CIPHER", 0); /*- DHE-RSA-AES256-SHA */
		in_mysql_ssl_set(mysql, 0, 0, 0, 0, cipher); /*- this always returns 0 */
	}

#ifdef HAVE_MYSQL_OPT_SSL_CA
	/*-
	 * MYSQL_OPT_SSL_CA - The path name of the Certificate Authority (CA) certificate file.
	 * This option, if used, must specify the same certificate used by the server.
	 * community/mariadb
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_CA")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CA, ptr))
		return (10);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CAPATH
	/*-
	 * MYSQL_OPT_SSL_CAPATH
	 * The path name of the directory that contains trusted SSL CA certificate files.
	 * community/mariadb
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_CAPATH")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CAPATH, ptr))
		return (11);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CERT
	/*-
	 * MYSQL_OPT_SSL_CERT
	 * The path name of the client public key certificate file.
	 * community/mariadb
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_CERT")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CERT, ptr))
		return (12);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CIPHER
	/*-
	 * MYSQL_OPT_SSL_CIPHER
	 * The list of permitted ciphers for SSL encryption.
	 * community/mariadb
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_CIPHER")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CIPHER, ptr))
		return (13);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CRL
	/*
	 * MYSQL_OPT_SSL_CRL (argument type: char *)
	 * The path name of the file containing certificate revocation lists.
	 * community/mariadb
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_CRL")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CRL, ptr))
		return (14);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_CRLPATH
	/*-
	 * MYSQL_OPT_SSL_CRLPATH (argument type: char *)
	 * The path name of the directory that contains files containing certificate revocation lists.
	 * community/mariadb
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_CRLPATH")) && int_mysql_options(mysql, MYSQL_OPT_SSL_CRLPATH, ptr))
		return (15);
#endif
#ifdef HAVE_MYSQL_OPT_SSL_ENFORCE
	/*-
	 * MYSQL_OPT_SSL_ENFORCE (argument type: my_bool *)
	 *
	 * Whether to require the connection to use SSL. If enabled and an encrypted
	 * connection cannot be established, the connection attempt fails.
	 * This option is deprecated as of MySQL 5.7.11 and is removed in MySQL 8.0.
	 * Instead, use MYSQL_OPT_SSL_MODE with a value of SSL_MODE_REQUIRED.
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_ENFORCE")))
		scan_int(ptr, &t);
	if ((ptr && t) || use_ssl) {
		ssl_mode = 1;
		if (int_mysql_options(mysql, MYSQL_OPT_SSL_ENFORCE, &ssl_mode))
			return (16);
	}
#endif
#ifdef HAVE_MYSQL_OPT_SSL_MODE
	/*-
	 * MYSQL_OPT_SSL_MODE (argument type: unsigned int *)
	 *
	 * The security state to use for the connection to the server: 
	 * SSL_MODE_DISABLED, SSL_MODE_PREFERRED, SSL_MODE_REQUIRED, SSL_MODE_VERIFY_CA,
	 * SSL_MODE_VERIFY_IDENTITY.
	 * The default is SSL_MODE_PREFERRED. These modes are the permitted values of
	 * the mysql_ssl_mode enumeration defined in mysql.h.
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_MODE")) || use_ssl) {
		if (ptr) {
			scan_uint(ptr, &ssl_mode);
		} else
			ssl_mode = SSL_MODE_REQUIRED;
		if (int_mysql_options(mysql, MYSQL_OPT_SSL_MODE, &ssl_mode))
			return (18);
	}
#endif
#ifdef HAVE_MYSQL_OPT_SSL_VERIFY_SERVER_CERT
	/*-
	 * MYSQL_OPT_SSL_VERIFY_SERVER_CERT (argument type: my_bool *)
	 * Enable or disable verification of the server's Common Name identity in its certificate
	 * against the host name used when connecting to the server. The connection is rejected
	 * if there is a mismatch. For encrypted connections, this feature can be used to prevent
	 * man-in-the-middle attacks. Identity verification is disabled by default.
	 * This option does not work with self-signed certificates, which do not contain the
	 * server name as the Common Name value.
	 *
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_VERIFY_CERT"))) {
		scan_int(ptr, &t);
		tmpv_c = t ? 1 : 0;
		if (int_mysql_options(mysql, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &tmpv_c))
			return (17);
	}
#endif
#ifdef HAVE_MYSQL_OPT_SSL_KEY
	/*-
	 * MYSQL_OPT_SSL_KEY (argument type: char *)
	 * The path name of the client private key file.
	 * community/mariadb
	 */
	if ((ptr = env_get("MYSQL_OPT_SSL_KEY")) && int_mysql_options(mysql, MYSQL_OPT_SSL_KEY, ptr))
		return (19);
#endif
#ifdef HAVE_MYSQL_OPT_TLS_VERSION
	/*-
	 * MYSQL_OPT_TLS_VERSION (argument type: char *)
	 * The protocols permitted by the client for encrypted connections.
	 * The value is a comma-separated list containing one or more protocol names.
	 * The protocols that can be named for this option depend on the SSL library
	 * used to compile MySQL. 
	 *
	 * When compiled using OpenSSL 1.0.1 or higher, MySQL supports the TLSv1, TLSv1.1, and TLSv1.2 protocols.
	 * When compiled using the bundled version of yaSSL, MySQL supports the TLSv1 and TLSv1.1 protocols.
	 */
	if ((ptr = env_get("MYSQL_OPT_TLS_VERSION")) && int_mysql_options(mysql, MYSQL_OPT_TLS_VERSION, ptr))
		return (20);
#endif
	return (0);
}
