/*
*   This code is bad.
*   Don't read it.
*   It works, that's all that matters.
*
*   I'll clean it up one day. Today is not that day.
*/


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "ui/gc.h"
#include "ui/menu.h"
#include "ui/settings.h"

#include "nx/fs.h"
#include "nx/ns.h"
#include "nx/nca.h"
#include "nx/ncm.h"
#include "nx/cnmt.h"
#include "nx/set.h"
#include "nx/crypto.h"
#include "nx/es.h"

#include "gfx/image.h"
#include "gfx/text.h"

#include "util/dir.h"
#include "util/file.h"
#include "util/util.h"
#include "util/error.h"
#include "util/log.h"


typedef struct
{
    uint64_t id;

    uint16_t base_total;
    uint16_t upp_total;
    uint16_t dlc_total;
} sort_t;

bool g_gamecard_mounted = false;
uint16_t g_game_pos = 0;
GameCard_t GAMECARD = {0};

FsDeviceOperator g_dop = {0};
FsGameCardHandle g_gc_handle = {0};


bool init_gc(void)
{
    if (!fs_open_device_operator(&g_dop))
    {
        write_log("failed to mount gcop\n");
        ui_display_error_box(ErrorCode_Init_Gc, __func__);
        return false;
    }
    return true;
}

void exit_gc(void)
{
    gc_unmount();
    fs_close_device_operator(&g_dop);
}

bool gc_poll(void)
{
    return fs_is_gamecard_inserted(&g_dop);
}


/*
*   GameCard Mount / Unmount.
*/

bool __gc_setup_file_tabel(GameCard_t *gamecard)
{
    if (!gamecard)
    {
        write_log("missing args in %s\n", __func__);
        return false;
    }

    DIR *dir = opendir(".");
    struct dirent *d = {0};
    if (!dir)
    {
        write_log("failed to open folder %s\n", __func__);
        return false;
    }

    memset(&gamecard->file_table, 0, sizeof(GameCardFileTable_t));
    
    while ((d = readdir(dir)))
    {
        char *found = strchr(d->d_name, '.');
        if (strcmp(found, ".cnmt.nca") == 0)
            gamecard->file_table.cnmt_count++;
        else if (strcmp(found, ".nca") == 0)
            gamecard->file_table.nca_count++;
        else if (strcmp(found, ".tik") == 0)
            gamecard->file_table.tik_count++;
        else if (strcmp(found, ".cert") == 0)
            gamecard->file_table.cert_count++;
        gamecard->file_table.file_count++;
    }
    closedir(dir);
    return true;
}

bool __gc_setup_string_table(GameCard_t *gamecard)
{
    if (!gamecard)
    {
        write_log("missing args in %s\n", __func__);
        return false;
    }

    gamecard->string_table = calloc(gamecard->file_table.file_count, sizeof(GameCardStringTable_t));
    if (!gamecard->string_table)
    {
        write_log("no file table");
        return false;
    }

    DIR *dir = opendir(".");
    struct dirent *d = {0};
    if (!dir)
    {
        write_log("failed to open folder %s\n", __func__);
        ui_display_error_box(ErrorCode_OpenDir, __func__);
        return false;
    }

    uint16_t i = 0;
    while ((d = readdir(dir)) && i < gamecard->file_table.file_count)
    {
        strncpy(gamecard->string_table[i].name, d->d_name, 0x30);
        write_log("table %s\n", gamecard->string_table[i].name);
        i++; 
    }

    closedir(dir);

    return true;
}

bool __id_match_check(sort_t *sorts, uint16_t count, uint16_t *pos, uint64_t id, NcmContentMetaType type)
{
    for (uint16_t i = 0; i < count; i++)
    {
        if (sorts[i].id == ncm_get_app_id_from_title_id(id, type))
        {
            *pos = i;
            return true;
        }
    }
    return false;
}


bool __idk_what_to_call_this(sort_t *sorts, uint16_t pos, NcmContentMetaType type)
{
    if (!sorts)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    switch (type)
    {
        case NcmContentMetaType_Application:
            sorts[pos].base_total++;
            return true;
        case NcmContentMetaType_Patch:
            sorts[pos].upp_total++;
            return true;
        case NcmContentMetaType_AddOnContent:
            sorts[pos].dlc_total++;
            return true;
        default:
            write_log("got incorrect ncm_meta_type: %u\n", type);
            return false;
    }
}

