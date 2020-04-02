#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "nx/crypto.h"
#include "nx/nca.h"
#include "nx/fs.h"

#include "util/util.h"
#include "util/file.h"
#include "util/log.h"


#define KEYGEN_LIMIT    0x20


typedef struct
{
    uint8_t total;
    NcaKeyArea_t key[KEYGEN_LIMIT];
} KeySection_t;

typedef struct
{
    KeySection_t keak[0x3]; // index
    KeySection_t title_key;
    KeySection_t mkey;
} keyz_t;

uint32_t g_max_keygen = 0;

bool g_has_keyz = false;
keyz_t g_keyz = {0};

// no, these are no the actual keys.
const uint8_t g_header_kek_src[0x10] = { 0x1F, 0x12, 0x91, 0x3A, 0x4A, 0xCB, 0xF0, 0x0D, 0x4C, 0xDE, 0x3A, 0xF6, 0xD5, 0x23, 0x88, 0x2A };
const uint8_t g_header_key_src[0x20] = { 0x5A, 0x3E, 0xD8, 0x4F, 0xDE, 0xC0, 0xD8, 0x26, 0x31, 0xF7, 0xE2, 0x5D, 0x19, 0x7B, 0xF5, 0xD0, 0x1C, 0x9B, 0x7B, 0xFA, 0xF6, 0x28, 0x18, 0x3D, 0x71, 0xF6, 0x4D, 0x73, 0xF1, 0x50, 0xB9, 0xD2 };

// neither are these.
const uint8_t g_keak_application_source[0x10] = { 0x7F, 0x59, 0x97, 0x1E, 0x62, 0x9F, 0x36, 0xA1, 0x30, 0x98, 0x06, 0x6F, 0x21, 0x44, 0xC3, 0x0D };
const uint8_t g_keak_ocean_source[0x10] = { 0x32, 0x7D, 0x36, 0x08, 0x5A, 0xD1, 0x75, 0x8D, 0xAB, 0x4E, 0x6F, 0xBA, 0xA5, 0x55, 0xD8, 0x82 };
const uint8_t g_keak_system_source[0x10] = { 0x87, 0x45, 0xF1, 0xBB, 0xA6, 0xBE, 0x79, 0x64, 0x7D, 0x04, 0x8B, 0xA6, 0x7B, 0x5F, 0xDA, 0x4A };

// nor is this.
const uint8_t g_titlekey_source[0x10] = { 0x1e, 0xdc, 0x7b, 0x3b, 0x60, 0xe6, 0xb4, 0xd8, 0x78, 0xb8, 0x17, 0x15, 0x98, 0x5e, 0x62, 0x9b };


bool parse_keys(void);

bool init_crypto(void)
{
    if (R_FAILED(splCryptoInitialize()))
    {
        return false;
    }

    // generate header kek.
    if (R_FAILED(splCryptoGenerateAesKek(g_header_kek_src, 0, 0, HEADER_KEK)))
    {
        return false;
    }
    // gen header key 0.
    if (R_FAILED(splCryptoGenerateAesKey(HEADER_KEK, g_header_key_src, HEADER_KEY_0)))
    {
        return false;
    }

    // gen header key 1.
    if (R_FAILED(splCryptoGenerateAesKey(HEADER_KEK, g_header_key_src + 0x10, HEADER_KEY_1)))
    {
        return false;
    }

    // parse keys file.
    g_has_keyz = parse_keys();

    // get HOS keygen.
    g_max_keygen = fs_get_key_gen_from_boot0();

    return true;
}

void exit_crypto(void)
{
    splCryptoExit();
}

uint32_t crypto_get_sys_key_gen(void)
{
    return g_max_keygen;
}

uint32_t crypto_get_highest_key_gen(NcaKeyAreaEncryptionKeyIndex index)
{
    return g_keyz.keak[index].total > g_max_keygen ? g_keyz.keak[index].total : g_max_keygen;
}

bool crypto_has_key_gen(NcaKeyAreaEncryptionKeyIndex index, uint32_t key_gen)
{
    return key_gen <= crypto_get_highest_key_gen(index);
}

bool has_keys(void)
{
    return g_has_keyz;
}

bool crypto_has_key_gen_from_keys(NcaKeyAreaEncryptionKeyIndex index, uint8_t key_gen)
{
    if (index > 0x2 || key_gen > KEYGEN_LIMIT)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    return key_gen <= g_keyz.keak[index].total;
}

const uint8_t *crypto_get_keak_from_keys(NcaKeyAreaEncryptionKeyIndex index, uint8_t key_gen)
{
    if (!crypto_has_key_gen_from_keys(index, key_gen))
    {
        return NULL;
    }

    if (key_gen) key_gen--;

    #ifdef DEBUG
    write_log("\n keygen: %u got key: ", key_gen);
    for (int i = 0; i < sizeof(NcaKeyArea_t); i++)
    {
        write_log("%x", g_keyz.keak[index].key[key_gen].area[i]);
    }
    write_log("\n");
    #endif
    return g_keyz.keak[index].key[key_gen].area;
}

const uint8_t *crypto_get_titlekek_from_keys(uint8_t key_gen)
{
    if (key_gen) key_gen--;
    return g_keyz.title_key.key[key_gen].area;
}

size_t __get_file_size(FILE *fp)
{
    if (!fp) return 0;
    size_t off = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, off, SEEK_SET);
    return size;
}

