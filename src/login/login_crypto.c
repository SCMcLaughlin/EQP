
#include "login_crypto.h"
#include "eqp_basic.h"
#include "eqp_alloc.h"

STRUCT_DEFINE(LoginCrypto)
{
    DES_key_schedule    keySchedule;
    DES_key_schedule    keyScheduleTrilogy;
    byte                buffer[EQP_LOGIN_CRYPTO_BUFFER_SIZE];
};

LoginCrypto* login_crypto_create(Basic* basic)
{
    LoginCrypto* crypto     = eqp_alloc_type(basic, LoginCrypto);
    DES_cblock iv           = {0, 0, 0, 0, 0, 0, 0, 0};
    DES_cblock ivTrilogy    = {19, 217, 19, 109, 208, 52, 21, 251};
    
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);
    
    DES_key_sched(&iv, &crypto->keySchedule);
    DES_key_sched(&ivTrilogy, &crypto->keyScheduleTrilogy);
    
    return crypto;
}

void login_crypto_destroy(LoginCrypto* crypto)
{
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    
    free(crypto);
}

void login_crypto_clear(LoginCrypto* crypto)
{
    memset(crypto->buffer, 0, EQP_LOGIN_CRYPTO_BUFFER_SIZE);
}

void* login_crypto_data(LoginCrypto* crypto)
{
    return crypto->buffer;
}

uint32_t login_crypto_process(LoginCrypto* crypto, const void* input, uint32_t length, int encrypt, int isTrilogy)
{
    uint32_t rem = length % 8;
    
    if (rem)
        length += 8 - rem;
    
    if (length > EQP_LOGIN_CRYPTO_BUFFER_SIZE)
        length = EQP_LOGIN_CRYPTO_BUFFER_SIZE;
    
    if (!isTrilogy)
    {
        DES_cblock iv = {0, 0, 0, 0, 0, 0, 0, 0};
        DES_ncbc_encrypt((const byte*)input, crypto->buffer, length, &crypto->keySchedule, &iv, encrypt);
    }
    else
    {
        DES_cblock ivTrilogy = {19, 217, 19, 109, 208, 52, 21, 251};
        DES_ncbc_encrypt((const byte*)input, crypto->buffer, length, &crypto->keyScheduleTrilogy, &ivTrilogy, encrypt);
    }
    
    return length;
}

void login_crypto_hash(LoginCrypto* crypto, const char* password, uint32_t passlen, const byte* salt, uint32_t saltlen)
{
    PKCS5_PBKDF2_HMAC_SHA1(password, passlen, salt, saltlen, EQP_LOGIN_CRYPTO_HASH_ITERATIONS, EQP_LOGIN_CRYPTO_HASH_SIZE, crypto->buffer);
}
