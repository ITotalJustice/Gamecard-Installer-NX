#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <switch.h>

#include "nx/nca.h"
#include "nx/ncm.h"
#include "nx/fs.h"
#include "nx/crypto.h"

#include "ui/menu.h"
#include "ui/settings.h"

#include "util/file.h"
#include "util/log.h"


#define BUFFER_SIZE 0x800000


mtx_t nca_mtx = {0};
cnd_t can_read = {0};
cnd_t can_write = {0};

typedef struct
{
    FILE *fp;
    uint8_t *data;
    uint64_t offset;
    size_t data_size;
    size_t data_read;
    size_t data_written;
    size_t total_size;
    NcmInstall_t ncm;
} NcaInstall_t;


NcaKeySlot_t KEYSLOT = {0};

bool nca_set_keyslot(uint64_t id, const uint8_t *key)
{
    if (!id || !key)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    // reset to 0.
    memset(&KEYSLOT, 0, sizeof(NcaKeySlot_t));

    // set new values.
    KEYSLOT.id = id;
    memcpy(KEYSLOT.key.area, key, 0x10);

    return true;
}

NcaKeySlot_t *nca_get_keyslot(void)
{
    return &KEYSLOT;
}

NcaKeyArea_t *nca_get_keyslot_key(void)
{
    return &KEYSLOT.key;
}

uint64_t nca_get_keyslot_id(void)
{
    return KEYSLOT.id;
}

bool nca_check_if_magic_valid(uint32_t magic)
{
    switch (magic)
    {
        case NCA0_MAGIC:
            return true;
        case NCA2_MAGIC:
            return true;
        case NCA3_MAGIC:
            return true;
        default:
            return false;
    }
}

const char *nca_get_magic_string(uint32_t magic)
{
    switch (magic)
    {
        case NCA0_MAGIC:
            return "NCA0";
        case NCA2_MAGIC:
            return "NCA1";
        case NCA3_MAGIC:
            return "NCA3";
        default:
            return "NULL";
    }
}

const char *nca_get_distribution_type_string(NcaDistributionType type)
{
    switch (type)
    {
        case NcaDistributionType_System:
            return "System";
        case NcaDistributionType_GameCard:
            return "GameCard";
        default:
            return "NULL";
    }
}

const char *nca_get_content_type_string(NcaContentType type)
{
    switch (type)
    {
        case NcaContentType_Program:
            return "Program";
        case NcaContentType_Meta:
            return "Meta";
        case NcaContentType_Control:
            return "Control";
        case NcaContentType_Manual:
            return "Manual";
        case NcaContentType_Data:
            return "Data";
        case NcaContentType_PublicData:
            return "PublicData";
        default:
            return "NULL";
    }
}

const char *nca_get_keak_index_string(NcaKeyAreaEncryptionKeyIndex index)
{
    switch (index)
    {
        case NcaKeyAreaEncryptionKeyIndex_Application:
            return "Application";
        case NcaKeyAreaEncryptionKeyIndex_Ocean:
            return "Ocean";
        case NcaKeyAreaEncryptionKeyIndex_System:
            return "System";
        default:
            return "NULL";
    }
}

const char *nca_get_file_system_type_string(NcaFileSystemType type)
{
    switch (type)
    {
        case NcaFileSystemType_RomFS:
            return "RomFS";
        case NcaFileSystemType_PFS0:
            return "PFS0";
        default:
            return "NULL";
    }
}

const char *nca_get_hash_type_string(NcaHashType type)
{
    switch (type)
    {
        case NcaHashType_Auto:
            return "Auto";
        case NcaHashType_HierarchicalSha256:
            return "HierarchicalSha256";
        case NcaHashType_HierarchicalIntegrity:
            return "HierarchicalIntegrity";
        default:
            return "NULL";
    }
}

const char *nca_get_encryption_type_string(NcaEncryptionType type)
{
    switch (type)
    {
        case NcaEncryptionType_Auto:
            return "Auto";
        case NcaEncryptionType_None:
            return "None";
        case NcaEncryptionType_AesCtrOld:
            return "AesCtrOld";
        case NcaEncryptionType_AesCtr:
            return "AesCtr";
        case NcaEncryptionType_AesCtrEx:
            return "AesCtrEx";
        default:
            return "NULL";
    }
}

