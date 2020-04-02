#ifndef _NCM_H_
#define _NCM_H_

#include <stdint.h>
#include <stdbool.h>
#include <switch.h>


typedef struct
{
    NcmContentMetaKey key;
    uint8_t storage_id;
    uint8_t padding[0x7];
} NcmContentStorageRecord;

typedef struct
{
    char major[0x3];
    char minor[0x3];
    char macro[0x3];
    char bug_fix[0x4];
} NcmVersionString_t;

typedef struct
{
    NcmContentStorage storage;          // 
    NcmPlaceHolderId placeholder_id;    // placeholder_id where the nca is written to.
    NcmContentId content_id;
} NcmInstall_t;


/*
* standard ncm memes.
*/

//
bool ncm_init(void);

//
void ncm_exit(void);

// open storage frome the given storage ID (nand / sd card).
bool ncm_open_storage(NcmContentStorage *cs, NcmStorageId fs_storage_ID);

// close storage.
void ncm_close_storage(NcmContentStorage *cs);

// check if the placeholder_id exists, should be used to check if needed to attempt to delete said placeholder_id.
bool ncm_check_if_placeholder_id_exists(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id);

// create a placeholder_id.
bool ncm_generate_placeholder_id(NcmContentStorage *cs, NcmPlaceHolderId *placeholder_out);

// create a placeholder_id.
bool ncm_create_placeholder_id(NcmContentStorage *cs, const NcmContentId *content_id, const NcmPlaceHolderId *placeholder_id, int64_t size);

// get the path of the placeholder_id.
bool ncm_get_placeholder_path_id(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id, char *out, size_t size);

// write data to a placeholder_id, for installing nca's.
bool ncm_write_placeholder_id(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id, uint64_t offset, const void *data, size_t data_size);

// delete placeholder_id, should be used if register fails.
bool ncm_delete_placeholder_id(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id);

// delete all placeholders, should be added either as an option or run on app startup.
void ncm_delete_all_placeholders_id(void);

// register a placeholder_id (remember to clean up after).
bool ncm_register_placeholder_id(NcmContentStorage *cs, const NcmContentId *content_id, const NcmPlaceHolderId *placeholder_id);

// check if the nca exists.
bool ncm_check_if_content_id_exists(NcmContentStorage *cs, const NcmContentId *content_id);

// delete registered nca.
bool ncm_delete_content_id(NcmContentStorage *cs, const NcmContentId *content_id);

// get the storage path of an installed nca from the nca_id.
bool ncm_get_content_id_path(NcmContentStorage *cs, char *path_out, size_t size, const NcmContentId *content_id);

// get size of placeholder_id.
int64_t ncm_get_placeholder_id_size(NcmContentStorage *cs, const NcmPlaceHolderId *placeholder_id);

// get the size of installed nca from the given nca_id.
int64_t ncm_get_content_id_size(NcmContentStorage *cs, const NcmContentId *content_id);

// read data from the nca into void *out.
bool ncm_read_content_id(NcmContentStorage *cs, void *data, size_t data_size, uint64_t offset, const NcmContentId *content_id);


/*
* ncm_database
*/

// open database.
bool ncm_open_database(NcmContentMetaDatabase *db, NcmStorageId storage_id);

// set the data in the database.
bool ncm_set_database(NcmContentMetaDatabase *db, const NcmContentMetaKey *key, const void *data, size_t data_size);

// commit changes made to the database.
bool ncm_commit_database(NcmContentMetaDatabase *db);

// close the database.
void ncm_close_database(NcmContentMetaDatabase *db);


/*
* misc
*/

// get the free size of the selected storage device (sd card / nand).
int64_t ncm_get_storage_free_space(NcmStorageId storage_id);

// get the total size of the selected storage device (sd card / nand).
int64_t ncm_get_storage_total_size(NcmStorageId storage_id);

//
int32_t ncm_get_application_meta_total(NcmContentMetaDatabase *db, NcmContentMetaType type);

//
int32_t ncm_get_meta_total(NcmContentMetaDatabase *db, NcmContentMetaType type);

//
bool ncm_get_latest_key(NcmContentMetaDatabase *db, NcmContentMetaKey *key_out, uint64_t id);

//
bool ncm_get_latest_key2(NcmContentMetaKey *out_key, uint64_t id); // temp name

//
bool ncm_is_key_newer(const NcmContentMetaKey *key);


/*
*   Ncm Utils
*/

//
uint64_t ncm_get_app_id_from_title_id(uint64_t title_id, NcmContentMetaType type);
void ncm_get_version_string(uint32_t version, NcmVersionString_t *out);

//
size_t ncm_calculate_content_info_size(const NcmContentInfo *info);
size_t ncm_calculate_content_infos_size(const NcmContentInfo *infos, uint16_t count);
bool ncm_get_content_type_from_infos(NcmContentInfo *infos, uint16_t count, NcmContentInfo *found, NcmContentType wanted_type);

//
const char *ncm_get_storage_id_string(NcmStorageId storage_id);
const char *ncm_get_meta_type_string(NcmContentMetaType type);
const char *ncm_get_content_type_string(NcmContentType type);
const char *ncm_get_attribute_string(NcmContentMetaAttribute attribute);
const char *ncm_get_install_type_string(NcmContentInstallType type);


/*
*   Debug
*/

void ncm_print_key(const NcmContentMetaKey *key);
void ncm_print_keys(const NcmContentMetaKey *key, uint16_t count);
void ncm_print_application_record(const NcmContentStorageRecord *record);
void ncm_print_application_records(const NcmContentStorageRecord *records, uint32_t count);

#endif