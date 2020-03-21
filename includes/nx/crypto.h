#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include <stdint.h>

#include "nx/nca.h"


typedef enum
{
    EncryptMode_Decrypt = 0x0,
    EncryptMode_Encrypt = 0x1
} EncryptMode;


uint8_t HEADER_KEK[0x10];
uint8_t HEADER_KEY[0x20];
uint8_t HEADER_KEY_0[0x10];
uint8_t HEADER_KEY_1[0x10];


//
bool init_crypto(void);

//
void exit_crypto(void);

//
bool has_keys(void);

//
bool has_key_gen(uint8_t key_gen);

//
const uint8_t *get_keak(uint8_t key_gen);

//


//
const uint8_t *return_keak_source(uint8_t crypto_type);

//
void *crypto_aes(const void *in, void *out, uint8_t *key, EncryptMode mode);

//
void *crypto_aes_ctr(const void *in, void *out, uint8_t *key, uint8_t *counter, size_t size, uint64_t offset, EncryptMode mode);

//
void *crypto_aes_cbc(const void *in, void *out, const uint8_t *key, size_t size, void *iv, EncryptMode mode);

// use to encrypt decrypt the nca header.
void *crypto_aes_xts(const void *in, void *out, uint8_t *key0, uint8_t *key1, uint64_t sector, size_t sector_size, size_t data_size, EncryptMode mode);

#endif