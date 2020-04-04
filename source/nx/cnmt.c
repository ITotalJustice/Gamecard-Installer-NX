#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "nx/fs.h"
#include "nx/ncm.h"
#include "nx/ns.h"
#include "nx/nca.h"
#include "nx/cnmt.h"

#include "util/util.h"
#include "util/log.h"


bool cnmt_push_record(const NcmContentMetaKey *key, NcmStorageId storage_id)
{
    if (!key)
    {
        write_log("missing params in cnmt_push_record\n");
        return false;
    }

    uint64_t app_id = ncm_get_app_id_from_title_id(key->id, key->type);

    NcmContentStorageRecord record = {0};
    record.key = *key;
    record.storage_id = storage_id;

    if (!ns_push_application_record(app_id, &record, 1))
    {
        return false;
    }

    return true;
}

bool cnmt_set_db(const NcmContentMetaKey *key, const NcmContentMetaHeader *header, const void *extended_header, const NcmContentInfo *info, NcmStorageId storage_id)
{
    if (!key || !header || !extended_header || !info)
    {
        write_log("missing params in cnmt_set_db\n");
        return false;
    }

    size_t size = sizeof(NcmContentMetaHeader) + header->extended_header_size + (header->content_count * sizeof(NcmContentInfo));
    uint8_t *data = calloc(1, size);
    if (!data)
    {
        write_log("failed to alloc cnmt data for ncm db\n");
        return false;
    }

    uint64_t data_offset = 0;
    safe_memcpy(&data[data_offset], header, sizeof(NcmContentMetaHeader));
    safe_memcpy(&data[data_offset += sizeof(NcmContentMetaHeader)], extended_header, header->extended_header_size);
    safe_memcpy(&data[data_offset += header->extended_header_size], info, header->content_count * sizeof(NcmContentInfo));

    NcmContentMetaDatabase db = {0};
    if (!ncm_open_database(&db, storage_id))
    {
        free(data);
        return false;
    }

    if (!ncm_set_database(&db, key, data, size))
    {
        free(data);
        ncm_close_database(&db);
        return false;
    }
    
    if (!ncm_commit_database(&db))
    {
        free(data);
        ncm_close_database(&db);
        return false;
    }

    free(data);
    ncm_close_database(&db);
    return true;
}

void cnmt_set_extended_header(void *extended_header, NcmContentMetaType type)
{
    if (!extended_header)
    {
        write_log("missing params in cnmt_set_extended_header\n");
        return;
    }

    if (type == NcmContentMetaType_Application)
    {
        NcmApplicationMetaExtendedHeader *ext_header = (NcmApplicationMetaExtendedHeader*)extended_header;
        ext_header->required_system_version = 0;
        ext_header->required_application_version = 0;
    }
    else if (type == NcmContentMetaType_Patch)
    {
        NcmPatchMetaExtendedHeader *ext_header = (NcmPatchMetaExtendedHeader*)extended_header;
        ext_header->required_system_version = 0;
    }
    else if (type == NcmContentMetaType_AddOnContent)
    {
        NcmAddOnContentMetaExtendedHeader *ext_header = (NcmAddOnContentMetaExtendedHeader*)extended_header;
        ext_header->required_application_version = 0;
    }
}

bool cnmt_get_header_and_key(const uint8_t *cnmt_data, NcmContentMetaHeader *header_out, NcmContentMetaKey *key_out, int64_t offset)
{
    if (!cnmt_data || !header_out || !key_out)
    {
        write_log("missing params in cnmt_get_header_from_file\n");
        return false;
    }

    CnmtFullHeader_t header = {0};
    safe_memcpy(&header, cnmt_data + offset, sizeof(CnmtFullHeader_t));

    header_out->extended_header_size = header.extended_header_size;
    header_out->content_count = header.content_count;
    header_out->content_meta_count = header.content_meta_count;
    header_out->attributes = header.attributes;
    header_out->storage_id = 0;

    key_out->id = header.title_id;
    key_out->version = header.title_version;
    key_out->type = header.meta_type;
    key_out->install_type = NcmContentInstallType_Full;

    return true;
}

