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
    void *data;
    size_t data_size;
    size_t data_read;
    size_t data_written;
    size_t total_size;
    ncm_install_struct_t ncm;
} thread_t;


bool nca_check_if_magic_valid(uint32_t magic)
{
    switch (magic)
    {
        case NCA0_MAGIC: return true;
        case NCA2_MAGIC: return true;
        case NCA3_MAGIC: return true;
        default:
        {
            write_log("incorrect nca magic. got %u\n", magic);
            return false;
        }
    }
}

const char *nca_get_string_from_id(NcmContentId nca_id, char *out)
{
    uint64_t nca_id_lower = __bswap64(*(uint64_t *)nca_id.c);
    uint64_t nca_id_upper = __bswap64(*(uint64_t *)(nca_id.c + 0x8));
    snprintf(out, 0x21, "%016lx%016lx", nca_id_lower, nca_id_upper);
    return out;
}

NcmContentId nca_get_id_from_string(const char *nca_in_string)
{
    NcmContentId nca_id     = {0};
    char lowerU64[0x11]     = {0};
    char upperU64[0x11]     = {0};
    memcpy(lowerU64, nca_in_string, 16);
    memcpy(upperU64, nca_in_string + 16, 16);
    *(uint64_t *)nca_id.c = __bswap64(strtoul(lowerU64, NULL, 16));
    *(uint64_t *)(nca_id.c + 8) = __bswap64(strtoul(upperU64, NULL, 16));
    return nca_id;
}

const char *nca_return_key_gen_string(uint8_t key_gen)
{
    switch (key_gen)
    {
        // old key gen.
        case NcaOldKeyGeneration_100: return "1.0.0";
        case NcaOldKeyGeneration_300: return "3.0.0";

        // new key gen.
        case NcaKeyGeneration_301: return "3.0.1";
        case NcaKeyGeneration_400: return "4.0.0";
        case NcaKeyGeneration_500: return "5.0.0";
        case NcaKeyGeneration_600: return "6.0.0";
        case NcaKeyGeneration_620: return "6.2.0";
        case NcaKeyGeneration_700: return "7.0.0";
        case NcaKeyGeneration_810: return "8.1.0";
        case NcaKeyGeneration_900: return "9.0.0";
        case NcaKeyGeneration_910: return "9.1.0";

        // unkown.
        default: return "UNKNOWN";
    }
}

uint16_t nca_return_key_gen_int(uint8_t key_gen)
{
    switch (key_gen)
    {
        // old key gen.
        case NcaOldKeyGeneration_100: return 100;
        case NcaOldKeyGeneration_300: return 300;

        // new key gen.
        case NcaKeyGeneration_301: return 301;
        case NcaKeyGeneration_400: return 400;
        case NcaKeyGeneration_500: return 500;
        case NcaKeyGeneration_600: return 600;
        case NcaKeyGeneration_620: return 620;
        case NcaKeyGeneration_700: return 700;
        case NcaKeyGeneration_810: return 810;
        case NcaKeyGeneration_900: return 900;
        case NcaKeyGeneration_910: return 910;

        // unkown.
        default: return 0;
    }
}

void nca_encrypt_header(nca_header_t *header)
{
    crypto_aes_xts(header, header, HEADER_KEY_0, HEADER_KEY_1, 0, NCA_SECTOR_SIZE, NCA_XTS_SECTION_SIZE, EncryptMode_Encrypt);
}

void nca_decrypt_header(nca_header_t *header)
{
    crypto_aes_xts(header, header, HEADER_KEY_0, HEADER_KEY_1, 0, NCA_SECTOR_SIZE, NCA_XTS_SECTION_SIZE, EncryptMode_Decrypt);
}

bool nca_get_header(FILE *fp, uint64_t offset, nca_header_t *header)
{
    if (!fp || !header)
        return false;

    return fread(header, 1, NCA_XTS_SECTION_SIZE, fp) == NCA_XTS_SECTION_SIZE ? true : false;
}

bool nca_get_header_decrypted(FILE *fp, uint64_t offset, nca_header_t *header)
{
    bool ret = nca_get_header(fp, offset, header);
    if (!ret)
        return ret;

    nca_decrypt_header(header);
    return true;
}

void nca_set_distribution_type(nca_header_t *header)
{
    header->distribution_type = NcaDistributionType_System;
}



/*
*   nca install stuff.
*/

int nca_read(void *in)
{
    thread_t *t = (thread_t *)in;
    if (!t)
        return -1;

    void *buf = calloc(1, BUFFER_SIZE);
    if (!buf)
        return -1;

    for (uint64_t done = t->data_written, bufsize = BUFFER_SIZE; appletMainLoop() && done < t->total_size; done += bufsize)
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
    thread_t *t = (thread_t *)in;
    if (!t)
        return -1;

    while (appletMainLoop() && t->data_written < t->total_size)
    {
        mtx_lock(&nca_mtx);
        if (t->data_size == 0)
        {
            cnd_wait(&can_write, &nca_mtx);
        }

        ncm_write_placeholder(&t->ncm.storage, &t->ncm.placeholder_id, t->data_written, t->data, t->data_size);
        t->data_written += t->data_size;
        t->data_size = 0;

        mtx_unlock(&nca_mtx);
        cnd_signal(&can_read);
    }

    return 0;
}