const char *nca_return_key_gen_string(uint8_t key_gen)
{
    switch (key_gen)
    {
        case NcaOldKeyGeneration_100:
            return "1.0.0";
        case NcaOldKeyGeneration_Unused:
            return "Unused";
        case NcaOldKeyGeneration_300:
            return "3.0.0";
        case NcaKeyGeneration_301:
            return "3.0.1";
        case NcaKeyGeneration_400:
            return "4.0.0";
        case NcaKeyGeneration_500:
            return "5.0.0";
        case NcaKeyGeneration_600:
            return "6.0.0";
        case NcaKeyGeneration_620:
            return "6.2.0";
        case NcaKeyGeneration_700:
            return "7.0.0";
        case NcaKeyGeneration_810:
            return "8.1.0";
        case NcaKeyGeneration_900:
            return "9.0.0";
        case NcaKeyGeneration_910:
            return "9.1.0";
        case NcaKeyGeneration_Invalid:
            return "Invalid";
        default:
            return "UNKNOWN";
    }
}

uint16_t nca_return_key_gen_int(uint8_t key_gen)
{
    switch (key_gen)
    {
        case NcaOldKeyGeneration_100:
            return 100;
        case NcaOldKeyGeneration_Unused:
            return 100;
        case NcaOldKeyGeneration_300:
            return 300;
        case NcaKeyGeneration_301:
            return 301;
        case NcaKeyGeneration_400:
            return 400;
        case NcaKeyGeneration_500:
            return 500;
        case NcaKeyGeneration_600:
            return 600;
        case NcaKeyGeneration_620:
            return 620;
        case NcaKeyGeneration_700:
            return 700;
        case NcaKeyGeneration_810:
            return 810;
        case NcaKeyGeneration_900:
            return 900;
        case NcaKeyGeneration_910:
            return 910;
        case NcaKeyGeneration_Invalid:
            return UINT16_MAX;
        default:
            return 0;
    }
}

const char *nca_get_string_from_id(const NcmContentId *nca_id, char *out)
{
    uint64_t nca_id_lower = __bswap64(*(uint64_t *)nca_id->c);
    uint64_t nca_id_upper = __bswap64(*(uint64_t *)(nca_id->c + 0x8));
    snprintf(out, 0x21, "%016lx%016lx", nca_id_lower, nca_id_upper);
    return out;
}

const NcmContentId nca_get_id_from_string(const char *nca_in_string)
{
    NcmContentId nca_id = {0};
    char lowerU64[0x11] = {0};
    char upperU64[0x11] = {0};
    memcpy(lowerU64, nca_in_string, 0x10);
    memcpy(upperU64, nca_in_string + 0x10, 0x10);
    *(uint64_t *)nca_id.c = __bswap64(strtoul(lowerU64, NULL, 0x10));
    *(uint64_t *)(nca_id.c + 8) = __bswap64(strtoul(upperU64, NULL, 0x10));
    return nca_id;
}

bool nca_encrypt_header(const NcaHeader_t *in_header, NcaHeader_t *out_header)
{
    if (!in_header || !out_header)
    {
        write_log("empty params in %s\n", __func__);
        return false;
    }

    if (!crypto_aes_xts(in_header, out_header, HEADER_KEY_0, HEADER_KEY_1, 0, NCA_SECTOR_SIZE, NCA_XTS_SECTION_SIZE, EncryptMode_Encrypt))
    {
        return false;
    }

    return true;
}

bool nca_decrypt_header(const NcaHeader_t *in_header, NcaHeader_t *out_header)
{
    if (!in_header || !out_header)
    {
        write_log("empty params in %s\n", __func__);
        return false;
    }

    if (!crypto_aes_xts(in_header, out_header, HEADER_KEY_0, HEADER_KEY_1, 0, NCA_SECTOR_SIZE, NCA_XTS_SECTION_SIZE, EncryptMode_Decrypt))
    {
        write_log("failed to decrypt nac header\n");
        return false;
    }

    return true;
}

bool nca_get_header(FILE *fp, uint64_t offset, NcaHeader_t *header)
{
    if (!fp || !header)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    return read_file(header, NCA_XTS_SECTION_SIZE, offset, fp) == NCA_XTS_SECTION_SIZE;
}

