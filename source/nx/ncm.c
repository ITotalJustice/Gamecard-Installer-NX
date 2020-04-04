#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <switch.h>

#include "nx/ncm.h"
#include "util/log.h"


bool ncm_init(void)
{
    Result rc = ncmInitialize();
    if (R_FAILED(rc))
    {
        write_log("Failed to init ncm\n");
        return false;
    }
    return true;
}

void ncm_exit(void)
{
    ncmExit();
}

bool ncm_open_storage(NcmContentStorage *cs, NcmStorageId storage_id)
{
    if (!cs)
    {
        write_log("missing params in ncm_open_storage\n");
        return false;
    }

    Result rc = ncmOpenContentStorage(cs, storage_id);
    if (R_FAILED(rc))
    {
        write_log("failed to open content storage\n");
        return false;
    }

    return true;
}

bool ncm_check_if_placeholder_id_exists(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id)
{
    if (!cs || !placeholder_id)
    {
        write_log("missing params in ncm_check_if_placeholder_exists\n");
        return false;
    }

    bool exists_out = false;
    if (R_FAILED(ncmContentStorageHasPlaceHolder(cs, &exists_out, placeholder_id)))
    {
        write_log("failed to check if placeholder_id exists\n");
        return false;
    }
    return exists_out;
}

bool ncm_generate_placeholder_id(NcmContentStorage *cs, NcmPlaceHolderId *placeholder_id)
{
    if (!cs || !placeholder_id)
    {
        write_log("missing params in ncm_generate_placeholder\n");
        return false;
    }

    Result rc = ncmContentStorageGeneratePlaceHolderId(cs, placeholder_id);
    if (R_FAILED(rc))
    {
        write_log("failed to generate placeholder_id ID\n");
        return false;
    }
    return true;
}

bool ncm_create_placeholder_id(NcmContentStorage *cs, const NcmContentId *content_id, const NcmPlaceHolderId *placeholder_id, int64_t size)
{
    if (!cs || !placeholder_id || !content_id)
    {
        write_log("missing params in ncm_create_placeholder\n");
        return false;
    }

    Result rc = ncmContentStorageCreatePlaceHolder(cs, content_id, placeholder_id, size);
    if (R_FAILED(rc))
    {
        write_log("failed to create placeholder_id\n");
        return false;
    }
    return true;
}

bool ncm_get_placeholder_id_path(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id, char *out_path, size_t size)
{
    if (!cs || !placeholder_id || !out_path || !size)
    {
        write_log("missing params in ncm_get_placeholder_path\n");
        return false;
    }

    Result rc = ncmContentStorageGetPlaceHolderPath(cs, out_path, size, placeholder_id);
    if (R_FAILED(rc))
    {
        write_log("failed to get placeholder_id path\n");
        return false;
    }
    return true;
}

bool ncm_write_placeholder_id(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id, uint64_t offset, const void *data, size_t data_size)
{
    if (!cs || !placeholder_id || !data || !data_size)
    {
        write_log("missing params in ncm_write_placeholder\n");
        return false;
    }

    Result rc = ncmContentStorageWritePlaceHolder(cs, placeholder_id, offset, data, data_size);
    if (R_FAILED(rc))
    {
        write_log("failed to write to placeholder_id\n");
        return false;
    }
    return true;
}

bool ncm_delete_placeholder_id(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id)
{
    if (!cs || !placeholder_id)
    {
        write_log("missing params in ncm_delete_placeholder\n");
        return false;
    }

    Result rc = ncmContentStorageDeletePlaceHolder(cs, placeholder_id);
    if (R_FAILED(rc))
    {
        write_log("failed to delete placeholder_id\n");
        return false;
    }
    return true;
}

