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


void cnmt_push_record(cnmt_header_t *cnmt_header, cnmt_struct_t *cnmt_struct, void *ext_header)
{
    // set up the cnmt header.
    NcmContentMetaHeader ncm_cnmt_header = { cnmt_header->ext_header_size, cnmt_struct->total_cnmt_infos, cnmt_header->content_meta_count, cnmt_header->attributes, 0 };

    // set up the cnmt key.
    NcmContentMetaKey cnmt_key = { cnmt_header->title_id, cnmt_header->title_version, cnmt_header->meta_type, NcmContentInstallType_Full, {0} };

    // set up cnmt record.
    ContentStorageRecord cnmt_storage_record = { cnmt_key, cnmt_struct->storage_id };

    // get the app_id and setup data.
    uint64_t app_id = ncm_get_app_id_from_title_id(cnmt_key.id, cnmt_key.type);
    size_t data_size = sizeof(NcmContentMetaHeader) + ncm_cnmt_header.extended_header_size + (sizeof(NcmContentInfo) * ncm_cnmt_header.content_count);
    uint8_t *data = malloc(data_size);

    // offset of data, start at 0 (of course).
    uint64_t data_offset = 0;

    // store all the data.
    memcpy(&data[data_offset], &ncm_cnmt_header, sizeof(NcmContentMetaHeader));
    memcpy(&data[data_offset += sizeof(NcmContentMetaHeader)], ext_header, ncm_cnmt_header.extended_header_size);
    memcpy(&data[data_offset += ncm_cnmt_header.extended_header_size], cnmt_struct->cnmt_infos, ncm_cnmt_header.content_count * sizeof(NcmContentInfo));

    // set the database.
    NcmContentMetaDatabase db;
    ncm_open_database(&db, cnmt_struct->storage_id);
    ncm_set_database(&db, &cnmt_key, data, data_size);
    ncm_commit_database(&db);
    ncm_close_database(&db);
    free(data);

    uint8_t *app_record = malloc(sizeof(ContentStorageRecord));
    size_t app_record_size = 0;

    // check if any app_cnmt data exists, if so, add it to the start of the app_record buffer.
    uint32_t cnmt_count = ns_count_application_content_meta(app_id);
    if (cnmt_count)
    {
        app_record_size = cnmt_count * sizeof(ContentStorageRecord);
        uint8_t *old_app_buf = malloc(app_record_size);
        memset(old_app_buf, 0, app_record_size);

        ns_list_application_record_content_meta(0, app_id, old_app_buf, app_record_size, cnmt_count);

        app_record = realloc(app_record, app_record_size + sizeof(ContentStorageRecord)); // 
        memcpy(app_record, old_app_buf, app_record_size);
        free(old_app_buf);
    }

    // push record.
    memcpy(&app_record[app_record_size], &cnmt_storage_record, sizeof(ContentStorageRecord));
    ns_delete_application_record(app_id);
    ns_push_application_record(app_id, (ContentStorageRecord *)app_record, app_record_size + sizeof(ContentStorageRecord));
    free(app_record);
}

void *cnmt_set_ext_header(cnmt_struct_t *cnmt_struct, cnmt_header_t cnmt_header, uint64_t *offset)
{
    void *ext_data;

    if (cnmt_header.meta_type == NcmContentMetaType_Application)
    {
        NcmApplicationMetaExtendedHeader ext_header;
        *offset += fs_read_file(&ext_header, sizeof(NcmApplicationMetaExtendedHeader), *offset, FsReadOption_None, &cnmt_struct->cnmt_file);
        ext_header.required_system_version = 0;
        ext_data = malloc(sizeof(NcmApplicationMetaExtendedHeader));
        memcpy(ext_data, &ext_header, sizeof(NcmApplicationMetaExtendedHeader));
    }
    else if (cnmt_header.meta_type == NcmContentMetaType_Patch)
    {
        NcmPatchMetaExtendedHeader ext_header;
        *offset += fs_read_file(&ext_header, sizeof(NcmPatchMetaExtendedHeader), *offset, FsReadOption_None, &cnmt_struct->cnmt_file);
        ext_header.required_system_version = 0;
        ext_data = malloc(sizeof(NcmPatchMetaExtendedHeader));
        memcpy(ext_data, &ext_header, sizeof(NcmPatchMetaExtendedHeader));
    }
    else
    {
        ext_data = malloc(sizeof(NcmAddOnContentMetaExtendedHeader));
        *offset += fs_read_file(ext_data, sizeof(NcmAddOnContentMetaExtendedHeader), *offset, FsReadOption_None, &cnmt_struct->cnmt_file);
    }

    return ext_data;
}

