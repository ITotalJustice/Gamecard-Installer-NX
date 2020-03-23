#ifndef _FS_H_
#define _FS_H_

#include <stdbool.h>
#include <stdint.h>
#include <switch.h>


typedef enum
{
    GameCardStoragePartition_Root = 0x0
} GameCardStoragePartition;

typedef struct
{
    uint64_t title_id;
    uint32_t version;
} gamecard_update_partition_info_t;

typedef struct
{
    uint8_t rsa[0x100];
    uint32_t magic;
    uint8_t _0x104[0x4];
    uint8_t kek_index;
    uint8_t _0x109[0x7];
    uint8_t device_id[0x10];
    uint8_t _0x120[0x10];
    uint8_t encrypted_data;
} gamecard_cert_t;

typedef struct
{
    uint8_t version;                 // always 1.
    uint8_t _0x01;                   // padding.
    uint64_t permissions_bitmask;
    uint32_t data_size;
    uint32_t content_owner_id_section_size;
} fs_access_header_t;

typedef struct
{
    uint8_t version;                 // always 1.
    uint8_t _0x01;                   // padding.
    uint64_t permissions_bitmask;
    uint8_t _0xC[0x20];
} fs_access_control_t;


/*
*   FS FILE
*/

// open a file.
Result fs_open_file(FsFileSystem *fs_system, uint32_t mode, FsFile *file, const char *path, ...);

// create a file.
Result fs_create_file(FsFileSystem *fs_system, const char *path, int64_t size, uint32_t option);

// delete a file.
Result fs_delete_file(FsFileSystem *fs_system, const char *path);

//
Result fs_rename_file(FsFileSystem *system, const char *old, const char *new);

// get the size of a file.
int64_t fs_get_file_size(FsFile *file);

//
Result fs_set_file_size(FsFile *file, int64_t size);

// same as fread().
size_t fs_read_file(void *out, uint64_t size, int64_t offset, uint32_t option, FsFile *file);

//
Result fs_write_file(FsFile *file, uint64_t offset, void *out, uint64_t size, uint32_t option);

//
Result fs_flush_file(FsFile *file);

// close file.
void fs_close_file(FsFile *file);


/*
*   FS DIR
*/

// open a dir.
Result fs_open_dir(FsFileSystem *fs_system, uint32_t mode, FsDir *dir, const char *path, ...);

// create a dir.
Result fs_create_dir(FsFileSystem *fs_system, const char *path);

// delete a dir.
Result fs_delete_dir(FsFileSystem *fs_system, const char *path);

// delete a dir recursively.
Result fs_delete_dir_rec(FsFileSystem *fs_system, const char *path);

// similar to while((de = readir(dir)))
int64_t fs_read_dir(FsDir *dir, size_t max_files, FsDirectoryEntry *out);

// get the total of entries in a dir (recursive???).
int64_t fs_get_dir_total(FsDir *dir);

//
int64_t fs_search_dir_for_file(FsDir *dir, const char *file);

//
bool fs_search_dir_for_file_2(FsDir *dir, FsDirectoryEntry *out, const char *file);

//
bool fs_get_file_in_dir_and_open(FsFileSystem *system, FsDir *dir, FsFile *out, const char *file, uint32_t mode);

// close dir.
void fs_close_dir(FsDir *dir);


/*
*   FS SYSTEM
*/

// open file system.
Result fs_open_system(FsFileSystem *fs_system, FsFileSystemType fs_type, const char *path, ...);

//
Result fs_open_system_with_ID(FsFileSystem *system, uint64_t title_ID, FsFileSystemType fs_type, const char *path);

// open file system with ID.
Result fs_open_system_with_patch(FsFileSystem *fs_system, uint64_t title_ID, FsFileSystemType fs_type);

// open the sd card.
Result fs_open_sd_card(FsFileSystem *out, const char *path);

// open the nand.
Result fs_open_nand(FsFileSystem *out, const char *path);

//
bool fs_open_gamecard(const FsGameCardHandle *handle, FsGameCardPartition partition, FsFileSystem *out);

//
int64_t fs_get_system_free_space(FsFileSystem *system, const char *path, ...);

//
int64_t fs_get_total_system_size(FsFileSystem *system, const char *path, ...);

