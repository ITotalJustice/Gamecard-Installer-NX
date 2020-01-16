#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <switch.h>

#include "fs.h"


/*
*   FS FILE
*/

Result fs_open_file(FsFileSystem *system, u32 mode, FsFile *file, const char *path, ...)
{
    char new_path[FS_MAX_PATH];
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    Result rc = fsFsOpenFile(system, new_path, mode, file);
    if (R_FAILED(rc))
        printf("failed to open file %s\n", new_path);
    return rc;
}

Result fs_create_file(FsFileSystem *system, const char *path, int64_t size, u32 option)
{
    Result rc = fsFsCreateFile(system, path, size, option);
    if (R_FAILED(rc))
        printf("failed to create file %s\n", path);
    return rc;
}

Result fs_delete_file(FsFileSystem *system, const char *path)
{
    Result rc = fsFsDeleteFile(system, path);
    if (R_FAILED(rc))
        printf("failed to delete file %s\n", path);
    return rc;
}

Result fs_rename_file(FsFileSystem *system, const char *old, const char *new)
{
    Result rc = fsFsRenameFile(system, old, new);
    if (R_FAILED(rc))
        printf("failed to rename old file %s to %s\n", old, new);
    return rc;
}

int64_t fs_get_file_size(FsFile *file)
{
    int64_t size = 0;
    if (R_FAILED(fsFileGetSize(file, &size)))
        printf("failed to get file size\n");
    return size;
}

Result fs_set_file_size(FsFile *file, int64_t size)
{
    Result rc = fsFileSetSize(file, size);
    if (R_FAILED(rc))
        printf("failed to set file size ...\n");
    return rc;
}

size_t fs_read_file(void *out, uint64_t size, int64_t offset, u32 option, FsFile *file)
{
    size_t total = 0;
    if (R_FAILED(fsFileRead(file, offset, out, size, option, &total)))
        printf("failed to read file\n");
    if (total != size)
        printf("file read missmatch! total = %ld size = %ld\n", total, size);
    return total;
}

Result fs_write_file(FsFile *file, uint64_t offset, void *out, uint64_t size, u32 option)
{
    Result rc = fsFileWrite(file, offset, out, size, option);
    if (R_FAILED(rc))
        printf("failed to write to file ...\n");
    return rc;
}

Result fs_flush_file(FsFile *file)
{
    Result rc = fsFileFlush(file);
    if (R_FAILED(rc))
        printf("failed to flush file ...\n");
    return rc;
}

void fs_close_file(FsFile *file)
{
    fsFileClose(file);
    serviceClose(&file->s);
}


/*
*   FS DIR
*/

Result fs_open_dir(FsFileSystem *system, u32 mode, FsDir *dir, const char *path, ...)
{
    char new_path[FS_MAX_PATH];
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    Result rc = fsFsOpenDirectory(system, new_path, mode, dir);
    if (R_FAILED(rc))
        printf("failed to open dir %s\n", new_path);
    return rc;
}

Result fs_create_dir(FsFileSystem *system, const char *path)
{
    Result rc = fsFsCreateDirectory(system, path);
    if (R_FAILED(rc))
        printf("failed to create dir %s\n", path);
    return rc;
}

Result fs_delete_dir(FsFileSystem *system, const char *path)
{
    Result rc = fsFsDeleteDirectory(system, path);
    if (R_FAILED(rc))
        printf("failed to delete dir %s\n", path);
    return rc;
}

Result fs_delete_dir_rec(FsFileSystem *system, const char *path)
{
    Result rc = fsFsDeleteDirectoryRecursively(system, path);
    if (R_FAILED(rc))
        printf("failed to delete dir recursively %s\n", path);
    return rc;
}

int64_t fs_read_dir(FsDir *dir, size_t max_files, FsDirectoryEntry *out)
{
    int64_t total = 0;
    if (R_FAILED(fsDirRead(dir, &total, max_files, out)))
        printf("failed to read dir\n");
    if (total != max_files)
        printf("number of files read missmatch! total = %ld max_files = %ld\n", total, max_files);
    return total;
}

int64_t fs_get_dir_total(FsDir *dir)
{
    int64_t total = 0;
    if (R_FAILED(fsDirGetEntryCount(dir, &total)))
        printf("failed get total\n");
    return total;
}