void ncm_delete_all_placeholders_id(void)
{
    NcmContentStorage sd_storage = {0};
    if (ncm_open_storage(&sd_storage, NcmStorageId_SdCard))
    {
        if (R_FAILED(ncmContentStorageCleanupAllPlaceHolder(&sd_storage)))
        {
            write_log("failed to delete all placeholder_id from sd card\n");
        }
        ncm_close_storage(&sd_storage);
    }

    NcmContentStorage nand_storage = {0};
    if (ncm_open_storage(&nand_storage, NcmStorageId_BuiltInUser))
    {
        if (R_FAILED(ncmContentStorageCleanupAllPlaceHolder(&nand_storage)))
        {
            write_log("failed to delete all placeholder_id from nand\n");
        }
        ncm_close_storage(&nand_storage);
    }
}

bool ncm_register_placeholder_id(NcmContentStorage *cs, const NcmContentId *content_id, const NcmPlaceHolderId *placeholder_id)
{
    if (!cs || !placeholder_id || !placeholder_id)
    {
        write_log("missing params in ncm_generate_placeholder\n");
        return false;
    }

    Result rc = ncmContentStorageRegister(cs, content_id, placeholder_id);
    if (R_FAILED(rc))
    {
        write_log("failed to register placeholder_id\n");
        return false;
    }
    return true;
}

bool ncm_check_if_content_id_exists(NcmContentStorage *cs, const NcmContentId *content_id)
{
    if (!cs || !content_id)
    {
        write_log("missing params in ncm_check_if_content_id_exists\n");
        return false;
    }

    bool exist_out = 0;
    Result rc = ncmContentStorageHas(cs, &exist_out, content_id);
    if (R_FAILED(rc))
    {
        write_log("failed to check if nca exists\n");
        return false;
    }
    return exist_out;
}

bool ncm_delete_content_id(NcmContentStorage *cs, const NcmContentId *content_id)
{
    if (!cs || !content_id)
    {
        write_log("missing params in ncm_delete_content_id\n");
        return false;
    }

    Result rc = ncmContentStorageDelete(cs, content_id);
    if (R_FAILED(rc))
    {
        write_log("failed to delete nca\n");
        return false;
    }
    return true;
}

bool ncm_get_content_id_path(NcmContentStorage *cs, char *path_out, size_t size, const NcmContentId *content_id)
{
    if (!cs || !content_id || !path_out || !size)
    {
        write_log("missing params in ncm_get_content_id_path\n");
        return false;
    }

    Result rc = ncmContentStorageGetPath(cs, path_out, size, content_id);
    if (R_FAILED(rc))
    {
        write_log("failed to get storage path\n");
        return false;
    }
    return true;
}

int64_t ncm_get_placeholder_id_size(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id)
{
    if (!cs || !placeholder_id)
    {
        write_log("missing params in ncm_get_size_of_placeholder_id\n");
        return false;
    }

    int64_t size = 0;
    Result rc = ncmContentStorageGetSizeFromPlaceHolderId(cs, &size, placeholder_id);
    if (R_FAILED(rc))
    {
        write_log("failed to get size of placeholder_id\n");
        return 0;
    }
    return size;
}

int64_t ncm_get_content_id_size(NcmContentStorage *cs, const NcmContentId *content_id)
{
    if (!cs || !content_id)
    {
        write_log("missing params in ncm_get_size_of_content_id\n");
        return 0;
    }

    int64_t size = 0;
    Result rc = ncmContentStorageGetSizeFromContentId(cs, &size, content_id);
    if (R_FAILED(rc))
    {
        write_log("failed to get size of nca\n");
        return 0;
    }
    return size;
}

bool ncm_read_content_id(NcmContentStorage *cs, void *data, size_t data_size, uint64_t offset, const NcmContentId *content_id)
{
    Result rc = ncmContentStorageReadContentIdFile(cs, data, data_size, content_id, offset);
    if (R_FAILED(rc))
    {
        write_log("failed to read data from the nca\n");
        return false;
    }
    return true;
}

void ncm_close_storage(NcmContentStorage *cs)
{
    ncmContentStorageClose(cs);
    serviceClose(&cs->s);
}

bool ncm_open_database(NcmContentMetaDatabase *db, NcmStorageId storage_id)
{
    Result rc = ncmOpenContentMetaDatabase(db, storage_id);
    if (R_FAILED(rc))
    {
        write_log("failed to open database\n");
        return false;
    }
    return true;
}