bool find_keys_file(char **out, size_t *out_size)
{
    if (!out || !out_size)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    const char *KEY_PATHS[] =
    {
        // default
        "sdmc:/switch/prod.keys",
        "sdmc:/switch/keys.txt",
    };

    FILE *fp = {0};
    for (uint8_t i = 0; i < 2; i++)
    {
        fp = fopen(KEY_PATHS[i], "r");
        if (fp)
        {
            *out_size = __get_file_size(fp) + 1;
            if (*out_size > 0x800000)   // way too big to be a simple keys file. lets not load that.
            {
                write_log("keys file is way too large: %lu\n", *out_size);
                fclose(fp);
                return false;
            }
            *out = calloc(1, *out_size);
            if (!(*out))
            {
                fclose(fp);
                return false;
            }
            fread(*out, *out_size - 1, 1, fp);
            fclose(fp);
            return true;
        }
    }
    return false;
}

bool parse_hex_key(NcaKeyArea_t *keak, const char *hex, uint32_t len)
{
    if (!keak || !hex)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    char low[0x11] = {0};
    char upp[0x11] = {0};
    memcpy(low, hex, 0x10);
    memcpy(upp, hex + 0x10, 0x10);
    *(uint64_t *)keak->area = __bswap64(strtoul(low, NULL, 0x10));
    *(uint64_t *)(keak->area + 8) = __bswap64(strtoul(upp, NULL, 0x10));
    return true;
}

void find_keys_in_file(char *loaded_file, const char *search_key, KeySection_t *key_section)
{
    if (!loaded_file || !search_key || !key_section)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }

    char *ed = loaded_file;
    char parse_string[0x100] = {0};
    uint8_t skip = strlen(search_key) + 2 + 3;  // two extra hex + 2 spaces and =.

    for (uint8_t i = 0; i < KEYGEN_LIMIT; i++)
    {
        sprintf(parse_string, "%s%02x", search_key, i);
        if (!(ed = strstr(ed, parse_string)))
            break;
        ed += skip;
        write_log("found %s: %.32s\n", parse_string, ed);
        parse_hex_key(&key_section->key[i], ed, sizeof(NcaKeyArea_t));
        ed += (sizeof(NcaKeyArea_t) * 2) + 1;
        key_section->total++;
    }
}

bool parse_keys(void)
{
    // find keys.
    char *buf = {0};
    size_t size = 0;
    if (!find_keys_file(&buf, &size))
    {
        return false;
    }

    const char *key_text_keak_app = "key_area_key_application_";
    const char *key_text_keak_oce = "key_area_key_ocean_";
    const char *key_text_keak_sys = "key_area_key_system_";
    const char *key_text_title_kek = "titlekek_";
    const char *key_text_mkey = "master_key_";

    find_keys_in_file(buf, key_text_keak_app, &g_keyz.keak[NcaKeyAreaEncryptionKeyIndex_Application]);
    find_keys_in_file(buf, key_text_keak_oce, &g_keyz.keak[NcaKeyAreaEncryptionKeyIndex_Ocean]);
    find_keys_in_file(buf, key_text_keak_sys, &g_keyz.keak[NcaKeyAreaEncryptionKeyIndex_System]);
    find_keys_in_file(buf, key_text_title_kek, &g_keyz.title_key);
    find_keys_in_file(buf, key_text_mkey, &g_keyz.mkey);

    free(buf);
    return true;
}

const uint8_t *crypto_return_keak_source(NcaKeyAreaEncryptionKeyIndex index)
{
    switch (index)
    {
        case NcaKeyAreaEncryptionKeyIndex_Application:
            return g_keak_application_source;
        case NcaKeyAreaEncryptionKeyIndex_Ocean:
            return g_keak_ocean_source;
        case NcaKeyAreaEncryptionKeyIndex_System:
            return g_keak_system_source;
        default:
            return NULL;
    }
}

void *crypto_aes(const void *in, void *out, uint8_t *key, EncryptMode mode)
{
    Aes128Context ctx = {0};
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

void *crypto_aes_ctr(const void *in, void *out, uint8_t *key, uint8_t *counter, size_t size, uint64_t offset)
{
    Aes128CtrContext ctx = {0};
    aes128CtrContextCreate(&ctx, key, counter);

    uint64_t swp = __bswap64(offset >> 4);
    memcpy(&counter[0x8], &swp, 0x8);
    aes128CtrContextResetCtr(&ctx, counter);
    aes128CtrCrypt(&ctx, out, in, size);

    return out;
}

void *crypto_aes_cbc(const void *in, void *out, const uint8_t *key, size_t size, void *iv, EncryptMode mode)
{
    Aes128CbcContext ctx = {0};
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
    Aes128XtsContext ctx = {0};
    aes128XtsContextCreate(&ctx, key0, key1, mode);

    for (uint64_t pos = 0; pos < data_size; pos += sector_size)
    {
        aes128XtsContextResetSector(&ctx, sector++, true);
        switch (mode)
        {
            case EncryptMode_Decrypt:
            {
                aes128XtsDecrypt(&ctx, (uint8_t*)out + pos, (const uint8_t*)in + pos, sector_size);
                break;
            }
            case EncryptMode_Encrypt:
            {
                aes128XtsEncrypt(&ctx, (uint8_t*)out + pos, (const uint8_t*)in + pos, sector_size);
                break;
            }
        }
    }

    return out;
}