bool cnmt_parse(const uint8_t *cnmt_data, uint64_t offset, const NcmContentInfo *cnmt_info, Cnmt_t *cnmt_out)
{
    if (!cnmt_data || !cnmt_info || !cnmt_out)
    {
        write_log("missing params in cnmt parse\n");
        return false;
    }

    if (!cnmt_get_header_and_key(cnmt_data, &cnmt_out->header, &cnmt_out->key, offset))
    {
        return false;
    }
    offset += sizeof(CnmtFullHeader_t);

    // get extended header data.
    cnmt_out->extended_header = calloc(1, cnmt_out->header.extended_header_size);
    if (!cnmt_out->extended_header)
    {
        write_log("Failed to get cnmt ext header\n");
        return false;
    }

    // copy the extended header info.
    safe_memcpy(cnmt_out->extended_header, cnmt_data + offset, cnmt_out->header.extended_header_size);
    offset += cnmt_out->header.extended_header_size;

    // get all the ncaID's, add 1 for cnmt.
    cnmt_out->content_infos = calloc((cnmt_out->header.content_count + 1), sizeof(NcmContentInfo));
    if (!cnmt_out->content_infos)
    {
        write_log("Failed to alloc cnmt infos\n");
        free(cnmt_out->extended_header);
        return false;
    }

    // copy info of cnmt...
    safe_memcpy(&cnmt_out->content_infos[0], cnmt_info, sizeof(NcmContentInfo));
    uint16_t info_count = 1;

    // get all the content_infos.
    for (uint16_t i = 0; i < cnmt_out->header.content_count; i++)
    {
        NcmPackagedContentInfo packed_temp = {0};
        safe_memcpy(&packed_temp, cnmt_data + offset, sizeof(NcmPackagedContentInfo));
        offset += sizeof(NcmPackagedContentInfo);

        // skip deltas...
        if (packed_temp.info.content_type == NcmContentType_DeltaFragment)
        {
            continue;
        }
        
        // if not delta, copy data into struct.
        safe_memcpy(&cnmt_out->content_infos[info_count], &packed_temp.info, sizeof(NcmContentInfo));
        info_count++;
    }

    // update header with new content count.
    cnmt_out->header.content_count = info_count;
    return true;
}

bool cnmt_open_installed_file(const NcmContentId *content_id, const NcmContentInfo *cnmt_info, Cnmt_t *cnmt_out, NcmStorageId storage_id)
{
    if (!content_id || !cnmt_info || !cnmt_out)
    {
        write_log("missing params in cnmt_open\n");
        return false;
    }

    FsFileSystem fs = {0};
    FsDir dir = {0};
    FsFile file = {0};

    if (!fs_open_system_with_content_id_2(&fs, content_id, FsFileSystemType_ContentMeta, storage_id))
    {
        return false;
    }

    // open the mounted cnmt system as a dir.
    write_log("opening cnmt dir\n");
    if (!fs_open_dir(&fs, FsDirOpenMode_ReadFiles | FsDirOpenMode_ReadDirs, &dir, "/"))
    {
        fs_close_system(&fs);
        return false;
    }

    // find the cnmt file inside folder and open it.
    write_log("opening cnmt file\n");
    FsDirectoryEntry cnmt_entry = {0};
    if (!fs_search_dir_for_file_2(&dir, &cnmt_entry, "cnmt"))
    {
        fs_close_dir(&dir);
        fs_close_system(&fs);
        return false;
    }

    if (!fs_open_file(&fs, FsOpenMode_Read, &file, "/%s", cnmt_entry.name))
    {
        fs_close_dir(&dir);
        fs_close_system(&fs);
        return false;
    }
    
    int64_t cnmt_size = fs_get_file_size(&file);
    if (!cnmt_size)
    {
        fs_close_dir(&dir);
        fs_close_system(&fs);
        return false;
    }

    void *cnmt_data = calloc(1, cnmt_size);
    if (!cnmt_data)
    {
        write_log("failed to alloc cnmt data\n");
        fs_close_dir(&dir);
        fs_close_system(&fs);
        return false;
    }

    if (fs_read_file(cnmt_data, cnmt_size, 0, FsReadOption_None, &file) != cnmt_size)
    {
        write_log("cnmt data read size missmatch\n");
        fs_close_dir(&dir);
        fs_close_system(&fs);
        return false;
    }

    // close everything.
    fs_close_file(&file);
    fs_close_dir(&dir);
    fs_close_system(&fs);

    // parse the cnmt data.
    if (!cnmt_parse(cnmt_data, 0, cnmt_info, cnmt_out))
    {
        free(cnmt_data);
        return false;
    }

    free(cnmt_data);
    return true;
}