bool __gc_setup_entry(GameCardGameEntries_t *entries, const Cnmt_t *cnmt, uint8_t key_gen, uint16_t pos, NcmContentMetaType type)
{
    if (!entries || !cnmt)
    {
        write_log("missing params in __gc_sort_cnmt\n");
        return false;
    }

    switch (type)
    {
        case NcmContentMetaType_Application:
            memcpy(&GAMECARD.entries[pos].base[GAMECARD.entries[pos].base_count].cnmt, cnmt, sizeof(Cnmt_t));
            GAMECARD.entries[pos].base[GAMECARD.entries[pos].base_count].size = ncm_calculate_content_infos_size(GAMECARD.entries[pos].base[GAMECARD.entries[pos].base_count].cnmt.content_infos, GAMECARD.entries[pos].base[GAMECARD.entries[pos].base_count].cnmt.header.content_count);
            GAMECARD.entries[pos].base_size += GAMECARD.entries[pos].base[GAMECARD.entries[pos].base_count].size;
            GAMECARD.entries[pos].total_size += GAMECARD.entries[pos].base[GAMECARD.entries[pos].base_count].size;
            GAMECARD.entries[pos].base[GAMECARD.entries[pos].base_count].key_gen = key_gen;
            GAMECARD.entries[pos].base_count++;
            break;
        case NcmContentMetaType_Patch:
            memcpy(&GAMECARD.entries[pos].upp[GAMECARD.entries[pos].upp_count].cnmt, cnmt, sizeof(Cnmt_t));
            GAMECARD.entries[pos].upp[GAMECARD.entries[pos].base_count].size = ncm_calculate_content_infos_size(GAMECARD.entries[pos].upp[GAMECARD.entries[pos].upp_count].cnmt.content_infos, GAMECARD.entries[pos].upp[GAMECARD.entries[pos].upp_count].cnmt.header.content_count);
            GAMECARD.entries[pos].upp_size += GAMECARD.entries[pos].upp[GAMECARD.entries[pos].upp_count].size;
            GAMECARD.entries[pos].total_size += GAMECARD.entries[pos].upp[GAMECARD.entries[pos].upp_count].size;
            GAMECARD.entries[pos].upp[GAMECARD.entries[pos].upp_count].key_gen = key_gen;
            GAMECARD.entries[pos].upp_count++;
            break;
        case NcmContentMetaType_AddOnContent:
            memcpy(&GAMECARD.entries[pos].dlc[GAMECARD.entries[pos].dlc_count].cnmt, cnmt, sizeof(Cnmt_t));
            GAMECARD.entries[pos].dlc[GAMECARD.entries[pos].dlc_count].size = ncm_calculate_content_infos_size(GAMECARD.entries[pos].dlc[GAMECARD.entries[pos].dlc_count].cnmt.content_infos, GAMECARD.entries[pos].dlc[GAMECARD.entries[pos].dlc_count].cnmt.header.content_count);
            GAMECARD.entries[pos].dlc_size += GAMECARD.entries[pos].dlc[GAMECARD.entries[pos].dlc_count].size;
            GAMECARD.entries[pos].total_size += GAMECARD.entries[pos].dlc[GAMECARD.entries[pos].dlc_count].size;
            GAMECARD.entries[pos].dlc[GAMECARD.entries[pos].dlc_count].key_gen = key_gen;
            GAMECARD.entries[pos].dlc_count++;
            break;
        default:
            write_log("got incorrect ncm_meta_type: %u\n", type);
            return false;
    }

    GAMECARD.entries[pos].total_count++;
    return true;
}

