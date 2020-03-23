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

#include "util/log.h"


bool cnmt_push_record(const NcmContentMetaKey *key, uint64_t app_id, NcmStorageId storage_id)
{
    if (!key || !app_id)
    {
        write_log("missing params in cnmt_push_record\n");
        return false;
    }

    int32_t record_count = ns_count_application_content_meta(app_id);
    NcmContentStorageRecord *app_record = calloc(record_count + 1, sizeof(NcmContentStorageRecord));

    if (record_count > 0)
    {
        ns_list_application_record_content_meta(0, app_id, app_record, record_count);
    }

    app_record[record_count].meta_key = *(NcmContentMetaKey*)key;
    app_record[record_count].storage_id = storage_id;

    ns_delete_application_record(app_id);
    if (!ns_push_application_record(app_id, app_record, record_count + 1))
    {
        free(app_record);
        return false;
    }

    free(app_record);
    return true;
}

bool cnmt_set_db(const NcmContentMetaKey *key, const NcmContentMetaHeader *header, const void *extended_header, const NcmContentInfo *info, uint32_t info_count, NcmStorageId storage_id)
{
    if (!key || !header || !extended_header || !info)
    {
        write_log("missing params in cnmt_set_db\n");
        return false;
    }

    struct
    {
        const NcmContentMetaHeader *header;
        const void *extended_header;
        const NcmContentInfo *info;
    } const in = { header, extended_header, info };

    size_t size = sizeof(NcmContentMetaHeader) + header->extended_header_size + (info_count * sizeof(NcmContentInfo));

    NcmContentMetaDatabase db = {0};
    if (!ncm_open_database(&db, storage_id))
    {
        return false;
    }

    if (!ncm_set_database(&db, key, &in, size))
    {
        ncm_close_database(&db);
        return false;
    }
    
    if (!ncm_commit_database(&db))
    {
        ncm_close_database(&db);
        return false;
    }

    ncm_close_database(&db);
    return true;
}

void cnmt_set_extended_header(void *extended_header, NcmContentMetaType type)
{
    if (type == NcmContentMetaType_Application)
    {
        NcmApplicationMetaExtendedHeader *ext_header = (NcmApplicationMetaExtendedHeader*)extended_header;
        write_log("meta is application. sys: %u  app_ver: %u\n", ext_header->required_system_version, ext_header->required_application_version);
        ext_header->required_system_version = 0;
        ext_header->required_application_version = 0;
    }
    else if (type == NcmContentMetaType_Patch)
    {
        NcmPatchMetaExtendedHeader *ext_header = (NcmPatchMetaExtendedHeader*)extended_header;
        write_log("meta is path. sys: %u\n", ext_header->required_system_version);
        ext_header->required_system_version = 0;
    }
    else if (type == NcmContentMetaType_AddOnContent)
    {
        NcmAddOnContentMetaExtendedHeader *ext_header = (NcmAddOnContentMetaExtendedHeader*)extended_header;
        write_log("meta is addon. app_ver: %u\n", ext_header->required_application_version);
        ext_header->required_application_version = 0;
    }
}

void cnmt_print_header(const NcmContentMetaHeader *header, const NcmContentMetaKey *key)
{
    write_log("\ncnmt header print\n");
    write_log("\ttitle id: %lX\n", ncm_get_app_id_from_title_id(key->id, key->type));
    write_log("\ttitle version: %u\n", key->version);
    write_log("\tmeta type: %s\n", ncm_get_meta_type_string(key->type));
    write_log("\text header size: %u\n", header->extended_header_size);
    write_log("\tcontent count: %u\n", header->content_count);
    write_log("\tcontent meta count: %u\n", header->content_meta_count);
    write_log("\tattributes: %s\n", ncm_get_attribute_string(header->attributes));
}

bool cnmt_get_meta_header_and_key(const CnmtHeader_t *header, NcmContentMetaHeader *meta_header, NcmContentMetaKey *key)
{
    if (!header || !meta_header || !key)
    {
        write_log("missing params in cnmt_get_meta_header_and_key\n");
        return false;
    }

    meta_header->extended_header_size = header->extended_header_size;
    meta_header->content_count = header->content_count;
    meta_header->content_meta_count = header->content_meta_count;
    meta_header->attributes = header->attributes;
    meta_header->storage_id = 0;

    key->id = header->title_id;
    key->version = header->title_version;
    key->type = header->meta_type;
    key->install_type = NcmContentInstallType_Full;
    
    return true;
}

bool cnmt_get_header_from_file(NcmContentMetaHeader *header_out, NcmContentMetaKey *key_out, FsFile *file, int64_t offset)
{
    if (!header_out || !key_out)
    {
        write_log("missing params in cnmt_get_header_from_file\n");
        return false;
    }

    CnmtHeader_t header = {0};
    size_t size = fs_read_file(&header, sizeof(CnmtHeader_t), offset, FsReadOption_None, file);
    if (size != sizeof(CnmtHeader_t))
    {
        write_log("cnmt header read size mismatch\n");
        return false;
    }

    return cnmt_get_meta_header_and_key(&header, header_out, key_out);
}