bool nca_get_header_decrypted(FILE *fp, uint64_t offset, NcaHeader_t *header)
{
    if (!fp || !header)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    if (!nca_get_header(fp, offset, header))
    {
        return false;
    }

    return nca_decrypt_header(header, header);
}

bool nca_has_rights_id(const FsRightsId *rights_id)
{
    if (!rights_id)
    {
        write_log("empty param in %s\n", __func__);
        return false;
    }

    for (uint8_t i = 0; i < sizeof(FsRightsId); i++)
    {
        if (rights_id->c[i])
        {
            return true;
        }
    }
    return false;
}

bool nca_set_rights_id(FsRightsId *rights_id)
{
    if (!rights_id)
    {
        write_log("missing args in %s\n", __func__);
        return false;
    }

    for (uint8_t i = 0; i < sizeof(FsRightsId); i++)
    {
        rights_id->c[i] = 0x0;
    }

    return true;
}

void nca_set_distribution_type(NcaHeader_t *header)
{
    if (!header)
    {
        write_log("empty param in %s\n", __func__);
        return;
    }
    header->distribution_type = NcaDistributionType_System;
}



/*
*   nca install stuff.
*/

int nca_read(void *in)
{
    NcaInstall_t *t = (NcaInstall_t *)in;
    if (!t)
    {
        write_log("missing params in %s\n", __func__);
        return -1;
    }

    void *buf = calloc(1, BUFFER_SIZE);
    if (!buf)
    {
        write_log("failed to alloc buffer in %s\n", __func__);
        return -1;
    }

    for (uint64_t done = t->data_written, bufsize = BUFFER_SIZE; done < t->total_size; done += bufsize, t->offset += bufsize)
    {
        if (done + bufsize > t->total_size)
            bufsize = t->total_size - done;

        read_file(buf, bufsize, t->offset, t->fp);

        mtx_lock(&nca_mtx);
        if (t->data_size != 0)
        {
            cnd_wait(&can_read, &nca_mtx);
        }

        memcpy(t->data, buf, bufsize);
        t->data_size = bufsize;

        mtx_unlock(&nca_mtx);
        cnd_signal(&can_write);
    }

    free(buf);
    return 0;
}

int nca_write(void *in)
{
    NcaInstall_t *t = (NcaInstall_t *)in;
    if (!t)
    {
        write_log("missing params in %s\n", __func__);
        return -1;
    }

    while (t->data_written < t->total_size)
    {
        mtx_lock(&nca_mtx);
        if (t->data_size == 0)
        {
            cnd_wait(&can_write, &nca_mtx);
        }

        if (!ncm_write_placeholder_id(&t->ncm.storage, &t->ncm.placeholder_id, t->data_written, t->data, t->data_size))
        {
            return -1;
        }

        t->data_written += t->data_size;
        t->data_size = 0;

        mtx_unlock(&nca_mtx);
        cnd_signal(&can_read);
    }

    return 0;
}

bool nca_setup_placeholder_id(NcmInstall_t *out, size_t size, const NcmContentId *content_id, NcmStorageId storage_id)
{
    // open ncm storage.
    if (!ncm_open_storage(&out->storage, storage_id))
    {
        return false;
    }

    // generate a placeholder id.
    if (!ncm_generate_placeholder_id(&out->storage, &out->placeholder_id))
    {
        ncm_close_storage(&out->storage);
        return false;
    }

    // check if the placeholder exists, delete if found.
    if (ncm_check_if_placeholder_id_exists(&out->storage, &out->placeholder_id))
    {
        if (!ncm_delete_placeholder_id(&out->storage, &out->placeholder_id))
        {
            ncm_close_storage(&out->storage);
            return false;
        }
    }

    // finally create placeholder.
    if (!ncm_create_placeholder_id(&out->storage, memcpy(&out->content_id, content_id, sizeof(NcmContentId)), &out->placeholder_id, size))
    {
        ncm_close_storage(&out->storage);
        return false;
    }
    
    return true;
}

void nca_update_lower_ctr(uint8_t *ctr, uint64_t offset)
{
    if (!ctr)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }
    
    uint64_t swp = __bswap64(offset);
    memcpy(ctr, &swp, 0x8);
}

