#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
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

bool ncm_check_if_placeholder_id_exists(NcmContentStorage *cs, NcmPlaceHolderId *placeholder_id)
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

bool ncm_create_placeholder_id(NcmContentStorage *cs, NcmContentId *content_id, NcmPlaceHolderId *placeholder_id, int64_t size)
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

bool ncm_get_placeholder_id_path(NcmContentStorage *cs, NcmPlaceHolderId *placeholder_id, char *out_path, size_t size)
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

bool ncm_write_placeholder_id(NcmContentStorage *cs, NcmPlaceHolderId *placeholder_id, uint64_t offset, void *data, size_t data_size)
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

bool ncm_delete_placeholder_id(NcmContentStorage *cs, NcmPlaceHolderId *placeholder_id)
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

bool ncm_register_placeholder_id(NcmContentStorage *cs, NcmContentId *content_id, NcmPlaceHolderId *placeholder_id)
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

bool ncm_check_if_content_id_exists(NcmContentStorage *cs, NcmContentId *content_id)
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

bool ncm_delete_content_id(NcmContentStorage *cs, NcmContentId *content_id)
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

bool ncm_get_content_id_path(NcmContentStorage *cs, char *path_out, size_t size, NcmContentId *content_id)
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

int64_t ncm_get_placeholder_id_size(NcmContentStorage *cs, NcmPlaceHolderId *placeholder_id)
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

int64_t ncm_get_content_id_size(NcmContentStorage *cs, NcmContentId *content_id)
{
    if (!cs || !content_id)
    {
        write_log("missing params in ncm_get_size_of_content_id\n");
        return false;
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

bool ncm_read_content_id(NcmContentStorage *cs, void *data, size_t data_size, uint64_t offset, NcmContentId *content_id)
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

bool ncm_get_version(uint32_t version, uint8_t *major, uint8_t *minor, uint8_t *macro)
{
    if (!version || !major || !minor || !macro)
    {
        write_log ("Missing params in ncm_get_version\n");
        return false;
    }

    *major = (u8)((version >> 26) & 0x3F);
    *minor = (u8)((version >> 20) & 0x3F);
    *macro = (u8)((version >> 16) & 0xF);
    return true;
}

void ncm_get_version_string(uint32_t version, NcmVersionString_t *out)
{
    sprintf(out->major, "%u", (version >> 26) & 0x3F);
    sprintf(out->minor, "%u", (version >> 20) & 0x3F);
    sprintf(out->macro, "%u", (version >> 16) & 0xF);
    sprintf(out->bug_fix, "%u", (u16)version);
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
            return title_id ^ 0x1000;
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