bool __gc_parse_cnmt(void)
{
    // get the total cnmt.nca.
    if (!__gc_setup_file_tabel(&GAMECARD))
    {
        //TODO
        return false;
    }

    if (!GAMECARD.file_table.cnmt_count)
    {
        write_log("no cnmt's found!? How on earth is that possible\n");
        ui_display_error_box(ErrorCode_Mount_NoMeta, __func__);
        return false;
    }

    if (!__gc_setup_string_table(&GAMECARD))
    {
        //TODO
        return false;
    }


    Cnmt_t *cnmt = calloc(GAMECARD.file_table.cnmt_count, sizeof(Cnmt_t));
    if (!cnmt)
    {
        write_log("failed to alloc cnmt\n");
        ui_display_error_box(ErrorCode_Alloc, __func__);
        return false;
    }

    sort_t *sorts = calloc(GAMECARD.file_table.cnmt_count, sizeof(sort_t));
    if (!sorts)
    {
        write_log("failed to alloc sorts\n");
        free(cnmt);
        ui_display_error_box(ErrorCode_Alloc, __func__);
        return false;
    }

    uint8_t *key_gens = calloc(GAMECARD.file_table.cnmt_count, sizeof(uint8_t));
    if (!key_gens)
    {
        write_log("failed to alloc key_gens\n");
        free(sorts);
        free(cnmt);
        ui_display_error_box(ErrorCode_Alloc, __func__);
        return false;
    }

    // lets get each cnmt and parse it.
    for (uint16_t i = 0, j = 0; i < GAMECARD.file_table.file_count && j < GAMECARD.file_table.cnmt_count; i++)
    {
        // we only want cnmt.nca.
        if (!strstr(GAMECARD.string_table[i].name, "cnmt.nca"))
        {
            continue;
        }

        // open cnmt.nca
        FILE *fp = fopen(GAMECARD.string_table[i].name, "rb");
        if (!fp)
        {
            write_log("failed to open %s\n", GAMECARD.string_table[i].name);
            free(key_gens);
            free(cnmt);
            free(sorts);
            ui_display_error_box(ErrorCode_OpenFile, __func__);
            return false;
        }

        // get the decrypted header.
        NcaHeader_t header = {0};
        if (!nca_get_header_decrypted(fp, 0, &header))
        {
            free(key_gens);
            free(cnmt);
            free(sorts);
            fclose(fp);
            ui_display_error_box(ErrorCode_DecryptNcaHeader, __func__);
            return false;
        }
        nca_print_header(&header);

        // sort keygen.
        key_gens[j] = header.key_gen ? header.key_gen : header.old_key_gen;

        // check if we can decrypt the keak.
        if (!crypto_has_key_gen(header.kaek_index, key_gens[j]))
        {
            free(key_gens);
            free(cnmt);
            free(sorts);
            fclose(fp);
            ui_display_error_box(ErrorCode_KeyGen, __func__);
        }

        // decrypt the keak. (cnmt.nca are always standard crypto).
        NcaKeyArea_t decrypted_key = {0};
        if (!nca_decrypt_keak(&header, &decrypted_key))
        {
            write_log("failed to decrypt stuff\n");
            free(key_gens);
            free(cnmt);
            free(sorts);
            fclose(fp);
            ui_display_error_box(ErrorCode_DecryptNcaKeak, __func__);
            return false;
        }

        // ensure that the first cnmt section is pfs0 (should always be true).
        if (header.section_header[0].fs_type != NcaFileSystemType_PFS0)
        {
            write_log("cnmt section 0 is not a pfs0!\n");
            free(key_gens);
            free(cnmt);
            free(sorts);
            fclose(fp);
    
            ui_display_error_box(ErrorCode_WrongFsType, __func__);
            return false;
        }

        // calc the offset.
        uint64_t section_offset = MEDIA_REAL(header.section_table[0].media_start_offset);

        // read section (pfs0 is always the first and only(?) section for a cnmt.nca)
        void *pfs0_data = calloc(1, header.section_header[0].pfs0_sb.pfs0_size);
        if (!pfs0_data)
        {
            write_log("failed to alloc pfs0_data\n");
            free(key_gens);
            free(cnmt);
            free(sorts);
            fclose(fp);
    
            ui_display_error_box(ErrorCode_Alloc, __func__);
            return false;
        }

        read_file(pfs0_data, header.section_header[0].pfs0_sb.pfs0_size, (section_offset + header.section_header[0].pfs0_sb.pfs0_offset), fp);
        fclose(fp);

        // decrypt section
        uint8_t ctr[0x10] = {0};
        crypto_aes_ctr(pfs0_data, pfs0_data, decrypted_key.area, ctr, header.section_header[0].pfs0_sb.pfs0_size, section_offset + header.section_header[0].pfs0_sb.pfs0_offset);

        // using fmemopen because i havent yet re-wrote the pfs0 section yet (works fine anyway).
        FILE *pfs0_fp = fmemopen(pfs0_data, header.section_header[0].pfs0_sb.pfs0_size, "r");
        if (!pfs0_fp)
        {
            write_log("failed to open cnmt data stream as a file\n");
            free(key_gens);
            free(cnmt);
            free(sorts);
            ui_display_error_box(ErrorCode_OpenFile, __func__);
            return false;
        }

        pfs0_struct_ptr ptr = {0};
        if (!pfs0_process(&ptr, 0, pfs0_fp))
        {
            write_log("failed to process pfs0\n");
            free(key_gens);
            free(cnmt);
            free(sorts);
            fclose(pfs0_fp);
            free(pfs0_data);
    
            ui_display_error_box(ErrorCode_Pfs0Prase, __func__);
            return false;
        }

        fclose(pfs0_fp);

        if (ptr.header.total_files > 1)
        {
            write_log("WARNING: More than one file found in cnmt pfs0\n");
        }

        NcmContentInfo cnmt_info = {0};
        memcpy(cnmt_info.size, &header.size, 0x6);
        cnmt_info.content_id = nca_get_id_from_string(GAMECARD.string_table[i].name);
        cnmt_info.content_type = NcmContentType_Meta;
        cnmt_info.id_offset = 0;

        if (!cnmt_parse(pfs0_data, ptr.raw_data_offset + ptr.file_table[0].data_offset, &cnmt_info, &cnmt[j]))
        {
            write_log("failed to parse cnmt\n");
            free(key_gens);
            free(cnmt);
            free(sorts);
            free(pfs0_data);
    
            ui_display_error_box(ErrorCode_CnmtPrase, __func__);
            return false;
        }

        // add stuff to sorts
        uint16_t pos = 0;
        if (__id_match_check(sorts, GAMECARD.file_table.game_count, &pos, cnmt[j].key.id, cnmt[j].key.type))
        {
            __idk_what_to_call_this(sorts, pos, cnmt[j].key.type);
        }
        else
        {
            __idk_what_to_call_this(sorts, GAMECARD.file_table.game_count, cnmt[j].key.type);
            sorts[GAMECARD.file_table.game_count].id = ncm_get_app_id_from_title_id(cnmt[j].key.id, cnmt[j].key.type);
            GAMECARD.file_table.game_count++;
        }

        pfs0_free_structs(&ptr);
        free(pfs0_data);
        j++;
    }

    GAMECARD.entries = calloc(GAMECARD.file_table.game_count, sizeof(GameCardGameEntries_t));
    if (!GAMECARD.entries)
    {
        ui_display_error_box(ErrorCode_Alloc, __func__);
        write_log("failed to alloc entries\n");
        free(key_gens);
        free(sorts);
        free(cnmt);
        return false;
    }

    for (uint16_t i = 0; i < GAMECARD.file_table.game_count; i++)
    {
        if (sorts[i].base_total)
        {
            GAMECARD.entries[i].base = calloc(sorts[i].base_total, sizeof(GameCardEntry_t));
            if (!GAMECARD.entries[i].base)
            {
                ui_display_error_box(ErrorCode_Alloc, __func__);
                write_log("failed to alloc entry base: %u\n", i);
                free(key_gens);
                free(sorts);
                free(cnmt);
                return false;
            }
        }
        if (sorts[i].upp_total)
        {
            GAMECARD.entries[i].upp = calloc(sorts[i].upp_total, sizeof(GameCardEntry_t));
            if (!GAMECARD.entries[i].upp)
            {
                ui_display_error_box(ErrorCode_Alloc, __func__);
                write_log("failed to alloc entry upp: %u\n", i);
                free(key_gens);
                free(sorts);
                free(cnmt);
                return false;
            }
        }
        if (sorts[i].dlc_total)
        {
            GAMECARD.entries[i].dlc = calloc(sorts[i].dlc_total, sizeof(GameCardEntry_t));
            if (!GAMECARD.entries[i].dlc)
            {
                ui_display_error_box(ErrorCode_Alloc, __func__);
                write_log("failed to alloc entry dlc: %u\n", i);
                free(key_gens);
                free(sorts);
                free(cnmt);
                return false;
            }
        }
    }

    for (uint16_t i = 0; i < GAMECARD.file_table.cnmt_count; i++)
    {
        uint16_t pos = 0;
        if (!__id_match_check(sorts, GAMECARD.file_table.game_count, &pos, cnmt[i].key.id, cnmt[i].key.type))
        {
            write_log("\n\nWARNING. __id_match_check failed: %u\n\n", i);
        }
        __gc_setup_entry(GAMECARD.entries, &cnmt[i], key_gens[i], pos, cnmt[i].key.type);
    }

    free(key_gens);
    free(sorts);
    free(cnmt);

    return true;
}