//
int64_t fs_get_sd_card_total_size(void);

//
int64_t fs_get_nand_total_size(void);

// close file system.
void fs_close_system(FsFileSystem *fs_system);


/*
*   FS STORAGE
*/

//
bool fs_open_storage_by_current_process(FsStorage *out);

//
bool fs_open_storage_by_id(FsStorage *out, uint64_t data_id, NcmStorageId storage_id);

//
bool fs_open_gamecard_storage(FsStorage *out, FsGameCardHandle *handle);

//
bool fs_read_storage(FsStorage *storage, void *out, uint64_t size, int64_t offset);

//
bool fs_write_stoarge(FsStorage *storage, const void *in, uint64_t size, int64_t offset);

//
bool fs_flush_storage(FsStorage *storage);

//
int64_t fs_get_storage_size(FsStorage *storage);

//
bool fs_set_storage_size(FsStorage *storage, int64_t size);

//
void fs_close_storage(FsStorage *storage);


/*
*   FS DEVICE OPERATOR
*/

//
bool fs_open_device_operator(FsDeviceOperator *out);

//
bool fs_is_sd_card_inserted(FsDeviceOperator *d);

//
bool fs_is_gamecard_inserted(FsDeviceOperator *d);

//
bool fs_get_gamecard_handle(FsGameCardHandle *out);

//
bool fs_get_gamecard_handle_from_device_operator(FsDeviceOperator *d, FsGameCardHandle *out);

//
uint8_t fs_get_game_card_attribute(FsDeviceOperator *d, const FsGameCardHandle *handle);

//
void fs_close_device_operator(FsDeviceOperator *d);


/*
*   FS MISC
*/

//
bool fs_open_system_with_content_id(FsFileSystem *fs, NcmContentStorage *cs, NcmContentId *content_id, FsFileSystemType type);

// set the archive bit for a folder, to be used with split nsp's.
bool fs_set_archive_bit(const char *path, ...);

// get free size of the sd card.
int64_t fs_get_sd_free_space(void);

// get free size of the nand.
int64_t fs_get_nand_free_space(void);

// get the total size of the selected storage device (sd card / nand).
int64_t fs_get_free_space_from_path(FsFileSystem* fs, const char* path, ...);

// check if exfat is supported.
// returns true if true.
bool fs_is_exfat_supported(void);

// returns the app_id from the given rights_id.
uint64_t fs_get_app_id_from_rights_id(FsRightsId rights_id);

// returns the key_gen from the given rights_id.
uint64_t fs_get_key_gen_from_rights_id(FsRightsId rights_id);

// close gamecard handle.
void fs_close_gamecard_handle(FsGameCardHandle *handle);


/*
*   FS Dev.
*/

//
Result fs_mount_sd_card(void);

//
bool fs_mount_device(const char *name, FsFileSystem fs);

//
bool fs_mount_user(void);

//
bool fs_mount_system(void);

//
bool fs_mount_safe(void);

//
bool fs_mount_gamecard_update(const FsGameCardHandle *handle);

//
bool fs_mount_gamecard_secure(const FsGameCardHandle *handle);

//
bool fs_mount_gamecard_partition(char *out, const FsGameCardHandle *handle, FsGameCardPartition partition);

//
int fs_device_add_path(const char *in_path, char *out_path, FsFileSystem **out_device);

//
int fs_unmount_device(const char *device);

//
bool fs_umount_all_devices(void);


/*
*   IPC functions
*/


//
bool fs_finalise_game_card_driver(FsDeviceOperator *d);

//
bool fs_set_speed_emulation_mode(FsDeviceOperator *d, uint32_t mode);

//
bool fs_get_speed_emulation_mode(FsDeviceOperator *d, uint32_t *mode);

//
bool fs_register_update_partition(void);

//
bool fs_open_registered_partition(FsFileSystem *fs);

//
bool fs_get_game_card_certificate(FsDeviceOperator *d, const FsGameCardHandle *handle, gamecard_cert_t *cert, size_t size);

//
bool fs_get_game_card_update_partition_info(FsDeviceOperator *d, const FsGameCardHandle *handle, gamecard_update_partition_info_t *info);


#endif