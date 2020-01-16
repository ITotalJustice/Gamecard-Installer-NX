#include <stdbool.h>
#include <switch.h>

#include "ui/gc.h"

#include "nx/fs.h"
#include "nx/ns.h"
#include "nx/nca.h"
#include "nx/ncm.h"
#include "nx/cnmt.h"

#include "gfx/image.h"
#include "gfx/text.h"

#include "util/dir.h"
#include "util/file.h"
#include "util/util.h"


typedef struct
{
    char cnmt[0x30];
    uint64_t title_id;
} gc_cnmt_t;


FsDeviceOperator g_dop = {0};
FsGameCardHandle g_gc_handle = {0};
char g_gc_mount_path[0x10] = {0};

// max games per gc is 16.
gc_cnmt_t gc_cnmt[0x10] = {0};
uint8_t g_cnmt_total = 0;
uint8_t g_curr_game = 0;


bool init_gc(void)
{
    if (R_FAILED(fs_open_device_operator(&g_dop)))
    {
        printf("failed to mount gcop\n");
        return false;
    }
    return true;
}

void exit_gc(void)
{
    fs_close_device_operator(&g_dop);
    fs_close_gamecard_handle(&g_gc_handle);
    fs_unmount_device(g_gc_mount_path);
}

bool poll_gc(void)
{
    return fs_is_gamecard_inserted(&g_dop);
}

bool setup_gamecard(gamecard_t *gamecard, gc_cnmt_t *gc_cnmt)
{
    struct dirent *d = NULL;
    DIR *dir = open_dir(".");
    if (!dir)
    {
        printf("failed to open dir %s\n", ".");
        fs_unmount_device(g_gc_mount_path);
        return false;
    }

    while ((d = readdir(dir)))
    {
        FILE *fp = fopen(d->d_name, "rb");
        if (!fp)
        {
            printf("failed to somehow open file in gamecard %s\n", d->d_name);
            closedir(dir);
            fs_unmount_device(g_gc_mount_path);
            return false;
        }

        // get the nca header, decrypt it to get the nca size.
        nca_header_t header = {0};
        nca_get_header_decrypted(fp, 0, &header);
        fclose(fp);

        // check if this file belongs to this cnmt.
        if (header.title_id != gc_cnmt->title_id)
            continue;

        // add the size of the nca.
        gamecard->size += header.size;

        // we need to get info on the game from at least one of the nca's.
        // It can be any nca header.
        if (header.content_type == NcaContentType_Meta)
        {
            // get the app_id.
            gamecard->app_id = ncm_get_app_id_from_title_id(header.title_id, NcmContentMetaType_Application);

            NsApplicationControlData control_data = {0};
            NacpLanguageEntry *lang = {0};

            ns_get_app_control_data(&control_data, gamecard->app_id);
            nacpGetLanguageEntry(&control_data.nacp, &lang);

            strcpy(gamecard->cnmt_name, d->d_name);
            gamecard->icon = create_image_from_mem(&control_data.icon, 0x20000, 90, 130, 0, 0);
            gamecard->title = create_text(&FONT_TEXT[QFontSize_18], 50, 420, Colour_Nintendo_White, lang->name);
            gamecard->author = create_text(&FONT_TEXT[QFontSize_18], 50, 455, Colour_Nintendo_White, lang->author);
            gamecard->text_app_id = create_text(&FONT_TEXT[QFontSize_18], 50, 490, Colour_Nintendo_White, "App-ID: 0%lX", gamecard->app_id);
            gamecard->text_key_gen = create_text(&FONT_TEXT[QFontSize_18], 50, 525, Colour_Nintendo_White, "Key-Gen: %s", nca_return_key_gen_string(header.key_gen ? header.key_gen : header.old_key_gen));
        }
    }

    gamecard->text_size = create_text(&FONT_TEXT[QFontSize_18], 50, 560, Colour_Nintendo_White, "Size: %.2fGB", (float)gamecard->size / 0x40000000);
    closedir(dir);
    return true;
}