bool gc_mount(void)
{
    if (!fs_get_gamecard_handle_from_device_operator(&g_dop, &g_gc_handle))
    {
        write_log("failed to get gc handle\n");
        ui_display_error_box(ErrorCode_Mount_Handle, __func__);
        return false;
    }

    if (!fs_mount_gamecard_secure(&g_gc_handle))
    {
        write_log("failed to mount gc\n");
        ui_display_error_box(ErrorCode_Mount_Secure, __func__);
        return false;
    }

    if (!change_dir("%s%s", GAMECARD_MOUNT_SECURE, ":/"))
    {
        write_log("failed to change path to gc\n");
        ui_display_error_box(ErrorCode_Mount_Chdir, __func__);
        fs_unmount_device(GAMECARD_MOUNT_SECURE);
        return false;
    }

    return __gc_parse_cnmt();
}

bool gc_unmount(void)
{
    fs_unmount_device(GAMECARD_MOUNT_SECURE);
    fs_close_gamecard_handle(&g_gc_handle);

    if (GAMECARD.entries)
    {
        for (uint16_t i = 0; i < GAMECARD.file_table.game_count; i++)
        {
            if (&GAMECARD.entries[i])
            {
                if (GAMECARD.entries[i].base_count && GAMECARD.entries[i].base)
                {
                    free(GAMECARD.entries[i].base);
                    GAMECARD.entries[i].base = NULL;
                }
                if (GAMECARD.entries[i].upp_count && GAMECARD.entries[i].upp)
                {
                    free(GAMECARD.entries[i].upp);
                    GAMECARD.entries[i].upp = NULL;
                }
                if (GAMECARD.entries[i].dlc_count && GAMECARD.entries[i].dlc)
                {
                    free(GAMECARD.entries[i].dlc);
                    GAMECARD.entries[i].dlc = NULL;
                }
            }
        }
        free(GAMECARD.entries);
        GAMECARD.entries = NULL;
    }
    if (GAMECARD.string_table)
    {
        free(GAMECARD.string_table);
        GAMECARD.string_table = NULL;
    }

    memset(&GAMECARD, 0, sizeof(GameCard_t));
    g_game_pos = 0;
    change_dir("sdmc:/");
    return true;
}


/*
*   GameCard Getters.
*/

uint16_t gc_get_game_count(void)
{
    return GAMECARD.file_table.game_count;
}

uint16_t gc_get_base_count(uint16_t game_pos)
{
    return GAMECARD.entries[game_pos].base_count;
}

uint16_t gc_get_upp_count(uint16_t game_pos)
{
    return GAMECARD.entries[game_pos].upp_count;
}