bool ncm_set_database(NcmContentMetaDatabase *db, const NcmContentMetaKey *key, const void *data, size_t data_size)
{
    Result rc = ncmContentMetaDatabaseSet(db, key, data, data_size);
    if (R_FAILED(rc))
    {
        write_log("failed to set meta database\n");
        return false;
    }
    return true;
}

bool ncm_commit_database(NcmContentMetaDatabase *db)
{
    Result rc = ncmContentMetaDatabaseCommit(db);
    if (R_FAILED(rc))
    {
        write_log("failed to commit database\n");
        return false;
    }
    return true;
}

void ncm_close_database(NcmContentMetaDatabase *db)
{
    ncmContentMetaDatabaseClose(db);
    serviceClose(&db->s);
}

int64_t ncm_get_storage_free_space(NcmStorageId storage_id)
{
    int64_t size = 0;
    NcmContentStorage cs = {0};
    if (!ncm_open_storage(&cs, storage_id))
    {
        return 0;
    }
    if (R_FAILED(ncmContentStorageGetFreeSpaceSize(&cs, &size)))
    {
        write_log("failed to get free storage space\n");
        return 0;
    }
    ncm_close_storage(&cs);
    return size;
}

int64_t ncm_get_storage_total_size(NcmStorageId storage_id)
{
    int64_t size = 0;
    NcmContentStorage cs = {0};
    if (!ncm_open_storage(&cs, storage_id))
    {
        return size;
    }
    if (R_FAILED(ncmContentStorageGetTotalSpaceSize(&cs, &size)))
    {
        write_log("failed to get total storage space\n");
        return 0;
    }
    ncm_close_storage(&cs);
    return size;
}

int32_t ncm_get_application_meta_total(NcmContentMetaDatabase *db, NcmContentMetaType type)
{
    int32_t total = 0;
    int32_t written = 0;
    NcmApplicationContentMetaKey key = {0};
    Result rc = ncmContentMetaDatabaseListApplication(db, &total, &written, &key, 1, type);
    if (R_FAILED(rc))
    {
        write_log("Failed to get appliaction meta total. type: %s\n", ncm_get_meta_type_string(type));
        return 0;
    }
    return total;
}

int32_t ncm_get_meta_total(NcmContentMetaDatabase *db, NcmContentMetaType type)
{
    int32_t total = 0;
    int32_t written = 0;
    NcmContentMetaKey key = {0};
    Result rc = ncmContentMetaDatabaseList(db, &total, &written, &key, 1, type, 0, 0, UINT64_MAX, NcmContentInstallType_Full);
    if (R_FAILED(rc))
    {
        write_log("Failed to get meta total. type: %s\n", ncm_get_meta_type_string(type));
        return 0;
    }
    return total;
}

bool ncm_get_latest_key(NcmContentMetaDatabase *db, NcmContentMetaKey *key_out, uint64_t id)
{
    if (!db || !key_out || !id)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    Result rc = ncmContentMetaDatabaseGetLatestContentMetaKey(db, key_out, id);
    if (rc == 3589) // none found, not an error. still return false because no key.
    {
        write_log("no latest key found for id: %lX\n", id);
        return false;
    }
    if (R_FAILED(rc))
    {
        write_log("failed to get latest key\n");
        return false;
    }

    return true;
}

bool ncm_get_latest_key2(NcmContentMetaKey *out_key, uint64_t id) // temp name
{
    if (!id)
    {
        write_log("missing params in %s\n", __func__);
        return 1;
    }

    // vars
    NcmContentMetaDatabase db_user = {0};
    NcmContentMetaDatabase db_sdcard = {0};
    NcmContentMetaKey key_user = {0};
    NcmContentMetaKey key_sdcard = {0};

    // open both storage.
    ncm_open_database(&db_user, NcmStorageId_BuiltInUser);
    ncm_open_database(&db_sdcard, NcmStorageId_SdCard);

    // first we try to find it in user, the sdcard.
    write_log("trying nand\n");
    bool has_user = ncm_get_latest_key(&db_user, &key_user, id);
    write_log("trying sd\n");
    bool has_sdcard = ncm_get_latest_key(&db_sdcard, &key_sdcard, id);

    // now close.
    ncm_close_database(&db_user);
    ncm_close_database(&db_sdcard);

    // check if any was found.
    if (!has_user && !has_sdcard)
    {
        return false;
    }

    // If theres 2 versions (ie. 2 updates exist because they were installed by bad installers), pick the highest value.
    if (has_user && has_sdcard)
    {
        write_log("found meta key id: %lx on sdcard and nand\n", id);

        // select highest versions
        if (key_user.version > key_sdcard.version)
        {
            *out_key = key_user;
            return true;
        }
        else
        {
            *out_key = key_sdcard;
            return true;
        }
    }

    if (has_user)
    {
        *out_key = key_user;
        return true;
    }
    else
    {
        *out_key = key_sdcard;
        return true;
    }
}