void cnmt_read_data(cnmt_struct_t *cnmt_struct)
{
    cnmt_header_t cnmt_header;
    uint64_t offset = 0;

    // main header.
    offset += fs_read_file(&cnmt_header, sizeof(cnmt_header_t), offset, FsReadOption_None, &cnmt_struct->cnmt_file);

    // get extended header data.
    void *ext_data = cnmt_set_ext_header(cnmt_struct, cnmt_header, &offset);

    // get all the ncaID's, add 1 for cnmt.
    cnmt_struct->cnmt_infos = malloc((cnmt_header.content_count + 1) * sizeof(NcmContentInfo));
    memset(cnmt_struct->cnmt_infos, 0, (cnmt_header.content_count + 1) * sizeof(NcmContentInfo));

    // copy info of cnmt...
    memcpy(&cnmt_struct->cnmt_infos[0], &cnmt_struct->cnmt_info, sizeof(NcmContentInfo));
    cnmt_struct->total_cnmt_infos = 1;

    for (uint32_t i = 0; i < cnmt_header.content_count; i++)
    {
        NcmPackagedContentInfo packed_temp;
        offset += fs_read_file(&packed_temp, sizeof(NcmPackagedContentInfo), offset, FsReadOption_None, &cnmt_struct->cnmt_file);

        // skip deltas...
        if (packed_temp.info.content_type == NcmContentType_DeltaFragment)
            continue;
        
        // if not delta, copy data into struct.
        memcpy(&cnmt_struct->cnmt_infos[cnmt_struct->total_cnmt_infos], &packed_temp.info, sizeof(NcmContentInfo));
        cnmt_struct->total_cnmt_infos++;
    }

    // lets push all the data!
    cnmt_push_record(&cnmt_header, cnmt_struct, ext_data);

    // freeeeeeeeeee.
    free(ext_data);
}

Result cnmt_open_cnmt_file(FsFileSystem *cnmt_system, FsDir *cnmt_dir, cnmt_struct_t *cnmt_struct)
{
    FsDirectoryEntry cnmt;
    if (!fs_search_dir_for_file_2(cnmt_dir, &cnmt, "cnmt"))
        return 1;
    if (R_FAILED(fs_open_file(cnmt_system, FsOpenMode_Read, &cnmt_struct->cnmt_file, "/%s", cnmt.name)))
        return 1;
    return 0;
}

Result cnmt_open(cnmt_struct_t *cnmt_struct)
{
    Result rc = 0;
    NcmContentStorage cnmt_storage = {0};
    FsFileSystem cnmt_system = {0};
    FsDir cnmt_dir;

    cnmt_struct->cnmt_info.content_type = NcmContentType_Meta;
    cnmt_struct->cnmt_info.id_offset = 0;

    // mount the storage.
    if (R_FAILED(rc = ncm_open_storage(&cnmt_storage, cnmt_struct->storage_id)))
        return rc;

    // get the path of the registered cnmt.
    char cnmt_registered_path[0x100] = {0};
    if (R_FAILED(rc = ncm_get_storage_path(&cnmt_storage, cnmt_registered_path, &cnmt_struct->cnmt_info.content_id)))
        return rc;

    // no longer need this open.
    ncm_close_storage(&cnmt_storage);

    // mount the registered cnmt.
    if (R_FAILED(rc = fs_open_system(&cnmt_system, FsFileSystemType_ContentMeta, cnmt_registered_path)))
        return rc;

    // open the mounted cnmt system as a dir.
    if (R_FAILED(rc = fs_open_dir(&cnmt_system, FsDirOpenMode_ReadFiles | FsDirOpenMode_ReadDirs, &cnmt_dir, "/")))
        return rc;

    // find the cnmt file inside folder and open it.
    if (R_FAILED(rc = cnmt_open_cnmt_file(&cnmt_system, &cnmt_dir, cnmt_struct)))
        return rc;

    fs_close_dir(&cnmt_dir);

    *cnmt_struct->cnmt_info.size = fs_get_file_size(&cnmt_struct->cnmt_file);
    
    // parse the cnmt data.
    cnmt_read_data(cnmt_struct);

    // close everything.
    fs_close_file(&cnmt_struct->cnmt_file);
    fs_close_system(&cnmt_system);

    return rc;
}