bool save_cnmt_path(void)
{
    struct dirent *d = NULL;
    DIR *dir = open_dir(".");
    if (!dir)
    {
        printf("failed to open dir %s\n", ".");
        return false;
    }

    int i = 0;
    while ((d = readdir(dir)))
    {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
            continue;

        if (strstr(d->d_name, "cnmt.nca"))
        {
            FILE *fp = fopen(d->d_name, "rb");
            if (!fp)
            {
                printf("failed to somehow open file in gamecard %s\n", d->d_name);
                closedir(dir);
                return false;
            }

            // get the nca header, decrypt it to get the nca size.
            nca_header_t header = {0};
            nca_get_header_decrypted(fp, 0, &header);
            fclose(fp);

            // save the cnmt info.
            gc_cnmt[i].title_id = header.title_id;
            strcpy(gc_cnmt[i].cnmt, d->d_name);
            i++;
        }
    }
    return true;
}

bool mount_gc(gamecard_t *gamecard)
{
    if (R_FAILED(fs_get_gamecard_handle_from_device_operator(&g_dop, &g_gc_handle)))
    {
        printf("failed to get gc handle\n");
        return false;
    }

    if (!fs_mount_gamecard_partition(g_gc_mount_path, g_gc_handle, FsGameCardPartition_Secure))
    {
        printf("failed to mount gc\n");
        return false;
    }
    
    if (!change_dir("%s%s", g_gc_mount_path, ":/"))
    {
        printf("failed to change path to gc\n");
        fs_unmount_device(g_gc_mount_path);
        return false;
    }

    g_cnmt_total = get_dir_total_filter(".", "cnmt.nca");
    if (!g_cnmt_total)
    {
        printf("no cnmt's found!? How on earth is that possible\n");
        fs_unmount_device(g_gc_mount_path);
        return false;
    }

    if (!save_cnmt_path())
    {
        printf("failed to save the paths of the cnmt's\n");
        fs_unmount_device(g_gc_mount_path);
        return false;
    }

    return setup_gamecard(gamecard, &gc_cnmt[g_curr_game]);
}

void swap_game_in_gc(gamecard_t *gamecard)
{
    // theres no need to swap if theres only one game in the gc.
    if (g_cnmt_total == 1)
        return;
    
    reset_gc(gamecard);
    g_curr_game = g_cnmt_total == (g_curr_game + 1) ? 0 : g_curr_game + 1;
    setup_gamecard(gamecard, &gc_cnmt[g_curr_game]);
}

void reset_gc(gamecard_t *gamecard)
{
    free_image(gamecard->icon);
    free_text(gamecard->title);
    free_text(gamecard->author);
    free_text(gamecard->text_size);
    free_text(gamecard->text_key_gen);
    free_text(gamecard->text_app_id);
    memset(gamecard, 0, sizeof(gamecard_t));
}

void unmount_gc(gamecard_t *gamecard)
{
    fs_close_gamecard_handle(&g_gc_handle);
    fs_umount_all_devices();
    g_cnmt_total = 0;
    memset(g_gc_mount_path, 0, 0x10);
    memset(gc_cnmt, 0, sizeof(gc_cnmt_t));
    reset_gc(gamecard);
}

bool install_gc(gamecard_t *gamecard, NcmStorageId storage_id)
{
    if (!gamecard)
    {
        printf("gamecard is null? failed to install\n");
        return false;
    }

    if (storage_id != NcmStorageId_BuiltInUser && storage_id != NcmStorageId_SdCard)
    {
        printf("got wrong storage id %u\n", storage_id);
        return false;
    }

    NcmContentId content_id = nca_get_id_from_string(gamecard->cnmt_name);
    if (!nca_start_install(content_id, storage_id))
    {
        printf("failed to install nca\n");
        return false;
    }

    // the cnmt install is old code from yati (aka its very bad).
    cnmt_struct_t cnmt = {0};
    cnmt.cnmt_info.content_id = content_id;
    cnmt.storage_id = storage_id;

    if (R_FAILED(cnmt_open(&cnmt)))
    {
        printf("failed to install cnmt\n");
        return false;
    }

    for (uint32_t i = 1; i < cnmt.total_cnmt_infos; i++)
    {
        if (!nca_start_install(cnmt.cnmt_infos[i].content_id, storage_id))
        {
            printf("failed to install nca\n");
            break;
        }
    }
    
    free(cnmt.cnmt_infos);
    return true;
}