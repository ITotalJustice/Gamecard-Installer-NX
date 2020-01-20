#include <stdint.h>
#include <string.h>
#include <switch.h>

#include "nx/crypto.h"
#include "nx/nca.h"


uint8_t g_header_kek_src[0x10] = { 0x1F, 0x12, 0x91, 0x3A, 0x4A, 0xCB, 0xF0, 0x0D, 0x4C, 0xDE, 0x3A, 0xF6, 0xD5, 0x23, 0x88, 0x2A };
uint8_t g_header_key_src[0x20] = { 0x5A, 0x3E, 0xD8, 0x4F, 0xDE, 0xC0, 0xD8, 0x26, 0x31, 0xF7, 0xE2, 0x5D, 0x19, 0x7B, 0xF5, 0xD0, 0x1C, 0x9B, 0x7B, 0xFA, 0xF6, 0x28, 0x18, 0x3D, 0x71, 0xF6, 0x4D, 0x73, 0xF1, 0x50, 0xB9, 0xD2 };

const uint8_t g_keak_application_source[0x10] = { 0x7F, 0x59, 0x97, 0x1E, 0x62, 0x9F, 0x36, 0xA1, 0x30, 0x98, 0x06, 0x6F, 0x21, 0x44, 0xC3, 0x0D };
const uint8_t g_keak_ocean_source[0x10] = { 0x32, 0x7D, 0x36, 0x08, 0x5A, 0xD1, 0x75, 0x8D, 0xAB, 0x4E, 0x6F, 0xBA, 0xA5, 0x55, 0xD8, 0x82 };
const uint8_t g_keak_system_source[0x10] = { 0x87, 0x45, 0xF1, 0xBB, 0xA6, 0xBE, 0x79, 0x64, 0x7D, 0x04, 0x8B, 0xA6, 0x7B, 0x5F, 0xDA, 0x4A };


bool init_crypto(void)
{
    if (R_FAILED(splCryptoInitialize()))
    {
        return false;
    }
    if (R_FAILED(splCryptoGenerateAesKek(g_header_kek_src, 0, 0, HEADER_KEK)))
    {
        return false;
    }
    if (R_FAILED(splCryptoGenerateAesKey(HEADER_KEK, g_header_key_src, HEADER_KEY_0)))
    {
        return false;
    }
    if (R_FAILED(splCryptoGenerateAesKey(HEADER_KEK, g_header_key_src + 0x10, HEADER_KEY_1)))
    {
        return false;
    }
    return true;
}

void exit_crypto(void)
{
    splCryptoExit();
}

const uint8_t *return_keak_source(uint8_t crypto_type)
{
    switch (crypto_type)
    {
        case NcaKeyAreaEncryptionKeyIndex_Application: return g_keak_application_source;
        case NcaKeyAreaEncryptionKeyIndex_Ocean: return g_keak_ocean_source;
        case NcaKeyAreaEncryptionKeyIndex_System: return g_keak_system_source;
        default: return NULL;
    }
}

void *crypto_aes(const void *in, void *out, uint8_t *key, EncryptMode mode)
{
    Aes128Context ctx;
    aes128ContextCreate(&ctx, key, mode);

    switch (mode)
    {
        case EncryptMode_Decrypt:
        {
            aes128EncryptBlock(&ctx, out, in);
            break;
        }
        case EncryptMode_Encrypt:
        {
            aes128DecryptBlock(&ctx, out, in);
            break;
        }
    }

    return out;
}

void *crypto_aes_ctr(const void *in, void *out, uint8_t *key, uint8_t *counter, size_t size, uint64_t offset, EncryptMode mode)
{
    Aes128CtrContext ctx;
    aes128CtrContextCreate(&ctx, key, counter);

    uint64_t swp = __bswap64(offset >> 4);
    memcpy(&counter[0x8], &swp, 0x8);
    aes128CtrContextResetCtr(&ctx, counter);
    aes128CtrCrypt(&ctx, out, in, size);

    return out;
}

void *crypto_aes_cbc(const void *in, void *out, const uint8_t *key, size_t size, void *iv, EncryptMode mode)
{
    Aes128CbcContext ctx;
    aes128CbcContextCreate(&ctx, key, iv, mode);
    aes128CbcContextResetIv(&ctx, iv);

    switch (mode)
    {
        case EncryptMode_Decrypt:
        {
            aes128CbcDecrypt(&ctx, out, in, size);
            break;
        }
        case EncryptMode_Encrypt:
        {
            aes128CbcEncrypt(&ctx, out, in, size);
            break;
        }
    }

    return out;
}

void *crypto_aes_xts(const void *in, void *out, uint8_t *key0, uint8_t *key1, uint64_t sector, size_t sector_size, size_t data_size, EncryptMode mode)
{
    Aes128XtsContext ctx;
    aes128XtsContextCreate(&ctx, key0, key1, mode);

    for (uint64_t pos = 0; pos < data_size; pos += sector_size)
    {
        aes128XtsContextResetSector(&ctx, sector++, true);
        switch (mode)
        {
            case EncryptMode_Decrypt:
            {
                aes128XtsDecrypt(&ctx, (uint8_t *)out + pos, (const uint8_t *)in + pos, sector_size);
                break;
            }
            case EncryptMode_Encrypt:
            {
                aes128XtsEncrypt(&ctx, (uint8_t *)out + pos, (const uint8_t *)in + pos, sector_size);
                break;
            }
        }
    }

    return out;
}