bool ncm_is_key_newer(const NcmContentMetaKey *key)
{
    NcmContentMetaKey key_out = {0};

    // if it couldnt find a key, its newer.
    if (!ncm_get_latest_key2(&key_out, key->id))
    {
        return true;
    }

    write_log("new: %u installed: %u\n", key->version, key_out.version);
    return key->version >= key_out.version;
}

bool ncm_get_version(uint32_t version, uint8_t *major, uint8_t *minor, uint8_t *macro)
{
    if (!version || !major || !minor || !macro)
    {
        write_log ("Missing params in %s\n", __func__);
        return false;
    }

    *major = (uint8_t)((version >> 26) & 0x3F);
    *minor = (uint8_t)((version >> 20) & 0x3F);
    *macro = (uint8_t)((version >> 16) & 0xF);
    return true;
}

void ncm_get_version_string(uint32_t version, NcmVersionString_t *out)
{
    sprintf(out->major, "%u", (version >> 26) & 0x3F);
    sprintf(out->minor, "%u", (version >> 20) & 0x3F);
    sprintf(out->macro, "%u", (version >> 16) & 0xF);
    sprintf(out->bug_fix, "%u", (uint16_t)version);
}

#include <string.h>
size_t ncm_calculate_content_info_size(const NcmContentInfo *info)
{
    if (!info)
    {
        write_log("missing params in %s\n", __func__);
        return 0;
    }

    size_t size = 0;
    memcpy(&size, info->size, 0x6);
    return size;
}

size_t ncm_calculate_content_infos_size(const NcmContentInfo *infos, uint16_t count)
{
    if (!infos || !count)
    {
        write_log("missing params in %s\n", __func__);
        return 0;
    }

    size_t size = 0;
    
    for (uint16_t i = 0; i < count; i++)
    {
        size_t tmp_size = 0;
        if (!(tmp_size = ncm_calculate_content_info_size(&infos[i])))
        {
            return 0;
        }
        size += tmp_size;
    }

    return size;
}

bool ncm_get_content_type_from_infos(NcmContentInfo *infos, uint16_t count, NcmContentInfo *found, NcmContentType wanted_type)
{
    if (!infos || !count || !found)
    {
        write_log("missing params in %s\n", __func__);
        return false;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        if (infos[i].content_type == wanted_type)
        {
            *found = infos[i];
            return true;
        }
    }

    return false;
}

uint64_t ncm_get_app_id_from_title_id(uint64_t title_id, NcmContentMetaType type)
{
    switch (type)
    {
        case NcmContentMetaType_SystemProgram:
            return title_id;
        case NcmContentMetaType_SystemData:
            return title_id;
        case NcmContentMetaType_SystemUpdate:
            return title_id;
        case NcmContentMetaType_BootImagePackage:
            return title_id;
        case NcmContentMetaType_BootImagePackageSafe:
            return title_id;
        case NcmContentMetaType_Application:
            return title_id;
        case NcmContentMetaType_Patch:
            return title_id ^ 0x800;
        case NcmContentMetaType_AddOnContent:
            return (title_id ^ 0x1000) & ~0xFFF;
        default:
            return 0;                              
    }
}

