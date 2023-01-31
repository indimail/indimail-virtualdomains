#ifndef TLS_AUX_H
#define TLS_AUX_H 1

#undef LIBWOLFSSL_VERSION_HEX

#include "config.h"
#include "fetchmail.h"

#ifdef SSL_ENABLE

# ifdef HAVE_WOLFSSL_OPTIONS_H
#  include <wolfssl/options.h>
# endif

# include <openssl/ssl.h>

# undef USING_WOLFSSL
# ifdef LIBWOLFSSL_VERSION_HEX
#  define USING_WOLFSSL 1
#  define OSSL110_API 1
# else
#  if OPENSSL_VERSION_NUMBER < 0x1010000fL
#   undef OSSL110_API
#  else
#   define OSSL110_API 1
#  endif
#  if OPENSSL_VERSION_NUMBER < 0x1010000fL
#   define OpenSSL_version(t) SSLeay_version((t))
#   define OpenSSL_version_num() SSLeay()
#   define OPENSSL_VERSION (SSLEAY_VERSION)
#   define OPENSSL_DIR (SSLEAY_DIR)
#   define OPENSSL_ENGINES_DIR (-1)
#  endif
# endif /* LIBWOLFSSL_VERSION_STRING */
#endif /* SSL_ENABLE */

#endif /* TLS_AUX_H */