bool nca_setup_placeholder(ncm_install_struct_t *out, size_t size, NcmContentId *content_id, NcmStorageId storage_id)
{
    // open ncm storage.
    if (R_FAILED(ncm_open_storage(&out->storage, storage_id)))
        return false;

    // generate a placeholder id.
    if (R_FAILED(ncm_generate_placeholder_id(&out->storage, &out->placeholder_id)))
    {
        ncm_close_storage(&out->storage);
        return false;
    }

    // check if the placeholder exists, delete if found.
    if (ncm_check_if_placeholder_exists(&out->storage, &out->placeholder_id))
        ncm_delete_placeholder(&out->storage, &out->placeholder_id);

    // finally create placeholder.
    if (R_FAILED(ncm_create_placeholder(&out->storage, memcpy(&out->content_id, content_id, sizeof(NcmContentId)), &out->placeholder_id, size)))
    {
        ncm_close_storage(&out->storage);
        return false;
    }
    
    return true;
}

FILE *nca_try_open_file(NcmContentId content_id, char *out_string, const char *mode)
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

bool nca_decrypt_keak(nca_header_t *header, nca_key_area_t *out)
{
    if (!header || !out)
    {
        write_log("NULL args in decrypt key area func\n");
        return false;
    }
   
    // decrypt to get the key.
    nca_key_area_t decrypted_nca_keys[NCA_SECTION_TOTAL] = {0};
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

    // copy decrypted key.
    memcpy(out, &decrypted_nca_keys[0x2], sizeof(nca_key_area_t));
    return true;
}

bool nca_encrypt_keak(nca_header_t *header, const nca_key_area_t *decrypted_key, uint8_t key_gen)
{
    if (!header || !decrypted_key || !has_key_gen(key_gen))
    {
        write_log("NULL args in encrypt key area func\n");
        return false;
    }

    Aes128Context ctx = {0};
    nca_key_area_t decrypted_nca_keys[NCA_SECTION_TOTAL] = {0};
    memcpy(&decrypted_nca_keys[0x2], decrypted_key, sizeof(nca_key_area_t));

    const uint8_t *keak = get_keak(key_gen);
    if (!keak) return false;

    aes128ContextCreate(&ctx, keak, true);
    for (uint8_t i = 0; i < NCA_SECTION_TOTAL; i++)
    {
        aes128EncryptBlock(&ctx, &header->key_area[i], &decrypted_nca_keys[i]);
    }

    header->key_gen = key_gen;
    header->old_key_gen = key_gen;

    return true;
}

#include "nx/input.h"
bool nca_start_install(NcmContentId content_id, NcmStorageId storage_id)
{
    // this will be the thread that is passed to the read / write.
    thread_t nca = {0};

    // first open the nca file to install.
    char nca_string[0x21] = {0};
    nca.fp = nca_try_open_file(content_id, nca_string, "rb");
    if (!nca.fp)
    {
        write_log("failed to open nca file... %s\n", nca_string);
        return false;
    }

    // next we need to get the header.
    nca_header_t *header = calloc(1, NCA_XTS_SECTION_SIZE);
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

    nca_key_area_t keak = {0};
    if (is_lower_key_gen_enabled())
    {
        nca_decrypt_keak(header, &keak);
        nca_encrypt_keak(header, &keak, 0);
    }
    
    // now that we have the actual size of the nca, we can setup the placeholder.
    if (!nca_setup_placeholder(&nca.ncm, header->size, &content_id, storage_id))
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

    // re encrypt the header.
    nca_encrypt_header(header);

    // now write the header to the placeholder.
    if (R_FAILED(ncm_write_placeholder(&nca.ncm.storage, &nca.ncm.placeholder_id, 0, header, NCA_XTS_SECTION_SIZE)))
    {
        write_log("failed to setup placeholder\n");
        fclose(nca.fp);
        free(header);
        ncm_delete_placeholder(&nca.ncm.storage, &nca.ncm.placeholder_id);
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
        fclose(nca.fp);
        return false;
    }

    // setup the 2 worker threads.
    mtx_init(&nca_mtx, mtx_plain);
    cnd_init(&can_read);
    cnd_init(&can_write);

    thrd_t t_read = {0};
    thrd_t t_write = {0};

    thrd_create(&t_read, nca_read, &nca);
    thrd_create(&t_write, nca_write, &nca);

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

    // loop until file has finished installing.
    while (appletMainLoop() && nca.data_written != nca.total_size)
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
    if (!appletMainLoop())
    {
        cnd_signal(&can_read);
        cnd_signal(&can_write);
        mtx_unlock(&nca_mtx);
    }
    ui_free_progress_bar(p_bar);

    int ret_read = 0;
    int ret_write = 0;
    thrd_join(t_read, &ret_read);
    thrd_join(t_write, &ret_write);
    mtx_destroy(&nca_mtx);
    cnd_destroy(&can_read);
    cnd_destroy(&can_write);
    free(nca.data);
    fclose(nca.fp);

    // check if the threads were unsuccesful.
    if (ret_read == -1 || ret_write == -1)
    {
        write_log("threading error read: %d write: %d\n", ret_read, ret_write);
        ncm_delete_placeholder(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }

    // check if the ncas exists, if so, delete it.
    if (ncm_check_if_nca_exists(&nca.ncm.storage, &nca.ncm.content_id))
    {
        if (R_FAILED(ncm_delete_nca(&nca.ncm.storage, &nca.ncm.content_id)))
        {
            write_log("failed to delete nca\n");
            ncm_delete_placeholder(&nca.ncm.storage, &nca.ncm.placeholder_id);
            ncm_close_storage(&nca.ncm.storage);
            return false;
        }
    }

    // register the placeholder.
    if (R_FAILED(ncm_register_placeholder(&nca.ncm.storage, &nca.ncm.content_id, &nca.ncm.placeholder_id)))
    {
        write_log("failed to register placeholder\n");
        ncm_delete_placeholder(&nca.ncm.storage, &nca.ncm.placeholder_id);
        ncm_close_storage(&nca.ncm.storage);
        return false;
    }
    
    ncm_close_storage(&nca.ncm.storage);
    return true;
}