
#ifndef EQP_LOGIN_CRYPTO_H
#define EQP_LOGIN_CRYPTO_H

#include "define.h"
#include "random.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/des.h>

#define EQP_LOGIN_CRYPTO_BUFFER_SIZE        2048
#define EQP_LOGIN_CRYPTO_HASH_SIZE          20      /* Recommended maximum for PBKDF2 using SHA1 */
#define EQP_LOGIN_CRYPTO_HASH_ITERATIONS    50000   /* Takes ~80 milliseconds on a 1.90GHz laptop CPU core */

STRUCT_DECLARE(Basic);
STRUCT_DECLARE(LoginCrypto);

LoginCrypto*    login_crypto_create(R(Basic*) basic);
void            login_crypto_destroy(R(LoginCrypto*) crypto);
void            login_crypto_clear(R(LoginCrypto*) crypto);

void*           login_crypto_data(R(LoginCrypto*) crypto);
#define         login_crypto_data_type(crypto, type) ((type*)login_crypto_data((crypto)))

uint32_t        login_crypto_process(R(LoginCrypto*) crypto, R(const void*) input, uint32_t length, int encrypt, int isTrilogy);
#define         login_crypto_encrypt_standard(crypto, input, len) login_crypto_process((crypto), (input), (len), 1, 0)
#define         login_crypto_decrypt_standard(crypto, input, len) login_crypto_process((crypto), (input), (len), 0, 0)
#define         login_crypto_encrypt_trilogy(crypto, input, len) login_crypto_process((crypto), (input), (len), 1, 1)
#define         login_crypto_decrypt_trilogy(crypto, input, len) login_crypto_process((crypto), (input), (len), 0, 1)

void            login_crypto_hash(R(LoginCrypto*) crypto, R(const char*) password, uint32_t passlen, R(const byte*) salt, uint32_t saltlen);

#endif//EQP_LOGIN_CRYPTO_H