int64_t fs_search_dir_for_file(FsDir *dir, const char *file)
{
    for (int64_t i = 0, total = fs_get_dir_total(dir); i < total; i++)
    {
        FsDirectoryEntry entry;
        fs_read_dir(dir, 1, &entry);
        if (strstr(entry.name, file))
            return i;
    }
    return -1;
}

bool fs_search_dir_for_file_2(FsDir *dir, FsDirectoryEntry *out, const char *file)
{
    for (int64_t i = 0, total = fs_get_dir_total(dir); i < total; i++)
    {
        FsDirectoryEntry entry;
        fs_read_dir(dir, 1, &entry);
        if (strstr(entry.name, file))
        {
            memcpy(out, &entry, sizeof(FsDirectoryEntry));
            return true;
        }
    }
    
    printf("couldn't find file %s\n", file);
    return false;
}

bool fs_get_file_in_dir_and_open(FsFileSystem *system, FsDir *dir, FsFile *out, const char *file, uint32_t mode)
{
    FsDirectoryEntry entry;
    if (!fs_search_dir_for_file_2(dir, &entry, file))
        return false;
    if (R_FAILED(fs_open_file(system, mode, out, "/%s", entry.name)))
        return false;
    return true;
}

uint64_t fs_get_dir_total_file(FsDir *dir, const char *file)
{
    uint64_t total = 0;
    for (int64_t i = 0, dir_total = fs_get_dir_total(dir); i < dir_total; i++)
    {
        FsDirectoryEntry entry;
        fs_read_dir(dir, 1, &entry);
        if (strstr(entry.name, file))
            total++;
    }
    return total;
}

uint64_t fs_get_dir_total_file_2(FsDir *dir, FsDirectoryEntry **out, const char *file)
{
    uint64_t total = 0;
    for (int64_t i = 0, dir_total = fs_get_dir_total(dir); i < dir_total; i++)
    {
        FsDirectoryEntry entry;
        fs_read_dir(dir, 1, &entry);
        if (strstr(entry.name, file))
        {
            out[total] = malloc(sizeof(FsDirectoryEntry));  // this probably doesn't work.
            memcpy(out[total], &entry, sizeof(FsDirectoryEntry));
            total++;
        }
    }
    return total;
}

void fs_close_dir(FsDir *dir)
{
    fsDirClose(dir);
    serviceClose(&dir->s);
}


/*
*   FS SYSTEM
*/

Result fs_open_system(FsFileSystem *out, FsFileSystemType fs_type, const char *path, ...)
{
    char new_path[FS_MAX_PATH];
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    Result rc = fsOpenFileSystem(out, fs_type, new_path);
    if (R_FAILED(rc))
        printf("failed to open file system %s\n", new_path);
    return rc;
}

Result fs_open_system_with_id(FsFileSystem *out, uint64_t id, FsFileSystemType fs_type, const char *path)
{
    Result rc = fsOpenFileSystemWithId(out, id, fs_type, path);
    if (R_FAILED(rc))
        printf("failed to open file system with ID %s\n", path);
    return rc;
}

Result fs_open_system_with_patch(FsFileSystem *out, uint64_t id, FsFileSystemType fs_type)
{
    Result rc = fsOpenFileSystemWithPatch(out, id, fs_type);
    if (R_FAILED(rc))
        printf("failed to open file system with patch %ld\n", id);
    return rc;
}

Result fs_open_sd_card(FsFileSystem *out, const char *path)
{
    Result rc = fs_open_system(out, FsContentStorageId_SdCard, path);
    if (R_FAILED(rc))
        printf("failed to open sd card\n");
    return rc;
}

Result fs_open_nand(FsFileSystem *out, const char *path)
{
    Result rc = fs_open_system(out, FsContentStorageId_User, path);
    if (R_FAILED(rc))
        printf("failed to open nand\n");
    return rc;
}

bool fs_open_gamecard(FsGameCardHandle handle, FsGameCardPartition partition, FsFileSystem *out)
{
    Result rc = fsOpenGameCardFileSystem(out, &handle, partition);
    if (R_SUCCEEDED(rc))
        return true;
    printf("failed to open gamecard...\n");
    return false;
}

