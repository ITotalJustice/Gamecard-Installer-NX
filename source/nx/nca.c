#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <switch.h>

#include "nx/nca.h"
#include "nx/ncm.h"
#include "nx/crypto.h"

#include "ui/menu.h"

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
    size_t data_size;
    size_t data_read;
    size_t data_written;
    size_t total_size;
    NcmInstall_t ncm;
} NcaInstall_t;


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

const char *nca_get_string_from_id(NcmContentId *nca_id, char *out)
{
    uint64_t nca_id_lower = __bswap64(*(uint64_t *)nca_id->c);
    uint64_t nca_id_upper = __bswap64(*(uint64_t *)(nca_id->c + 0x8));
    snprintf(out, 0x30, "%016lx%016lx", nca_id_lower, nca_id_upper);
    return out;
}

NcmContentId nca_get_id_from_string(const char *nca_in_string)
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
        write_log("empty params in nca_encrypt_header\n");
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
        write_log("empty params in nca_decrypt_header\n");
        return false;
    }

    if (!crypto_aes_xts(in_header, out_header, HEADER_KEY_0, HEADER_KEY_1, 0, NCA_SECTOR_SIZE, NCA_XTS_SECTION_SIZE, EncryptMode_Decrypt))
    {
        return false;
    }

    return true;
}

bool nca_get_header(FILE *fp, uint64_t offset, NcaHeader_t *header)
{
    if (!fp || !header)
    {
        write_log("empty params for nca get header\n");
        return false;
    }

    return fread(header, 1, NCA_XTS_SECTION_SIZE, fp) == NCA_XTS_SECTION_SIZE;
}

bool nca_get_header_decrypted(FILE *fp, uint64_t offset, NcaHeader_t *header)
{
    if (!fp || !header)
    {
        write_log("empty params for nca get header decrypted\n");
        return false;
    }

    if (!nca_get_header(fp, offset, header))
    {
        return false;
    }

    return nca_decrypt_header(header, header);
}

bool nca_has_rights_id(const RightsId_t *rights_id)
{
    if (!rights_id)
    {
        write_log("empty param in nca_has_rights_id\n");
        return false;
    }

    for (uint8_t i = 0; i < 0x10; i++)
    {
        if (rights_id->rights_id[i])
        {
            return true;
        }
    }
    return false;
}

void nca_set_distribution_type(NcaHeader_t *header)
{
    if (!header)
    {
        write_log("empty param in nca_set_distribution type\n");
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
        return -1;
    }

    void *buf = calloc(1, BUFFER_SIZE);
    if (!buf)
    {
        return -1;
    }

    for (uint64_t done = t->data_written, bufsize = BUFFER_SIZE; done < t->total_size; done += bufsize)
    {
        if (done + bufsize > t->total_size)
            bufsize = t->total_size - done;

        read_file(buf, bufsize, done, t->fp);

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

bool nca_setup_placeholder_id(NcmInstall_t *out, size_t size, NcmContentId *content_id, NcmStorageId storage_id)
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

FILE *nca_try_open_file(NcmContentId *content_id, char *out_string, const char *mode)
{
    FILE *fp = open_file2(mode, "%s%s", nca_get_string_from_id(content_id, out_string), ".nca");
    if (!fp)
    {
        fp = open_file2(mode, "%s%s", nca_get_string_from_id(content_id, out_string), ".cnmt.nca");
        if (!fp)
        {
            fp = open_file2(mode, "%s%s", nca_get_string_from_id(content_id, out_string), ".ncz");
            if (!fp)
            {
                return NULL;
            }
        }
    }
    return fp;
}

bool nca_decrypt_keak(NcaHeader_t *header, NcaKeyArea_t *out)
{
    if (!header || !out)
    {
        write_log("NULL args in decrypt key area func\n");
        return false;
    }
   
    // decrypt to get the key.
    NcaKeyArea_t decrypted_nca_keys[NCA_SECTION_TOTAL] = {0};
    Aes128Context ctx_dec = {0};

    // sort out the key gen.
    uint8_t key_gen = header->key_gen ? header->key_gen : header->old_key_gen;
    if (key_gen) key_gen--;

    const uint8_t *keak = get_keak(key_gen);
    if (!keak) return false;

    aes128ContextCreate(&ctx_dec, keak, false);
    for (uint8_t i = 0; i < NCA_SECTION_TOTAL; i++)
    {
        aes128DecryptBlock(&ctx_dec, &decrypted_nca_keys[i], &header->key_area[i]);
    }

    write_log("\nencrypted keys\n");
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 0x10; j++)
        {
            write_log("%x", header->key_area[i].area[j]);
        }
        write_log("\n");
    }

    write_log("\ndecrypting keys\n");
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 0x10; j++)
        {
            write_log("%x", decrypted_nca_keys[i].area[j]);
        }
        write_log("\n");
    }

    // copy decrypted key.
    memcpy(out, &decrypted_nca_keys[0x2], sizeof(NcaKeyArea_t));
    return true;
}