/*
*   Debug
*/

void cnmt_print_header(const NcmContentMetaHeader *header, const NcmContentMetaKey *key)
{
    #ifdef DEBUG
    if (!header || !key)
    {
        write_log("missing params in cnmt_print_header\n");
        return;
    }

    write_log("\ncnmt header print\n");
    write_log("app_id: %lX\n", key->id);
    write_log("title_version: %u\n", key->version);
    write_log("meta_type: %s\n", ncm_get_meta_type_string(key->type));
    write_log("extended_header_size: %u\n", header->extended_header_size);
    write_log("content_count: %u\n", header->content_count);
    write_log("content_meta_count: %u\n", header->content_meta_count);
    write_log("attributes: %s\n", ncm_get_attribute_string(header->attributes));
    #endif
}

void cnmt_print_extended_header(const void *extended_header, NcmContentMetaType type)
{
    #ifdef DEBUG
    if (!extended_header)
    {
        write_log("missing params in cnmt_print_extended_header\n");
        return;
    }

    switch (type)
    {
        case NcmContentMetaType_Application:
        {
            write_log("\nmeta is application\n");
            NcmApplicationMetaExtendedHeader *ext_header = (NcmApplicationMetaExtendedHeader*)extended_header;
            write_log("required_system_version: %u\n", ext_header->required_system_version);
            write_log("required_app_version: %u\n\n", ext_header->required_application_version);
            break;
        }
        case NcmContentMetaType_Patch:
        {
            write_log("\nmeta is patch\n");
            NcmPatchMetaExtendedHeader *ext_header = (NcmPatchMetaExtendedHeader*)extended_header;
            write_log("required_system_version: %u\n\n", ext_header->required_system_version);
            break;
        }
        case NcmContentMetaType_AddOnContent:
        {
            write_log("\nmeta is addon\n");
            NcmAddOnContentMetaExtendedHeader *ext_header = (NcmAddOnContentMetaExtendedHeader*)extended_header;
            write_log("required_app_version: %u\n\n", ext_header->required_application_version);
            break;
        }
        case NcmContentMetaType_SystemProgram:
        {
            break;
        }
        case NcmContentMetaType_SystemData:
        {
            break;
        }
        case NcmContentMetaType_SystemUpdate:
        {
            break;
        }
        case NcmContentMetaType_BootImagePackage:
        {
            break;
        }
        case NcmContentMetaType_BootImagePackageSafe:
        {
            break;
        }
        case NcmContentMetaType_Delta:
        {
            break;
        }
        default:
        {
            write_log("\nunknown meta type: %u\n\n", type);
            break;
        }
    }
    #endif
}

void cnmt_print_content_info(const NcmContentInfo *info)
{
    #ifdef DEBUG
    if (!info)
    {
        write_log("missing params in cnmt_print_content_info\n");
        return;
    }

    char content_id_buffer[0x40] = {0};
    write_log("\ncontent info print\n");
    write_log("content_id: %s\n", nca_get_string_from_id(&info->content_id, content_id_buffer));
    write_log("size: %lu\n", (uint64_t)info->size);
    write_log("content_type: %s\n", ncm_get_content_type_string(info->content_type));
    write_log("offset: %u\n", info->id_offset);
    #endif
}

void cnmt_print_content_infos(const NcmContentInfo *info, uint16_t total)
{
    #ifdef DEBUG
    if (!info || !total)
    {
        write_log("missing params in cnmt_print_content_info\n");
        return;
    }

    for (uint16_t i = 0; i < total; i++)
    {
        cnmt_print_content_info(&info[i]);
    }
    #endif
}