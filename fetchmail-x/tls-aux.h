#ifndef TLS_AUX_H
#define TLS_AUX_H 1

#include "config.h"
#include "fetchmail.h"


#ifdef SSL_ENABLE
#include <openssl/opensslv.h>

# if defined(LIBRESSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER < 0x1010000fL
#  undef OSSL110_API
# else
#  define OSSL110_API 1
# endif
# if OPENSSL_VERSION_NUMBER < 0x1010000fL
#  define OpenSSL_version(t) SSLeay_version((t))
#  define OpenSSL_version_num() SSLeay()
#  define OPENSSL_VERSION (SSLEAY_VERSION)
#  define OPENSSL_DIR (SSLEAY_DIR)
#  define OPENSSL_ENGINES_DIR (-1)
# endif
#endif /* SSL_ENABLE */

#endif /* TLS_AUX_H */