uint16_t gc_get_dlc_count(uint16_t game_pos)
{
    return GAMECARD.entries[game_pos].dlc_count;
}

uint16_t gc_get_current_base_count(void)
{
    return GAMECARD.entries[g_game_pos].base_count;
}

uint16_t gc_get_current_upp_count(void)
{
    return GAMECARD.entries[g_game_pos].upp_count;
}

uint16_t gc_get_current_dlc_count(void)
{
    return GAMECARD.entries[g_game_pos].dlc_count;
}


/*
*   Change GameInfo.
*/

const GameCardEntry_t *__gc_get_gamecard_entry(uint16_t game_pos, uint16_t entry_pos)
{
    // ensure that we have a game in pos and a game-entry.
    if (game_pos >= GAMECARD.file_table.game_count || entry_pos >= GAMECARD.entries[game_pos].total_count)
    {
        write_log("missing params in %s\n", __func__);
        return NULL;
    }

    if (entry_pos < GAMECARD.entries[game_pos].base_count)
    {
        return &GAMECARD.entries[game_pos].base[entry_pos];
    }
    else if (entry_pos < GAMECARD.entries[game_pos].base_count + GAMECARD.entries[game_pos].upp_count)
    {
        return &GAMECARD.entries[game_pos].upp[entry_pos - GAMECARD.entries[game_pos].base_count];
    }
    else
    {
        return &GAMECARD.entries[game_pos].dlc[entry_pos - (GAMECARD.entries[game_pos].base_count + GAMECARD.entries[game_pos].upp_count)];
    }
}

bool gc_setup_game_info(GameInfo_t *out_info, uint16_t game_pos)
{
    if (!out_info || game_pos >= GAMECARD.file_table.game_count)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    // get the first entry.
    // this ensures that even if the user mounts an xci with only a update / dlc entry, the info will still be shown.
    const GameCardEntry_t *tmp_entry = __gc_get_gamecard_entry(game_pos, 0);
    if (!tmp_entry)
    {
        return false;
    }

    // reset the info.
    memset(out_info, 0, sizeof(GameInfo_t));

    // calculate the size in GiB. If size is less than 10MiB, show size in MiB.
    char size_type_text[] = "GiB";
    float game_size = (float)GAMECARD.entries[game_pos].total_size / 0x40000000;
    if (game_size < 0.01)
    {
        game_size = (float)GAMECARD.entries[game_pos].total_size / 0x100000;
        strcpy(size_type_text, "MiB");
    }

    // store the values.
    out_info->app_id = tmp_entry->cnmt.key.id;
    out_info->key_gen = tmp_entry->key_gen;
    out_info->base_count = GAMECARD.entries[game_pos].base_count;
    out_info->upp_count = GAMECARD.entries[game_pos].upp_count;
    out_info->dlc_count = GAMECARD.entries[game_pos].dlc_count;
    out_info->total_count = GAMECARD.entries[game_pos].total_count;

    // create the textures.
    out_info->text_app_id = create_text(&FONT_TEXT[QFontSize_18], 50, 495, Colour_Nintendo_White, "App-ID: 0%lX", tmp_entry->cnmt.key.id);
    out_info->text_key_gen = create_text(&FONT_TEXT[QFontSize_18], 50, 535, Colour_Nintendo_White, "Key-Gen: %u (%s)", out_info->key_gen, nca_return_key_gen_string(out_info->key_gen));
    out_info->text_size = create_text(&FONT_TEXT[QFontSize_18], 50, 575, Colour_Nintendo_White, "Size: %.2f %s", game_size, size_type_text);
    out_info->text_entry_contents = create_text(&FONT_TEXT[QFontSize_18], 50, 615, Colour_Nintendo_White, "Base: %u Upp: %u DLC: %u", GAMECARD.entries[game_pos].base_count, GAMECARD.entries[game_pos].upp_count, GAMECARD.entries[game_pos].dlc_count);
    enable_text_clip(out_info->text_app_id, 0, 325);
    enable_text_clip(out_info->text_key_gen, 0, 325);
    enable_text_clip(out_info->text_size, 0, 325);
    enable_text_clip(out_info->text_entry_contents, 0, 325);

    // TODO: manually parse the control.nacp if this fails.
    NsApplicationControlData control_data = {0};
    size_t control_data_size = ns_get_app_control_data(&control_data, out_info->app_id);
    if (control_data_size)
    {
        size_t icon_size = control_data_size - sizeof(NacpStruct);
        out_info->icon = create_image_from_mem(&control_data.icon, icon_size, 90, 130, 0, 0);
        out_info->title = create_text(&FONT_TEXT[QFontSize_18], 50, 415, Colour_Nintendo_White, control_data.nacp.lang[0].name);
        out_info->author = create_text(&FONT_TEXT[QFontSize_18], 50, 455, Colour_Nintendo_White, control_data.nacp.lang[0].author);
        enable_text_clip(out_info->title, 0, 325);
        enable_text_clip(out_info->author, 0, 325);
    }

    return true;
}