bool cnmt_read_data(NcmContentInfo *cnmt_info, FsFile *file, NcmStorageId storage_id, CnmtContentInfos_t *infos)
{
    // main header.
    NcmContentMetaHeader header = {0};
    NcmContentMetaKey key = {0};
    uint64_t offset = 0;

    if (!cnmt_get_header_from_file(&header, &key, file, offset))
    {
        return false;
    }

    // get extended header data.
    void *extended_header = calloc(1, header.extended_header_size);
    if (!extended_header)
    {
        write_log("Failed to get cnmt ext header\n");
        return false;
    }

    offset += fs_read_file(extended_header, header.extended_header_size, offset, FsReadOption_None, file);

    // change the sys / application fw needed for cnmt extended header.
    write_log("settings ext header\n");
    cnmt_set_extended_header(extended_header, key.type);

    // get all the ncaID's, add 1 for cnmt.
    infos->content_infos = calloc((header.content_count + 1), sizeof(NcmContentInfo));
    if (!infos->content_infos)
    {
        write_log("Failed to alloc cnmt infos\n");
        return false;
    }

    // copy info of cnmt...
    memcpy(&infos->content_infos[0], cnmt_info, sizeof(NcmContentInfo));
    infos->info_count = 1;

    for (uint16_t i = 0; i < header.content_count; i++)
    {
        NcmPackagedContentInfo packed_temp = {0};
        offset += fs_read_file(&packed_temp, sizeof(NcmPackagedContentInfo), offset, FsReadOption_None, file);

        // skip deltas...
        if (packed_temp.info.content_type == NcmContentType_DeltaFragment)
        {
            continue;
        }
        
        // if not delta, copy data into struct.
        memcpy(&infos->content_infos[infos->info_count], &packed_temp.info, sizeof(NcmContentInfo));
        infos->info_count++;
    }

    if (!cnmt_set_db(&key, &header, extended_header, infos->content_infos, infos->info_count, storage_id))
    {
        free(extended_header);
        return false;
    }

    if (key.type == NcmContentMetaType_Application || key.type == NcmContentMetaType_Patch || key.type == NcmContentMetaType_AddOnContent)
    {
        return cnmt_push_record(&key, ncm_get_app_id_from_title_id(key.id, key.type), storage_id);
    }

    return true;
}

bool cnmt_open_cnmt_file(FsFileSystem *fs, FsDir *dir, FsFile *file)
{
    FsDirectoryEntry cnmt = {0};
    if (!fs_search_dir_for_file_2(dir, &cnmt, "cnmt"))
    {
        return false;
    }

    if (R_FAILED(fs_open_file(fs, FsOpenMode_Read, file, "/%s", cnmt.name)))
    {
        return false;
    }

    return true;
}

bool cnmt_open(NcmContentId *content_id, NcmStorageId storage_id, CnmtContentInfos_t *infos)
{
    if (!content_id || !infos)
    {
        write_log("missing params in cnmt_open\n");
        return false;
    }

    Result rc = 0;
    NcmContentStorage cs = {0};
    FsFileSystem fs = {0};
    FsDir dir = {0};
    FsFile file = {0};

    // mount the storage.
    write_log("openening ncm storage\n");
    if (!ncm_open_storage(&cs, storage_id))
    {
        return false;
    }

    if (!ncm_check_if_content_id_exists(&cs, content_id))
    {
        write_log("failed to check if content id exists\n");
        ncm_close_storage(&cs);
        return false;
    }

    int64_t cnmt_size = ncm_get_content_id_size(&cs, content_id);
    if (!cnmt_size)
    {
        write_log("failed to get cnmt size\n");
        ncm_close_storage(&cs);
        return false;
    }

    // save the size;
    NcmContentInfo cnmt_info = {0};
    memcpy(&cnmt_info.content_id, content_id, sizeof(NcmContentId));
    memcpy(cnmt_info.size, &cnmt_size, 0x6);
    cnmt_info.content_type = NcmContentType_Meta;

    if (!fs_open_system_with_content_id(&fs, &cs, content_id, FsFileSystemType_ContentMeta))
    {
        ncm_close_storage(&cs);
        return false;
    }

    // no longer need this open.
    ncm_close_storage(&cs);

    // open the mounted cnmt system as a dir.
    write_log("opening cnmt dir\n");
    if (R_FAILED(rc = fs_open_dir(&fs, FsDirOpenMode_ReadFiles | FsDirOpenMode_ReadDirs, &dir, "/")))
    {
        fs_close_system(&fs);
        return false;
    }

    // find the cnmt file inside folder and open it.
    write_log("opening cnmt file\n");
    if (!cnmt_open_cnmt_file(&fs, &dir, &file))
    {
        fs_close_dir(&dir);
        fs_close_system(&fs);
        return false;
    }
    
    // parse the cnmt data.
    bool ret = cnmt_read_data(&cnmt_info, &file, storage_id, infos);

    // close everything.
    fs_close_file(&file);
    fs_close_dir(&dir);
    fs_close_system(&fs);

    if (!ret)
    {
        return false;
    }

    return true;
}