int64_t fs_get_system_free_space(FsFileSystem *system, const char *path, ...)
{
    char new_path[FS_MAX_PATH];
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    int64_t size = 0;
    Result rc = fsFsGetFreeSpace(system, new_path, &size);
    if (R_FAILED(rc))
        printf("failed to get system free space from path %s\n", new_path);
    return size;
}

int64_t fs_get_sd_free_space(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs;
    if (R_FAILED(rc = fs_open_sd_card(&fs, "/")))
        return size;
    if (R_FAILED(fsFsGetFreeSpace(&fs, "/", &size)))
        printf("failed to get sd card free space\n");
    fs_close_system(&fs);
    return size;
}

int64_t fs_get_nand_free_space(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs;
    if (R_FAILED(rc = fs_open_nand(&fs, "/")))
        return size;
    if (R_FAILED(fsFsGetFreeSpace(&fs, "/", &size)))
        printf("failed to get nand free space\n");
    fs_close_system(&fs);
    return size;
}

int64_t fs_get_sd_card_total_size(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs;
    if (R_FAILED(rc = fs_open_sd_card(&fs, "/")))
        return size;
    if (R_FAILED(rc = fs_get_total_system_size(&fs, "/")))
        return size;
    fs_close_system(&fs);
    return size;
}

int64_t fs_get_nand_total_size(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs;
    if (R_FAILED(rc = fs_open_nand(&fs, "/")))
        return size;
    if (R_FAILED(rc = fs_get_total_system_size(&fs, "/")))
        return size;
    fs_close_system(&fs);
    return size;
}

int64_t fs_get_total_system_size(FsFileSystem *system, const char *path, ...)
{
    char new_path[FS_MAX_PATH];
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    int64_t size = 0;
    Result rc = fsFsGetTotalSpace(system, new_path, &size);
    if (R_FAILED(rc))
        printf("failed to get total system size from path %s\n", new_path);
    return size;
}

void fs_close_system(FsFileSystem *system)
{
    fsFsClose(system);
    serviceClose(&system->s);
}


/*
*   FS STORAGE
*/

Result fs_open_storage_by_current_process(FsStorage *out)
{
    Result rc = fsOpenDataStorageByCurrentProcess(out);
    if (R_FAILED(rc))
        printf("failed to open storage by current process\n");
    return rc;
}

Result fs_open_storage_by_id(FsStorage *out, u_int64_t data_id, NcmStorageId storage_id)
{
    Result rc = fsOpenDataStorageByDataId(out, data_id, storage_id);
    if (R_FAILED(rc))
        printf("failed to open stoarge using data id %lu\n", data_id);
    return rc;
}

bool fs_open_gamecard_storage(FsStorage *out, FsGameCardHandle handle)
{
    const struct
    {
        FsGameCardHandle handle;
        uint32_t partition;
    } in = { handle, GameCardStoragePartition_Root };

    // first one will always fail.
    // i assume its because of how the gamecard is already mounted in hos.
    // the first time fails but corrupts the current mount partition in hos.
    // the second works because it is nolonger properly mounted in hos anymore.
    // this explains why you need to eject the gc after moutning it this way.
    // unmounting the partition will now fix this as hos will not try and remount to fix it (theres no reason it usually ever should).
    // idk how to fix this though, not that much is know on the gamecard.
    // i'd like to try and work around opening the storage.
    serviceDispatchIn(fsGetServiceSession(), 30, in, .out_num_objects = 1, .out_objects = &out->s);
    if (R_FAILED(serviceDispatchIn(fsGetServiceSession(), 30, in, .out_num_objects = 1, .out_objects = &out->s)))
        return false;
    return true;
}

Result fs_read_storage(FsStorage *storage, void *out, uint64_t size, int64_t offset)
{
    Result rc = fsStorageRead(storage, offset, out, size);
    if (R_FAILED(rc))
        printf("failed to read storage...\n");
    return rc;
}

Result fs_write_storage(FsStorage *storage, const void *in, uint64_t size, int64_t offset)
{
    Result rc = fsStorageWrite(storage, offset, in, size);
    if (R_FAILED(rc))
        printf("failed to write to storage...\n");
    return rc;
}