const char *ncm_get_storage_id_string(NcmStorageId storage_id)
{
    switch (storage_id)
    {
        case NcmStorageId_None:
            return "None";
        case NcmStorageId_Host:
            return "Host";
        case NcmStorageId_GameCard:
            return "GameCard";
        case NcmStorageId_BuiltInSystem:
            return "BuildInSystem";
        case NcmStorageId_BuiltInUser:
            return "BuiltInUser";
        case NcmStorageId_SdCard:
            return "SdCard";
        case NcmStorageId_Any:
            return "Any";
        default:
            return "NULL";
    }
}

const char *ncm_get_meta_type_string(NcmContentMetaType type)
{
    switch (type)
    {
        case NcmContentMetaType_SystemProgram:
            return "SystemProgram";
        case NcmContentMetaType_SystemData:
            return "SystemData";
        case NcmContentMetaType_SystemUpdate:
            return "SystemUpdate";
        case NcmContentMetaType_BootImagePackage:
            return "BootImagePackage";
        case NcmContentMetaType_BootImagePackageSafe:
            return "BootImagePackageSafe";
        case NcmContentMetaType_Application:
            return "Application";
        case NcmContentMetaType_Patch:
            return "Patch";
        case NcmContentMetaType_AddOnContent:
            return "AddOnContent";
        case NcmContentMetaType_Delta:
            return "Delta";
        default:
            return "NULL";                              
    }
}

const char *ncm_get_content_type_string(NcmContentType type)
{
    switch (type)
    {
        case NcmContentType_Meta:
            return "Meta";
        case NcmContentType_Program:
            return "Program";
        case NcmContentType_Data:
            return "Data";
        case NcmContentType_Control:
            return "Control";
        case NcmContentType_HtmlDocument:
            return "HtmlDocument";
        case NcmContentType_LegalInformation:
            return "LegalInformation";
        case NcmContentType_DeltaFragment:
            return "DeltaFragment";
        default:
            return "NULL";
    }
}

const char *ncm_get_attribute_string(NcmContentMetaAttribute attribute)
{
    switch (attribute)
    {
        case NcmContentMetaAttribute_None:
            return "None";
        case NcmContentMetaAttribute_IncludesExFatDriver:
            return "IncludesExFatDriver";
        case NcmContentMetaAttribute_Rebootless:
            return "Rebootless";
        default:
            return "NULL";
    }
}

const char *ncm_get_install_type_string(NcmContentInstallType type)
{
    switch (type)
    {
        case NcmContentInstallType_Full:
            return "Full";
        case NcmContentInstallType_FragmentOnly:
            return "FragmentOnly";
        case NcmContentInstallType_Unknown:
            return "Unknown";
        default:
            return "NULL";
    }
}


/*
*   Debug
*/

void ncm_print_key(const NcmContentMetaKey *key)
{
    #ifdef DEBUG
    if (!key)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }

    write_log("\nncm key print\n");
    write_log("id: %lX\n", key->id);
    write_log("title_version: %u\n", key->version);
    write_log("meta_type: %s\n", ncm_get_meta_type_string(key->type));
    write_log("install_type: %s\n", ncm_get_install_type_string(key->install_type));
    #endif
}

void ncm_print_keys(const NcmContentMetaKey *key, uint16_t count)
{
    #ifdef DEBUG
    if (!key || !count)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }

    for (uint16_t i = 0; i < count; i++)
    {
        ncm_print_key(&key[i]);
    }
    #endif
}

void ncm_print_application_record(const NcmContentStorageRecord *record)
{
    #ifdef DEBUG
    if (!record)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }

    write_log("\napplication_record\n");
    write_log("id %lX\n", record->key.id);
    write_log("version: %u\n", record->key.version);
    write_log("meta_type: %s\n", ncm_get_meta_type_string(record->key.type));
    write_log("install_type: %s\n", ncm_get_install_type_string(record->key.install_type));
    write_log("storage_id: %s\n", ncm_get_storage_id_string(record->storage_id));
    #endif
}

void ncm_print_application_records(const NcmContentStorageRecord *records, uint32_t count)
{
    #ifdef DEBUG
    if (!records || !count)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        ncm_print_application_record(&records[i]);
    }
    #endif
}