void nca_update_upper_ctr(uint8_t *ctr, uint64_t offset)
{
    if (!ctr)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }
    
    uint64_t swp = __bswap64(offset >> 4);
    memcpy(ctr + 0x8, &swp, 0x8);
}

bool nca_decrypt_section(const void *in, void *out, const NcaKeyArea_t *key, uint8_t *ctr, size_t size, uint64_t offset)
{
    return true;
}

bool nca_read_encrypted(void *out, const NcaKeyArea_t *key, uint8_t *ctr, size_t size, uint64_t offset, FILE *fp)
{
    fread(out, size, 1, fp);
    return nca_decrypt_section(out, out, key, ctr, size, offset);
}

bool __nca_decrypt_keak_keyz(const NcaHeader_t *header, NcaKeyArea_t *out, uint32_t key_gen)
{
    write_log("using keys to decrypt keak\n");

    const uint8_t *keak = crypto_get_keak_from_keys(header->kaek_index, key_gen);
    if (!keak)
    {
        write_log("no return keak %s\n", __func__);
        return false;
    }

    // decrypt to get the key.
    NcaKeyArea_t decrypted_nca_keys[NCA_SECTION_TOTAL] = {0};
    Aes128Context ctx_dec = {0};

    aes128ContextCreate(&ctx_dec, keak, false);
    for (uint8_t i = 0; i < NCA_SECTION_TOTAL; i++)
    {
        aes128DecryptBlock(&ctx_dec, &decrypted_nca_keys[i], &header->key_area[i]);
    }

    nca_print_key_area(header->key_area);
    nca_print_key_area(decrypted_nca_keys);

    // copy decrypted key.
    memcpy(out, &decrypted_nca_keys[0x2], sizeof(NcaKeyArea_t));
    return true;
}

bool __nca_decrypt_keak_spl(const NcaHeader_t *header, NcaKeyArea_t *out, uint32_t key_gen)
{
    write_log("using spl to decrypt keak\n");

    NcaKeyArea_t kek = {0};
    if (R_FAILED(splCryptoGenerateAesKek(crypto_return_keak_source(header->kaek_index), key_gen, 0, kek.area)))
    {
        write_log("failed to generate aea kek\n");
        return false;
    }

    NcaKeyArea_t decrypted_nca_keys[NCA_SECTION_TOTAL] = {0};
    for (uint8_t i = 0; i < NCA_SECTION_TOTAL; i++)
    {
        if (R_FAILED(splCryptoGenerateAesKey(kek.area, &header->key_area[i].area, &decrypted_nca_keys[i].area)))
        {
            write_log("failed to aes key %i %s\n", i, __func__);
            return false;
        }
    }

    nca_print_key_area(header->key_area);
    nca_print_key_area(decrypted_nca_keys);

    // copy decrypted key.
    memcpy(out, &decrypted_nca_keys[0x2], sizeof(NcaKeyArea_t));
    return true;
}

bool nca_decrypt_keak(const NcaHeader_t *header, NcaKeyArea_t *out)
{
    if (!header || !out)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    // sort out the key gen.
    uint8_t key_gen = header->key_gen ? header->key_gen : header->old_key_gen;

    // check if we can use spl.
    if (crypto_get_sys_key_gen() >= key_gen)
    {
        return __nca_decrypt_keak_spl(header, out, key_gen);
    }
    else
    {
        return __nca_decrypt_keak_keyz(header, out, key_gen);
    }

    return true;
}

bool __nca_encrypt_keak_keyz(NcaHeader_t *header, const NcaKeyArea_t *decrypted_key, uint8_t key_gen)
{
    if (!header || !decrypted_key)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    Aes128Context ctx = {0};
    NcaKeyArea_t decrypted_nca_keys[NCA_SECTION_TOTAL] = {0};
    memcpy(&decrypted_nca_keys[0x2].area, decrypted_key->area, sizeof(NcaKeyArea_t));

    const uint8_t *keak = crypto_get_keak_from_keys(header->kaek_index, key_gen);
    if (!keak)
    {
        return false;
    }

    aes128ContextCreate(&ctx, keak, true);
    for (uint8_t i = 0; i < NCA_SECTION_TOTAL; i++)
    {
        aes128EncryptBlock(&ctx, &header->key_area[i].area, &decrypted_nca_keys[i].area);
    }

    return true;
}

