#include "config.h"
#include "fetchmail.h"

#ifdef SSL_ENABLE
#include <stdlib.h>
#include <string.h>
#include <openssl/x509.h>

/** return a constant copy of the default SSL certificate path
 * the directory with hashed certificates, see
 * SSL_CTX_load_verify_locations(3),
 * not to be modified by caller. */
const char *get_default_cert_path(void) {
	const char *rb = (char *)0, *tmp;

	tmp = X509_get_default_cert_dir_env();
	if (tmp) rb = getenv(tmp);
	if (!rb) rb = X509_get_default_cert_dir();

	return rb;
}

/** return a constant copy of the default SSL certificate file
 * the directory with hashed certificates, see
 * SSL_CTX_load_verify_locations(3),
 * not to be modified by caller. */
const char *get_default_cert_file(void) {
	const char *rb = (char *)0, *tmp;

	tmp = X509_get_default_cert_file_env();
	if (tmp) rb = getenv(tmp);
	if (!rb) rb = X509_get_default_cert_file();

	return rb;
}

#endif /* SSL_ENABLE */

#ifdef TEST
#include <stdio.h>

int main(void) {
#ifdef SSL_ENABLE
	const char *tmp;

	tmp = get_default_cert_file();
	printf("X509 default cert file: %s\n", tmp ? tmp : "(null)");

	tmp = get_default_cert_path();
	printf("X509 default cert path: %s\n", tmp ? tmp : "(null)");
#else
	puts("SSL support not compiled in.");
#endif /* SSL_ENABLE */
	exit(EXIT_SUCCESS);
}
#endif /* TEST */
