#ifndef _NCM_H_
#define _NCM_H_

#include <stdint.h>
#include <stdbool.h>
#include <switch.h>


typedef struct
{
    NcmContentMetaKey metaRecord;
    uint8_t storage_id;
    uint8_t padding[0x7];
} ContentStorageRecord;

typedef struct
{
    NcmContentStorage storage;          // 
    NcmPlaceHolderId placeholder_id;    // placeholder where the nca is written to.
    NcmContentId content_id;
} ncm_install_struct_t;


/*
* standard ncm memes.
*/

//
bool init_ncm(void);

//
void exit_ncm(void);

// open storage frome the given storage ID (nand / sd card).
Result ncm_open_storage(NcmContentStorage *ncm_storage, NcmStorageId fs_storage_ID);

// close storage.
void ncm_close_storage(NcmContentStorage *ncm_storage);

// check if the placeholder exists, should be used to check if needed to attempt to delete said placeholder.
bool ncm_check_if_placeholder_exists(NcmContentStorage *ncm_storage, NcmPlaceHolderId *ncm_placeholder_id);

// create a placeholder ID.
Result ncm_generate_placeholder_id(NcmContentStorage *ncm_storage, NcmPlaceHolderId *placeholder_out);

// create a placeholder.
Result ncm_create_placeholder(NcmContentStorage *ncm_storage, NcmContentId *ncm_content_id, NcmPlaceHolderId *ncm_placeholder_id, uint64_t size);

// get the path of the placeholder.
Result ncm_get_placeholder_path(NcmContentStorage *ncm_storage, NcmPlaceHolderId *ncm_placeholder_id, char *out, size_t size);

// write data to a placeholder, for installing nca's.
Result ncm_write_placeholder(NcmContentStorage *ncm_storage, NcmPlaceHolderId *ncm_placeholder_id, uint64_t offset, void *buf, size_t buf_size);

// delete placeholder, should be used if register fails.
Result ncm_delete_placeholder(NcmContentStorage *ncm_storage, NcmPlaceHolderId *ncm_placeholder_id);

// delete all placeholders, should be added either as an option or run on app startup.
void ncm_delete_all_placeholders(void);

// register a placeholder (remember to clean up after).
Result ncm_register_placeholder(NcmContentStorage *ncm_storage, NcmContentId *ncm_content_id, NcmPlaceHolderId *ncm_placeholder_id);

// check if the nca exists.
bool ncm_check_if_nca_exists(NcmContentStorage *ncm_storage, NcmContentId *ncm_content_id);

// delete registered nca.
Result ncm_delete_nca(NcmContentStorage *ncm_storage, NcmContentId *ncm_content_id);

// get the storage path of an installed nca from the nca_id.
Result ncm_get_storage_path(NcmContentStorage *ncm_storage, char *path_out, NcmContentId *ncm_content_id);

// get size of placeholder.
int64_t ncm_get_placeholder_size(NcmContentStorage *ncm_storage, NcmPlaceHolderId *placeholder_id);

// get the size of installed nca from the given nca_id.
int64_t ncm_get_nca_size(NcmContentStorage *ncm_storage, NcmContentId *ncm_content_id);

// read data from the nca into void *out.
Result ncm_read_nca_file(NcmContentStorage *ncm_storage, void *out, size_t data_size, uint64_t offset, NcmContentId *ncm_content_id);


/*
* ncm_database
*/

// open database.
Result ncm_open_database(NcmContentMetaDatabase *db, NcmStorageId storage_id);

// set the data in the database.
Result ncm_set_database(NcmContentMetaDatabase *db, NcmContentMetaKey *key, void *data, size_t data_size);

// commit changes made to the database.
Result ncm_commit_database(NcmContentMetaDatabase *db);

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
uint64_t ncm_get_app_id_from_title_id(uint64_t title_id, NcmContentMetaType contentMetaType);

#endif