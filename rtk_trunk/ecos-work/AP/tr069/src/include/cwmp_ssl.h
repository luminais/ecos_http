#ifndef _CWMP_SSL_H_
#define _CWMP_SSL_H_
#ifdef CWMP_ENABLE_SSL

#include "soapH.h"

#ifdef WITH_OPENSSL
int certificate_verify_cb(int ok, X509_STORE_CTX *store);
int CRYPTO_thread_setup();
void CRYPTO_thread_cleanup();
#elif defined(_WITH_MATRIXSSL_)
int certificate_verify_cb(sslCertInfo_t *cert, void *arg);
#endif

int certificate_setup( struct soap *soap, int use_cert );

#endif /*CWMP_ENABLE_SSL*/
#endif /*_CWMP_SSL_H_*/