bool gc_setup_detailed_game_info(GameInfoDetailed_t *info_out, uint16_t entry)
{
    if (!info_out || entry >= GAMECARD.entries[g_game_pos].total_count)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    // get the entry.
    const GameCardEntry_t *tmp_entry = __gc_get_gamecard_entry(g_game_pos, entry);
    if (!tmp_entry)
    {
        return false;
    }

    // setup the vars.
    info_out->type = tmp_entry->cnmt.key.type;
    info_out->id = tmp_entry->cnmt.key.id;
    info_out->keygen = tmp_entry->key_gen;
    info_out->version = tmp_entry->cnmt.key.version;
    info_out->content_count = tmp_entry->cnmt.header.content_count;
    info_out->content_meta_count = tmp_entry->cnmt.header.content_meta_count;

    // create the textures.
    info_out->text_type = create_text(&FONT_TEXT[QFontSize_18], 230, 360, Colour_Nintendo_White, "Type: %s", ncm_get_meta_type_string(info_out->type));
    info_out->text_id = create_text(&FONT_TEXT[QFontSize_18], 230, 390, Colour_Nintendo_White, "ID: %lX", info_out->id);
    info_out->text_keygen = create_text(&FONT_TEXT[QFontSize_18], 230, 420, Colour_Nintendo_White, "Key-Gen: %u", info_out->keygen);
    info_out->text_version = create_text(&FONT_TEXT[QFontSize_18], 230, 450, Colour_Nintendo_White, "Version: %u (%u.%u.%u.%u)", info_out->version, (info_out->version >> 26) & 0x3F, (info_out->version >> 20) & 0x2F, (info_out->version >> 16) & 0xF, (uint16_t)info_out->version);
    info_out->text_content_count = create_text(&FONT_TEXT[QFontSize_18], 230, 480, Colour_Nintendo_White, "Content Count: %u", info_out->content_count);
    info_out->text_content_meta_count = create_text(&FONT_TEXT[QFontSize_18], 230, 510, Colour_Nintendo_White, "Content Meta Count: %u", info_out->content_meta_count);

    info_out->entry = calloc(info_out->content_count, sizeof(GameInfoEntry_t));
    if (!info_out->entry)
    {
        ui_display_error_box(ErrorCode_Alloc, __func__);
        write_log("failed to alloc entry %s\n", __func__);
        return false;
    }

    // setup the textures.
    for (uint16_t i = 0, y = 195; i < info_out->content_count; i++, y += 47)
    {
        char name_buf[0x30] = {0};
        info_out->entry[i].name = create_text(&FONT_TEXT[QFontSize_18], 575, y, Colour_Nintendo_BrightSilver, "%s%s", nca_get_string_from_id(&tmp_entry->cnmt.content_infos[i].content_id, name_buf), tmp_entry->cnmt.content_infos[i].content_type == NcmContentType_Meta ? ".cnmt.nca" : ".nca");
        info_out->entry[i].type = create_text(&FONT_TEXT[QFontSize_18], 575, y += 27, Colour_Nintendo_BrightSilver, "Type: %s", ncm_get_content_type_string(tmp_entry->cnmt.content_infos[i].content_type));
        info_out->entry[i].size = create_text(&FONT_TEXT[QFontSize_18], info_out->entry[i].type->rect.x + info_out->entry[i].type->rect.w + 30, y, Colour_Nintendo_BrightSilver, "Size: %lu", ncm_calculate_content_info_size(&tmp_entry->cnmt.content_infos[i]));
    }

    return true;
}

bool gc_next_game(GameInfo_t *info_out)
{
    if (!info_out)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    if (GAMECARD.file_table.game_count == 0)
    {
        return true;
    }

    g_game_pos = g_game_pos == GAMECARD.file_table.game_count - 1 ? 0 : g_game_pos + 1;
    return gc_setup_game_info(info_out, g_game_pos);
}

bool gc_prev_game(GameInfo_t *info_out)
{
    if (!info_out)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    if (GAMECARD.file_table.game_count == 0)
    {
        return true;
    }

    g_game_pos = g_game_pos == 0 ? GAMECARD.file_table.game_count - 1 : g_game_pos - 1;
    return gc_setup_game_info(info_out, g_game_pos);
}

bool gc_change_game(GameInfo_t *info_out, uint16_t game_pos)
{
    if (!info_out)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    if (game_pos >= GAMECARD.file_table.game_count)
    {
        write_log("cannot swap game, pos too high %s\n", __func__);
        return true;
    }

    g_game_pos = game_pos;
    return gc_setup_game_info(info_out, g_game_pos);
}


/*
*   GameCard Install.
*/