bool __nca_ecnrypt_keak_spl(NcaHeader_t *header, const NcaKeyArea_t *decrypted_key, uint8_t key_gen)
{
    /*
    *   TODO:
    */
    return true;
}

bool nca_encrypt_keak(NcaHeader_t *header, const NcaKeyArea_t *decrypted_key, uint8_t key_gen)
{
    if (!header || !decrypted_key)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    if (!__nca_encrypt_keak_keyz(header, decrypted_key, key_gen))
    {
        return false;
    }

    // set the rights_id field.
    memset(&header->rights_id, 0, sizeof(FsRightsId));

    // update with new keygen version.
    if (key_gen > NcaOldKeyGeneration_300)
    {
        header->key_gen = key_gen;
        header->old_key_gen = NcaOldKeyGeneration_300;
    }
    else
    {
        header->key_gen = 0;
        header->old_key_gen = key_gen;
    }

    return true;
}

bool nca_start_install(const char *name, const NcmContentId *content_id, uint64_t offset, NcmStorageId storage_id, FILE *fp)
{
    if (!content_id || !fp || !name)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    // this will be the thread that is passed to the read / write.
    NcaInstall_t nca = {0};
    nca.fp = fp;
    
    // next we need to get the header.
    NcaHeader_t *header = calloc(1, NCA_XTS_SECTION_SIZE);
    if (!header)
    {
        write_log("failed to malloc header\n");
        return false;
    }

    if (!nca_get_header_decrypted(nca.fp, offset, header))
    {
        write_log("failed to get nca header\n");
        free(header);
        return false;
    }

    nca_print_header(header);

    if (setting_get_install_lower_key_gen())
    {
        if (nca_has_rights_id(&header->rights_id))
        {
            if (!nca_encrypt_keak(header, nca_get_keyslot_key(), 0))
            {
                free(header);
                return false;
            }
        }
        else
        {
            NcaKeyArea_t keak = {0};
            if (!nca_decrypt_keak(header, &keak))
            {
                free(header);
                return false;
            }
            if (!nca_encrypt_keak(header, &keak, 0))
            {
                free(header);
                return false;
            }
        }
    }

    // now that we have the actual size of the nca, we can setup the placeholder.
    if (!nca_setup_placeholder_id(&nca.ncm, header->size, content_id, storage_id))
    {
        write_log("failed to setup placeholder\n");
        free(header);
        return false;
    }

    // we need to flip the distribution bit so that gamecards can be installed.
    nca_set_distribution_type(header);

    // now update the nca struct with the actual size of the nca.
    nca.total_size = header->size;

    // re encrypt the header.
    if (!nca_encrypt_header(header, header))
    {
        write_log("Failed to encrypt header\n");
        free(header);
        ncm_delete_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }

    // now write the header to the placeholder.
    if (!ncm_write_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id, 0, header, NCA_XTS_SECTION_SIZE))
    {
        write_log("failed to write to placeholder\n");
        free(header);
        ncm_delete_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }

    // update the struct with the amount of data written.
    nca.offset = offset + NCA_XTS_SECTION_SIZE;
    nca.data_written = NCA_XTS_SECTION_SIZE;

    // free the header as its not needed.
    free(header);

    // allocate memory that will be shared between the read and write threads. (memory is protected).
    nca.data = calloc(1, BUFFER_SIZE);
    if (!nca.data)
    {
        write_log("allocating nca buffer, size is: 0x%X\n", BUFFER_SIZE);
        return false;
    }

    // setup the 2 worker threads.
    mtx_init(&nca_mtx, mtx_plain);
    cnd_init(&can_read);
    cnd_init(&can_write);
    write_log("init mtx and cnd for nca threaded install\n");

    thrd_t t_read = {0};
    thrd_t t_write = {0};

    write_log("creating threads\n");
    thrd_create(&t_read, nca_read, &nca);
    thrd_create(&t_write, nca_write, &nca);
    write_log("created threads\n");

    // TODO: make this into a struct
    // also dont do ui stuff in nca
    // instead, just have a state system for the ui, then do a callback in nca with the progress.
    uint8_t prev_time = 0;
    uint64_t prev_size = 0;
    uint64_t speed = 0;
    uint32_t eta_min = 0;
    uint8_t eta_sec = 0;

    // init the progress bar.
    progress_bar_t *p_bar = ui_init_progress_bar(name, speed, eta_min, eta_sec, nca.data_written, nca.total_size);
    if (!p_bar)
    {
        write_log("Failed to init progress bar!\n");
    }

    // loop until file has finished installing.
    while (nca.data_written != nca.total_size)
    {
        time_t uinx_time = time(NULL);
        struct tm* time_struct = gmtime(&uinx_time);

        if (prev_time != time_struct->tm_sec)
        {
            prev_time = time_struct->tm_sec;
            size_t size = nca.data_written;
            speed = size - prev_size;
            eta_min = ((nca.total_size - size) / speed) / 60;
            eta_sec = ((nca.total_size - size) / speed) % 60;
            prev_size = size;
            ui_update_progress_bar(p_bar, speed, eta_min, eta_sec, nca.data_written, nca.total_size);
        }
        update_button_spin();
        ui_display_progress_bar(p_bar);
        thrd_yield();
    }
    ui_free_progress_bar(p_bar);

    int ret_read = 0;
    int ret_write = 0;

    write_log("Waiting to joing threads\n");
    thrd_join(t_read, &ret_read);
    thrd_join(t_write, &ret_write);
    write_log("joined threads\n");

    mtx_destroy(&nca_mtx);
    cnd_destroy(&can_read);
    cnd_destroy(&can_write);
    free(nca.data);

    // check if the threads were unsuccesful.
    if (ret_read == -1 || ret_write == -1)
    {
        write_log("threading error read: %d write: %d\n", ret_read, ret_write);
        ncm_delete_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }

    // check if the ncas exists, if so, delete it.
    if (ncm_check_if_content_id_exists(&nca.ncm.storage, &nca.ncm.content_id))
    {
        write_log("nca seems to exist already, attempting to delete\n");
        if (!ncm_delete_content_id(&nca.ncm.storage, &nca.ncm.content_id))
        {
            write_log("failed to delete nca\n");
            ncm_delete_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id);
            ncm_close_storage(&nca.ncm.storage);
            return false;
        }
        write_log("deleted nca\n");
    }

    // register the placeholder.
    if (!ncm_register_placeholder_id(&nca.ncm.storage, &nca.ncm.content_id, &nca.ncm.placeholder_id))
    {
        write_log("failed to register placeholder\n");
        ncm_delete_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }
    
    ncm_close_storage(&nca.ncm.storage);
    write_log("finished nca install\n\n");

    return true;
}