Result fs_flush_storage(FsStorage *storage)
{
    Result rc = fsStorageFlush(storage);
    if (R_FAILED(rc))
        printf("failed to flush storage...\n");
    return rc;
}

int64_t fs_get_storage_size(FsStorage *storage)
{
    int64_t size = 0;
    if (R_FAILED(fsStorageGetSize(storage, &size)))
        printf("failed to get storage size...\n");
    return size;
}

Result fs_set_storage_size(FsStorage *storage, int64_t size)
{
    Result rc = fsStorageSetSize(storage, size);
    if (R_FAILED(rc))
        printf("failed to set storage size to %ld...\n", size);
    return rc;
}

void fs_close_storage(FsStorage *storage)
{
    fsStorageClose(storage);
    serviceClose(&storage->s);
}


/*
*   FS DEVICE OPERATOR
*/

Result fs_open_device_operator(FsDeviceOperator *out)
{
    Result rc = fsOpenDeviceOperator(out);
    if (R_FAILED(rc))
        printf("failed to open device operator...\n");
    return rc;
}

bool fs_is_sdcard_inserted(FsDeviceOperator *d)
{
    bool inserted = false;
    if (R_FAILED(fsDeviceOperatorIsSdCardInserted(d, &inserted)))
        printf("failed to check if sd card is inserted...\n");
    return inserted;
}

bool fs_is_gamecard_inserted(FsDeviceOperator *d)
{
    bool inserted = false;
    if (R_FAILED(fsDeviceOperatorIsGameCardInserted(d, &inserted)))
        printf("failed to check if sd card is inserted...\n");
    return inserted;
}

bool fs_get_gamecard_handle(FsGameCardHandle *out)
{
    FsDeviceOperator devop;
    Result rc = 0;

    rc = fsOpenDeviceOperator(&devop);
    if (R_FAILED(rc))
        return false;
    
    rc = fsDeviceOperatorGetGameCardHandle(&devop, out);
    fsDeviceOperatorClose(&devop);

    return R_FAILED(rc) ? false : true;
}

Result fs_get_gamecard_handle_from_device_operator(FsDeviceOperator *d, FsGameCardHandle *out)
{
    Result rc = fsDeviceOperatorGetGameCardHandle(d, out);
    if (R_FAILED(rc))
        printf("failed to get gamecard handle...\n");
    return rc;
}

uint8_t fs_get_gamecard_attribute(FsDeviceOperator *d, const FsGameCardHandle *handle)
{
    uint8_t attribute = 0;
    if (R_FAILED(fsDeviceOperatorGetGameCardAttribute(d, handle, &attribute)))
        printf("failed to get game card attribute...\n");
    return attribute;
}

void fs_close_device_operator(FsDeviceOperator *d)
{
    fsDeviceOperatorClose(d);
    serviceClose(&d->s);
}


/*
*   FS MISC
*/

Result fs_set_archive_bit(const char *path, ...)
{
    char new_path[FS_MAX_PATH];
    va_list v;
    va_start(v, path);
    vsprintf(new_path, path, v);
    va_end(v);

    Result rc = 0;
    FsFileSystem fs;
    if (R_FAILED(rc = fs_open_sd_card(&fs, "/")))
        return rc;
    if (R_FAILED(rc = fsFsSetConcatenationFileAttribute(&fs, new_path)))
        printf("failed to set archive bit for %s\n", new_path);
    fs_close_system(&fs);
    return rc;
}

bool fs_is_exfat_supported(void)
{
    bool supported = 0;
    if (R_FAILED(fsIsExFatSupported(&supported)))
        printf("failed to check if exfat is supported...\n");
    return supported;
}

uint64_t fs_get_app_id_from_rights_id(FsRightsId rights_id)
{
    return __bswap64(*(uint64_t *)rights_id.c);
}

uint64_t fs_get_key_gen_from_rights_id(FsRightsId rights_id)
{
    return __bswap64(*(uint64_t *)(rights_id.c + 0x8));
}

void fs_close_gamecard_handle(FsGameCardHandle *handle)
{
    svcCloseHandle(handle->value);
}


/*
*   IPC srv
*/