bool __gc_do_ticket_magic(const char *ticket_path, uint64_t id)  //nice function name bro.
{
    if (!ticket_path)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    uint64_t gen = strtoul(ticket_path + 0x10, NULL, 0x10);
    FILE *fp = fopen(ticket_path, "rb");
    if (!fp)
    {
        ui_display_error_box(ErrorCode_OpenFile, __func__);
        write_log("failed to open file %s\n", __func__);
        return false;
    }

    uint8_t key[0x10] = {0};
    // double fseek because a single fseek doesn't work.
    // i've been told that i have to align reads, but i like the fact that double fseek fixes a bug ;)
    fseek(fp, 0x180, SEEK_SET);
    fseek(fp, 0x180, SEEK_SET);
    fread(key, 0x10, 1, fp);
    fclose(fp);

    // decrypt the key.
    crypto_aes(key, key, crypto_get_titlekek_from_keys(gen), EncryptMode_Decrypt);

    // set the key in keyslot.
    // eventually i will re-write this so that i just pass a struct into nca_install.
    // this will have the ticket info in that struct.
    nca_set_keyslot(id, key);

    return true;
}

void __gc_matching_ticket(const GameCard_t *gamecard, const GameCardEntry_t *entry)
{
    // check if we have any tickets.
    if (!gamecard->file_table.tik_count || !gamecard->file_table.cert_count)
    {
        return;
    }

    // check if we already have the key.
    if (setting_get_install_lower_key_gen() && entry->cnmt.key.id == nca_get_keyslot_id())
    {
        return;
    }

    bool found_tik = false;
    bool found_cert = false;
    uint16_t tik_pos = 0;
    uint16_t cert_pos = 0;

    char str_id[0x11] = {0};
    snprintf(str_id, 0x11, "%016lx", (entry->cnmt.key.id));

    for (uint16_t i = 0; i < gamecard->file_table.file_count; i++)
    {
        char *ext = strchr(gamecard->string_table[i].name, '.');
        if (!ext)
        {
            continue;
        }
        if (strcmp(ext, ".tik") == 0 && strstr(gamecard->string_table[i].name, str_id))
        {
            write_log("found ticket\n");
            if (setting_get_install_lower_key_gen())
            {
                __gc_do_ticket_magic(gamecard->string_table[i].name, entry->cnmt.key.id);
                return;
            }
            tik_pos = i;
            found_tik = true;
        }
        else if (strcmp(ext, ".cert") == 0 && strstr(gamecard->string_table[i].name, str_id))
        {
            write_log("found cert\n");
            cert_pos = i;
            found_cert = true;
        }
    }

    if (found_tik && found_cert)
    {
        write_log("installing ticket %s %s\n", gamecard->string_table[tik_pos].name, gamecard->string_table[cert_pos].name);

        size_t tik_size = 0;
        size_t cert_size = 0;

        void *tik_buf = load_file_into_mem(gamecard->string_table[tik_pos].name, &tik_size);
        void *cert_buf = load_file_into_mem(gamecard->string_table[cert_pos].name, &cert_size);

        if (!tik_buf) write_log("no tik buf\n");
        if (!tik_size) write_log("no tik size\n");
        if (!cert_buf) write_log("no cert buf\n");
        if (!cert_size) write_log("no tik size\n");

        es_import_tik_and_cert(tik_buf, tik_size, cert_buf, cert_size);
        free(tik_buf);
        free(cert_buf);
    }

    return;
}

bool __gc_install(const GameCardEntry_t *entry, NcmStorageId storage_id)
{
    if (!entry)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    // add a settings option to make this optional.
    cnmt_set_extended_header(entry->cnmt.extended_header, entry->cnmt.key.type);

    // set the db and push record.
    if (!cnmt_set_db(&entry->cnmt.key, &entry->cnmt.header, entry->cnmt.extended_header, entry->cnmt.content_infos, storage_id))
    {
        ui_display_error_box(ErrorCode_NcmDb, __func__);
        return false;
    }
    
    if (!cnmt_push_record(&entry->cnmt.key, storage_id))
    {
        ui_display_error_box(ErrorCode_AppRecord, __func__);
        return false;
    }

    // clear unused data after pushing record.
    // ie, installing v2 over v1. v1 will still exist in ncm, this will delete it.
    // can be done manually, and will be done so soon.
    // this will also fix mistakes from bad installers that do not clean up left over ncas.
    nsDeleteRedundantApplicationEntity();

    for (uint16_t i = 0; i < entry->cnmt.header.content_count; i++)
    {
        // TODO: add check to make sure that the nca is in the filetable.
        char nca_name_buffer[0x301] = {0};
        snprintf(nca_name_buffer, 0x301, "%s%s", nca_get_string_from_id(&entry->cnmt.content_infos[i].content_id, nca_name_buffer), entry->cnmt.content_infos[i].content_type == NcmContentType_Meta ? ".cnmt.nca" : ".nca");

        FILE *fp = fopen(nca_name_buffer, "rb");
        if (!fp)
        {
            ui_display_error_box(ErrorCode_OpenFile, __func__);
            write_log("failed to open: %s\n", nca_name_buffer);
            return false;
        }
        if (!nca_start_install(nca_name_buffer, &entry->cnmt.content_infos[i].content_id, 0, storage_id, fp))
        {
            write_log("failed to install: %s\n", nca_name_buffer);
            fclose(fp);
            return false; 
        }
        fclose(fp);
    }

    return true;
}