/*
*   Debug
*/

void nca_print_header(const NcaHeader_t *header)
{
    #ifdef DEBUG
    if (!header)
    {
        write_log("missing params in nca_print_header\n");
        return;
    }

    write_log("\nnca header print\n");
    write_log("magic: %s\n", nca_get_magic_string(header->magic));
    write_log("title_id: 0%lX\n", header->title_id);
    write_log("context_id: %u\n", header->context_id);
    write_log("header1sig: %u\n", header->header_1_sig_key_gen);
    write_log("size: %lu\n", header->size);
    write_log("sdk_version: %u\n", header->sdk_version);
    write_log("encryption_type: %s\n", nca_has_rights_id(&header->rights_id) ? "titlekey" : "standard crypto");
    write_log("content_type: %s\n", nca_get_content_type_string(header->content_type));
    write_log("distribution_type: %s\n", nca_get_distribution_type_string(header->distribution_type));
    write_log("keak_index: %s\n", nca_get_keak_index_string(header->kaek_index));
    write_log("key_gen: %u (%s)\n\n", header->key_gen ? header->key_gen : header->old_key_gen, nca_return_key_gen_string(header->key_gen ? header->key_gen : header->old_key_gen));
    #endif
}

void nca_print_key_area(const NcaKeyArea_t *keak)
{
    #ifdef DEBUG
    if (!keak)
    {
        write_log("missing params in keak\n");
        return;
    }

    write_log("\nprinting keak\n");
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 0x10; j++)
        {
            write_log("%x", keak[i].area[j]);
        }
        write_log("\n");
    }
    #endif
}