bool fs_open_sd_card_event_notifier(FsEventNotifier *out)
{
    Result rc = serviceDispatch(fsGetServiceSession(), 500, .out_num_objects = 1, .out_objects = &out->s);
    if (R_SUCCEEDED(rc))
        return true;
    printf("sdcard event %08X\n", rc);
    return false;
}

bool fs_open_game_card_event_notifier(FsEventNotifier *out)
{
    Result rc = serviceDispatch(fsGetServiceSession(), 501, .out_num_objects = 1, .out_objects = &out->s);
    if (R_SUCCEEDED(rc))
        return true;
    printf("gamecard event %08X\n", rc);
    return false;
}

bool fs_open_sys_update_notifier(FsEventNotifier *out)
{
    if (!hosversionAtLeast(5, 0, 0))
        return false;

    Result rc = serviceDispatch(fsGetServiceSession(), 510, .out_num_objects = 1, .out_objects = &out->s);
    if (R_SUCCEEDED(rc))
        return true;
    printf("sdcard event %08X\n", rc);
    return false;
}

bool fs_notify_sys_update_event()
{
    if (!hosversionAtLeast(5, 0, 0))
        return false;
    
    //serviceDispatch(fsGetServiceSession(), 511); // not tested.
    return false;
}


/*
*   FS Dev.
*/

Result fs_mount_sd_card(void)
{
    Result rc = fsdevMountSdmc();
    if (R_FAILED(rc))
        printf("failed to mount sd card");
    return rc;
}

int fs_mount_device(const char *name, FsFileSystem fs)
{
    int rc = fsdevMountDevice(name, fs);
    if (rc == -1)
        printf("failed to mount device %s", name);
    return rc;
}

bool fs_mount_gamecard_partition(char *out, const FsGameCardHandle handle, FsGameCardPartition partition)
{
    if (out == NULL)
        return false;

    if (partition > FsGameCardPartition_Secure) // cannot mount logo.
        return false;

    FsFileSystem fs;
    if (!fs_open_gamecard(handle, partition, &fs))
        return false;
    
    const char partition_letters[] = { 'U', 'N', 'S' };
    snprintf(out, 0x10, "@Gc%c%08x", partition_letters[partition], handle.value);
    
    if (fsdevMountDevice(out, fs) != -1)
        return true;

    printf("failed to mount device %s\n", out);
    fsFsClose(&fs);
    return false;
}

int fs_device_add_path(const char *in_path, char *out_path, FsFileSystem **out_device)
{
    return fsdevTranslatePath(in_path, out_device, out_path);
}

int fs_unmount_device(const char *device)
{
    // idk what num it returns yet.
    return fsdevUnmountDevice(device);
}

Result fs_umount_all_devices(void)
{
    Result rc = fsdevUnmountAll();
    if (R_FAILED(rc))
        printf("failed to unmount all devices");
    return rc;
}


/*
*   IPC device op.
*/

void fs_get_game_card_update_partition_info(FsDeviceOperator *d, const FsGameCardHandle handle)
{
    //203.
}

void fs_get_game_card_certificate(FsDeviceOperator *d, const FsGameCardHandle handle)
{
    //206.
}

void fs_get_game_card_asic_info(FsDeviceOperator *d, const FsGameCardHandle *handle)
{
    //207.
}

void fs_get_game_card_id_set(FsDeviceOperator *d)
{
    //208.
}

void fs_get_game_card_image_hash(FsDeviceOperator *d)
{
    //211.
}

void fs_get_game_card_device_id_for_prod_card(FsDeviceOperator *d, const FsGameCardHandle *handle)
{
    if (!hosversionAtLeast(2, 0, 0))
        return;

    //212.
}

void fs_get_game_card_error_info(FsDeviceOperator *d)
{
    if (!hosversionAtLeast(2, 0, 0))
        return;
    //215.
}

void fs_get_game_card_error_report_info(FsDeviceOperator *d)
{
    if (!hosversionAtLeast(2, 1, 0))
        return;
    //216.
}

void fs_get_game_card_device_id(FsDeviceOperator *d, const FsGameCardHandle *handle)
{
    if (!hosversionAtLeast(3, 0, 0))
        return;
    //212.
}

void fs_get_gc_compatability_type()
{
    if (!hosversionAtLeast(9, 0, 0))
        return;
    //220.
}