bool gc_install_ex(uint16_t game_pos, NcmStorageId storage_id)
{
    if (storage_id != NcmStorageId_BuiltInUser && storage_id != NcmStorageId_SdCard)
    {
        write_log("got wrong storage id %u %s\n", storage_id, __func__);
        return false;
    }

    if (setting_get_install_lower_key_gen() == SettingFlag_On && !crypto_has_key_gen_from_keys(NcaKeyAreaEncryptionKeyIndex_Application, 0))
    {
        ui_display_error_box(ErrorCode_NoKeyFile, "");
        return false;
    }

    /*
    *   Get the override install location from settings.
    *   TODO: actually implement this into the settings menu.
    */
    SettingsInstallLocation base_location = setting_get_install_base_location();
    if (base_location == SettingsInstallLocation_Default) base_location = storage_id;
    SettingsInstallLocation upp_location = setting_get_install_upp_location();
    if (upp_location == SettingsInstallLocation_Default) upp_location = storage_id;
    SettingsInstallLocation dlc_location = setting_get_install_dlc_location();
    if (dlc_location == SettingsInstallLocation_Default) dlc_location = storage_id;

    SettingFlag base_flag = setting_get_install_base();
    SettingFlag upp_flag = setting_get_install_upp();
    SettingFlag dlc_flag = setting_get_install_dlc();

    // we will allow for split installs soon. so we need to check both storage sizes.
    size_t total_install_size_nand = 0;
    size_t total_install_size_sd = 0;

    // calculate the size of install for each storage.
    if (base_flag && GAMECARD.entries[game_pos].base_count)
    {
        if (base_location == SettingsInstallLocation_User)
        {
            total_install_size_nand += GAMECARD.entries[game_pos].base_size;
        }
        else
        {
            total_install_size_sd += GAMECARD.entries[game_pos].base_size;
        }
    }
    if (upp_flag && GAMECARD.entries[game_pos].upp_count)
    {
        if (base_location == SettingsInstallLocation_User)
        {
            total_install_size_nand += GAMECARD.entries[game_pos].upp_size;
        }
        else
        {
            total_install_size_sd += GAMECARD.entries[game_pos].upp_size;
        }
    }
    if (dlc_flag && GAMECARD.entries[game_pos].dlc_count)
    {
        if (base_location == SettingsInstallLocation_User)
        {
            total_install_size_nand += GAMECARD.entries[game_pos].dlc_size;
        }
        else
        {
            total_install_size_sd += GAMECARD.entries[game_pos].dlc_size;
        }
    }

    if (total_install_size_nand)
    {
        if (ns_get_storage_free_space(storage_id) <= total_install_size_nand)
        {
            write_log("not enough free nand space.\n");
            ui_display_error_box(ErrorCode_NoSpace, __func__);
            return false;
        }
    }

    if (total_install_size_sd)
    {
        if (ns_get_storage_free_space(storage_id) <= total_install_size_sd)
        {
            write_log("not enough free sd space.\n");
            ui_display_error_box(ErrorCode_NoSpace, __func__);
            return false;
        }
    }

    /*
    *   Actually installing stuff now.
    *   Loop though array.
    */

    // base.
    if (base_flag == SettingFlag_On)
    {
        for (uint16_t i = 0; i < GAMECARD.entries[game_pos].base_count; i++)
        {
            if (!ncm_is_key_newer(&GAMECARD.entries[game_pos].base[i].cnmt.key) && setting_get_overwrite_newer_version() == SettingFlag_Off)
            {
                continue;
            }

            __gc_matching_ticket(&GAMECARD, &GAMECARD.entries[game_pos].base[i]);
            if (!__gc_install(&GAMECARD.entries[game_pos].base[i], base_location))
            {
                return false;
            }
        }
    }

    // upp.
    if (upp_flag == SettingFlag_On)
    {
        for (uint16_t i = 0; i < GAMECARD.entries[game_pos].upp_count; i++)
        {
            if (!ncm_is_key_newer(&GAMECARD.entries[game_pos].upp[i].cnmt.key) && setting_get_overwrite_newer_version() == SettingFlag_Off)
            {
                continue;
            }

            __gc_matching_ticket(&GAMECARD, &GAMECARD.entries[game_pos].upp[i]);
            if (!__gc_install(&GAMECARD.entries[game_pos].upp[i], upp_location))
            {
                return false;
            }
        }
    }

    // dlc.
    if (dlc_flag == SettingFlag_On)
    {
        for (uint16_t i = 0; i < GAMECARD.entries[game_pos].dlc_count; i++)
        {
            if (!ncm_is_key_newer(&GAMECARD.entries[game_pos].dlc[i].cnmt.key) && setting_get_overwrite_newer_version() == SettingFlag_Off)
            {
                continue;
            }

            __gc_matching_ticket(&GAMECARD, &GAMECARD.entries[game_pos].dlc[i]);
            if (!__gc_install(&GAMECARD.entries[game_pos].dlc[i], dlc_location))
            {
                return false;
            }
        }
    }

    return true;
}

bool gc_install(NcmStorageId storage_id)
{
    return gc_install_ex(g_game_pos, storage_id);
}