bool nca_encrypt_keak(NcaHeader_t *header, const NcaKeyArea_t *decrypted_key, uint8_t key_gen)
{
    if (!header || !decrypted_key || !has_key_gen(key_gen))
    {
        write_log("NULL args in encrypt key area func\n");
        return false;
    }

    Aes128Context ctx = {0};
    NcaKeyArea_t decrypted_nca_keys[NCA_SECTION_TOTAL] = {0};
    memcpy(&decrypted_nca_keys[0x2], decrypted_key, sizeof(NcaKeyArea_t));

    const uint8_t *keak = get_keak(key_gen ? key_gen-1 : 0);
    if (!keak) return false;

    aes128ContextCreate(&ctx, keak, true);
    for (uint8_t i = 0; i < NCA_SECTION_TOTAL; i++)
    {
        aes128EncryptBlock(&ctx, &header->key_area[i], &decrypted_nca_keys[i]);
    }

    write_log("\nre-encrypted keys\n");
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 0x10; j++)
        {
            write_log("%x", header->key_area[i].area[j]);
        }
        write_log("\n\n");
    }

    header->key_gen = key_gen;
    header->old_key_gen = key_gen;

    return true;
}

bool nca_start_install(NcmContentId *content_id, NcmStorageId storage_id)
{
    write_log("starting to install nca to storage: %s\n", ncm_get_storage_id_string(storage_id));

    // this will be the thread that is passed to the read / write.
    NcaInstall_t nca = {0};

    // first open the nca file to install.
    char nca_string[0x30] = {0};
    nca.fp = nca_try_open_file(content_id, nca_string, "rb");
    if (!nca.fp)
    {
        write_log("failed to open nca file... %s\n", nca_string);
        return false;
    }

    // next we need to get the header.
    NcaHeader_t *header = calloc(1, NCA_XTS_SECTION_SIZE);
    if (!header)
    {
        write_log("failed to malloc header\n");
        fclose(nca.fp);
        return false;
    }

    if (!nca_get_header_decrypted(nca.fp, 0, header))
    {
        write_log("failed to get nca header\n");
        fclose(nca.fp);
        free(header);
        return false;
    }

    NcaKeyArea_t keak = {0};
    if (is_lower_key_gen_enabled())
    {
        write_log("lower keygen was enabled, time to get keys!\n");
        if (!nca_decrypt_keak(header, &keak))
        {
            fclose(nca.fp);
            return false;
        }
        if (!nca_encrypt_keak(header, &keak, 0))
        {
            fclose(nca.fp);
            return false;
        }
    }
    
    // now that we have the actual size of the nca, we can setup the placeholder.
    if (!nca_setup_placeholder_id(&nca.ncm, header->size, content_id, storage_id))
    {
        write_log("failed to setup placeholder\n");
        fclose(nca.fp);
        free(header);
        return false;
    }

    // we need to flip the distribution bit so that gamecards can be installed.
    nca_set_distribution_type(header);

    // now update the nca struct with the actual size of the nca.
    nca.total_size = header->size;
    write_log("Got nca size from header %lu\n", nca.total_size);

    // re encrypt the header.
    if (!nca_encrypt_header(header, header))
    {
        write_log("Failed to encrypt header\n");
        fclose(nca.fp);
        free(header);
        ncm_delete_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }

    // now write the header to the placeholder.
    if (!ncm_write_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id, 0, header, NCA_XTS_SECTION_SIZE))
    {
        write_log("failed to write to placeholder\n");
        fclose(nca.fp);
        free(header);
        ncm_delete_placeholder_id(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }

    // update the struct with the amount of data written.
    nca.data_written = NCA_XTS_SECTION_SIZE;

    // free the header as its not needed.
    free(header);

    // allocate memory that will be shared between the read and write threads. (memory is protected).
    nca.data = calloc(1, BUFFER_SIZE);
    if (!nca.data)
    {
        write_log("allocating nca buffer, size is: 0x%X\n", BUFFER_SIZE);
        fclose(nca.fp);
        return false;
    }

    // setup the 2 worker threads.
    mtx_init(&nca_mtx, mtx_plain);
    cnd_init(&can_read);
    cnd_init(&can_write);
    write_log("Init mtx and cnd\n");

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
    progress_bar_t *p_bar = ui_init_progress_bar(nca_string, speed, eta_min, eta_sec, nca.data_written, nca.total_size);
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
    write_log("Joined\n");

    write_log("destorying mtx and cnd\n");
    mtx_destroy(&nca_mtx);
    cnd_destroy(&can_read);
    cnd_destroy(&can_write);
    write_log("destroyed\n");

    write_log("freeing nca data buffer\n");
    free(nca.data);
    write_log("freed\n");

    write_log("closing nca file\n");
    fclose(nca.fp);
    write_log("closed\n");

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
    
    write_log("closing ncm storage\n");
    ncm_close_storage(&nca.ncm.storage);
    write_log("closed\n");

    return true;
}