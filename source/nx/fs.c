#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <switch.h>

#include "nx/fs.h"
#include "nx/ncm.h"
#include "util/log.h"


/*
*   FS FILE
*/

Result fs_open_file(FsFileSystem *system, u32 mode, FsFile *file, const char *path, ...)
{
    char new_path[FS_MAX_PATH] = {0};
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    Result rc = fsFsOpenFile(system, new_path, mode, file);
    if (R_FAILED(rc))
    {
        write_log("failed to open file %s\n", new_path);
    }
    return rc;
}

Result fs_create_file(FsFileSystem *system, const char *path, int64_t size, u32 option)
{
    Result rc = fsFsCreateFile(system, path, size, option);
    if (R_FAILED(rc))
    {
        write_log("failed to create file %s\n", path);
    }
    return rc;
}

Result fs_delete_file(FsFileSystem *system, const char *path)
{
    Result rc = fsFsDeleteFile(system, path);
    if (R_FAILED(rc))
    {
        write_log("failed to delete file %s\n", path);
    }
    return rc;
}

Result fs_rename_file(FsFileSystem *system, const char *old, const char *new)
{
    Result rc = fsFsRenameFile(system, old, new);
    if (R_FAILED(rc))
    {
        write_log("failed to rename old file %s to %s\n", old, new);
    }
    return rc;
}

int64_t fs_get_file_size(FsFile *file)
{
    int64_t size = 0;
    if (R_FAILED(fsFileGetSize(file, &size)))
    {
        write_log("failed to get file size\n");
    }
    return size;
}

Result fs_set_file_size(FsFile *file, int64_t size)
{
    Result rc = fsFileSetSize(file, size);
    if (R_FAILED(rc))
    {
        write_log("failed to set file size ...\n");
    }
    return rc;
}

size_t fs_read_file(void *out, uint64_t size, int64_t offset, u32 option, FsFile *file)
{
    size_t total = 0;
    if (R_FAILED(fsFileRead(file, offset, out, size, option, &total)))
    {
        write_log("failed to read file\n");
    }
    if (total != size)
    {
        write_log("file read missmatch! total = %ld size = %ld\n", total, size);
    }
    return total;
}

Result fs_write_file(FsFile *file, uint64_t offset, void *out, uint64_t size, u32 option)
{
    Result rc = fsFileWrite(file, offset, out, size, option);
    if (R_FAILED(rc))
    {
        write_log("failed to write to file ...\n");
    }
    return rc;
}

Result fs_flush_file(FsFile *file)
{
    Result rc = fsFileFlush(file);
    if (R_FAILED(rc))
    {
        write_log("failed to flush file ...\n");
    }
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
    char new_path[FS_MAX_PATH] = {0};
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    Result rc = fsFsOpenDirectory(system, new_path, mode, dir);
    if (R_FAILED(rc))
    {
        write_log("failed to open dir %s\n", new_path);
    }
    return rc;
}

Result fs_create_dir(FsFileSystem *system, const char *path)
{
    Result rc = fsFsCreateDirectory(system, path);
    if (R_FAILED(rc))
    {
        write_log("failed to create dir %s\n", path);
    }
    return rc;
}

Result fs_delete_dir(FsFileSystem *system, const char *path)
{
    Result rc = fsFsDeleteDirectory(system, path);
    if (R_FAILED(rc))
    {
        write_log("failed to delete dir %s\n", path);
    }
    return rc;
}

Result fs_delete_dir_rec(FsFileSystem *system, const char *path)
{
    Result rc = fsFsDeleteDirectoryRecursively(system, path);
    if (R_FAILED(rc))
    {
        write_log("failed to delete dir recursively %s\n", path);
    }
    return rc;
}

int64_t fs_read_dir(FsDir *dir, size_t max_files, FsDirectoryEntry *out)
{
    int64_t total = 0;
    if (R_FAILED(fsDirRead(dir, &total, max_files, out)))
    {
        write_log("failed to read dir\n");
    }

    if (total != max_files)
    {
        write_log("number of files read missmatch! total = %ld max_files = %ld\n", total, max_files);
    }

    return total;
}

int64_t fs_get_dir_total(FsDir *dir)
{
    int64_t total = 0;
    if (R_FAILED(fsDirGetEntryCount(dir, &total)))
    {
        write_log("failed get total\n");
    }
    return total;
}

int64_t fs_search_dir_for_file(FsDir *dir, const char *file)
{
    for (int64_t i = 0, total = fs_get_dir_total(dir); i < total; i++)
    {
        FsDirectoryEntry entry = {0};
        fs_read_dir(dir, 1, &entry);
        if (strstr(entry.name, file))
        {
            return i;
        }
    }
    return -1;
}

bool fs_search_dir_for_file_2(FsDir *dir, FsDirectoryEntry *out, const char *file)
{
    for (int64_t i = 0, total = fs_get_dir_total(dir); i < total; i++)
    {
        FsDirectoryEntry entry = {0};
        fs_read_dir(dir, 1, &entry);
        if (strstr(entry.name, file))
        {
            memcpy(out, &entry, sizeof(FsDirectoryEntry));
            return true;
        }
    }
    
    write_log("couldn't find file %s\n", file);
    return false;
}

bool fs_get_file_in_dir_and_open(FsFileSystem *system, FsDir *dir, FsFile *out, const char *file, uint32_t mode)
{
    FsDirectoryEntry entry = {0};
    if (!fs_search_dir_for_file_2(dir, &entry, file))
    {
        return false;
    }

    if (R_FAILED(fs_open_file(system, mode, out, "/%s", entry.name)))
    {
        return false;
    }

    return true;
}

uint64_t fs_get_dir_total_file(FsDir *dir, const char *file)
{
    uint64_t total = 0;
    for (int64_t i = 0, dir_total = fs_get_dir_total(dir); i < dir_total; i++)
    {
        FsDirectoryEntry entry = {0};
        fs_read_dir(dir, 1, &entry);
        if (strstr(entry.name, file))
        {
            total++;
        }
    }
    return total;
}

uint64_t fs_get_dir_total_file_2(FsDir *dir, FsDirectoryEntry **out, const char *file)
{
    uint64_t total = 0;
    for (int64_t i = 0, dir_total = fs_get_dir_total(dir); i < dir_total; i++)
    {
        FsDirectoryEntry entry = {0};
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
    char new_path[FS_MAX_PATH] = {0};
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    Result rc = fsOpenFileSystem(out, fs_type, new_path);
    if (R_FAILED(rc))
    {
        write_log("failed to open file system %s\n", new_path);
    }
    return rc;
}

Result fs_open_system_with_id(FsFileSystem *out, uint64_t id, FsFileSystemType fs_type, const char *path)
{
    Result rc = fsOpenFileSystemWithId(out, id, fs_type, path);
    if (R_FAILED(rc))
    {
        write_log("failed to open file system with ID %s\n", path);
    }
    return rc;
}

Result fs_open_system_with_patch(FsFileSystem *out, uint64_t id, FsFileSystemType fs_type)
{
    Result rc = fsOpenFileSystemWithPatch(out, id, fs_type);
    if (R_FAILED(rc))
    {
        write_log("failed to open file system with patch %ld\n", id);
    }
    return rc;
}

Result fs_open_sd_card(FsFileSystem *out, const char *path)
{
    Result rc = fs_open_system(out, FsContentStorageId_SdCard, path);
    if (R_FAILED(rc))
    {
        write_log("failed to open sd card\n");
    }
    return rc;
}

Result fs_open_nand(FsFileSystem *out, const char *path)
{
    Result rc = fs_open_system(out, FsContentStorageId_User, path);
    if (R_FAILED(rc))
    {
        write_log("failed to open nand\n");
    }
    return rc;
}

Result fs_open_nand_partition(FsFileSystem *out, FsBisPartitionId partition)
{
    Result rc = fsOpenBisFileSystem(out, partition, ""); // what is the string for?
    if (R_FAILED(rc))
    {
        write_log("failed to open nand filesystem partition: %u partition\n");
    }
    return rc;
}

bool fs_open_gamecard(const FsGameCardHandle *handle, FsGameCardPartition partition, FsFileSystem *out)
{
    Result rc = fsOpenGameCardFileSystem(out, handle, partition);
    if (R_FAILED(rc))
    {
        write_log("failed to open gamecard...\n");
        return false;
    }
    return true;
}

int64_t fs_get_system_free_space(FsFileSystem *system, const char *path, ...)
{
    char new_path[FS_MAX_PATH] = {0};
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    int64_t size = 0;
    Result rc = fsFsGetFreeSpace(system, new_path, &size);
    if (R_FAILED(rc))
    {
        write_log("failed to get system free space from path %s\n", new_path);
    }
    return size;
}

int64_t fs_get_sd_free_space(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs = {0};

    rc = fs_open_sd_card(&fs, "/");
    if (R_FAILED(rc))
    {
        return size;
    }

    rc = fsFsGetFreeSpace(&fs, "/", &size);
    if (R_FAILED(rc))
    {
        write_log("failed to get sd card free space\n");
    }

    fs_close_system(&fs);
    return size;
}

int64_t fs_get_nand_free_space(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs = {0};

    rc = fs_open_nand(&fs, "/");
    if (R_FAILED(rc))
    {
        return 0;
    }

    rc = fsFsGetFreeSpace(&fs, "/", &size);
    if (R_FAILED(rc))
    {
        fs_close_system(&fs);
        write_log("failed to get nand free space\n");
        return 0;
    }

    fs_close_system(&fs);
    return size;
}

int64_t fs_get_sd_card_total_size(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs = {0};

    rc = fs_open_sd_card(&fs, "/");
    if (R_FAILED(rc))
    {
        return 0;
    }

    size = fs_get_total_system_size(&fs, "/");
    fs_close_system(&fs);
    return size;
}

int64_t fs_get_nand_total_size(void)
{
    Result rc = 0;
    int64_t size = 0;
    FsFileSystem fs = {0};

    rc = fs_open_nand(&fs, "/");
    if (R_FAILED(rc))
    {
        return size;
    }

    size = fs_get_total_system_size(&fs, "/");
    fs_close_system(&fs);
    return size;
}

int64_t fs_get_total_system_size(FsFileSystem *system, const char *path, ...)
{
    char new_path[FS_MAX_PATH] = {0};
    va_list arg;
    va_start(arg, path);
    vsprintf(new_path, path, arg);
    va_end(arg);

    int64_t size = 0;
    Result rc = fsFsGetTotalSpace(system, new_path, &size);
    if (R_FAILED(rc))
    {
        write_log("failed to get total system size from path %s\n", new_path);
        return 0;
    }
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

bool fs_open_storage_by_current_process(FsStorage *out)
{
    Result rc = fsOpenDataStorageByCurrentProcess(out);
    if (R_FAILED(rc))
    {
        write_log("failed to open storage by current process\n");
        return false;
    }
    return true;
}

bool fs_open_storage_by_id(FsStorage *out, uint64_t data_id, NcmStorageId storage_id)
{
    Result rc = fsOpenDataStorageByDataId(out, data_id, storage_id);
    if (R_FAILED(rc))
    {
        write_log("failed to open stoarge using data id %lu\n", data_id);
        return false;
    }
    return true;
}

bool fs_open_gamecard_storage(FsStorage *out, FsGameCardHandle *handle)
{
    const struct
    {
        FsGameCardHandle handle;
        uint32_t partition;
    } in = { *handle, GameCardStoragePartition_Root };

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

bool fs_read_storage(FsStorage *storage, void *out, uint64_t size, int64_t offset)
{
    Result rc = fsStorageRead(storage, offset, out, size);
    if (R_FAILED(rc))
    {
        write_log("failed to read storage...\n");
        return false;
    }
    return true;
}

bool fs_write_storage(FsStorage *storage, const void *in, uint64_t size, int64_t offset)
{
    Result rc = fsStorageWrite(storage, offset, in, size);
    if (R_FAILED(rc))
    {
        write_log("failed to write to storage...\n");
        return false;
    }
    return true;
}

bool fs_flush_storage(FsStorage *storage)
{
    Result rc = fsStorageFlush(storage);
    if (R_FAILED(rc))
    {
        write_log("failed to flush storage...\n");
        return false;
    }
    return true;
}

int64_t fs_get_storage_size(FsStorage *storage)
{
    int64_t size = 0;
    Result rc = fsStorageGetSize(storage, &size);
    if (R_FAILED(rc))
    {
        write_log("failed to get storage size...\n");
        return 0;
    }
    return size;
}

bool fs_set_storage_size(FsStorage *storage, int64_t size)
{
    Result rc = fsStorageSetSize(storage, size);
    if (R_FAILED(rc))
    {
        write_log("failed to set storage size to %ld...\n", size);
        return false;
    }
    return true;
}

void fs_close_storage(FsStorage *storage)
{
    fsStorageClose(storage);
    serviceClose(&storage->s);
}


/*
*   FS DEVICE OPERATOR
*/

bool fs_open_device_operator(FsDeviceOperator *out)
{
    Result rc = fsOpenDeviceOperator(out);
    if (R_FAILED(rc))
    {
        write_log("failed to open device operator...\n");
        return false;
    }
    return true;
}

bool fs_is_sdcard_inserted(FsDeviceOperator *d)
{
    bool inserted = false;
    if (R_FAILED(fsDeviceOperatorIsSdCardInserted(d, &inserted)))
    {
        write_log("failed to check if sd card is inserted...\n");
        return false;
    }
    return inserted;
}

bool fs_is_gamecard_inserted(FsDeviceOperator *d)
{
    bool inserted = false;
    if (R_FAILED(fsDeviceOperatorIsGameCardInserted(d, &inserted)))
    {
        write_log("failed to check if sd card is inserted...\n");
        return false;
    }
    return inserted;
}

bool fs_get_gamecard_handle(FsGameCardHandle *out)
{
    FsDeviceOperator devop = {0};
    Result rc = 0;

    rc = fsOpenDeviceOperator(&devop);
    if (R_FAILED(rc))
    {
        return false;
    }
    
    rc = fsDeviceOperatorGetGameCardHandle(&devop, out);
    fsDeviceOperatorClose(&devop);

    if (R_FAILED(rc))
    {
        write_log("Failed to get gamecard handle\n");
        return false;
    }

    return true;
}

bool fs_get_gamecard_handle_from_device_operator(FsDeviceOperator *d, FsGameCardHandle *out)
{
    Result rc = fsDeviceOperatorGetGameCardHandle(d, out);
    if (R_FAILED(rc))
    {
        write_log("failed to get gamecard handle...\n");
        return false;
    }
    return true;
}

uint8_t fs_get_gamecard_attribute(FsDeviceOperator *d, const FsGameCardHandle *handle)
{
    uint8_t attribute = 0;
    if (R_FAILED(fsDeviceOperatorGetGameCardAttribute(d, handle, &attribute)))
    {
        write_log("failed to get game card attribute...\n");
        return 0;
    }
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

bool fs_open_system_with_content_id(FsFileSystem *fs, NcmContentStorage *cs, NcmContentId *content_id, FsFileSystemType type)
{
    if (!fs || !cs || !content_id)
    {
        write_log("missing params in fs_open_system_with_content_id\n");
        return false;
    }

    if (!ncm_check_if_content_id_exists(cs, content_id))
    {
        return false;
    }

    char path[FS_MAX_PATH] = {0};
    if (!ncm_get_content_id_path(cs, path, FS_MAX_PATH, content_id))
    {
        return false;
    }

    if (!fs_open_system(fs, type, path))
    {
        return false;
    }

    return true;
}

bool fs_set_archive_bit(const char *path, ...)
{
    char new_path[FS_MAX_PATH] = {0};
    va_list v;
    va_start(v, path);
    vsprintf(new_path, path, v);
    va_end(v);

    Result rc = 0;
    FsFileSystem fs = {0};

    rc = fs_open_sd_card(&fs, "/");
    if (R_FAILED(rc))
    {
        return false;
    }

    rc = fsFsSetConcatenationFileAttribute(&fs, new_path);
    if (R_FAILED(rc))
    {
        write_log("failed to set archive bit for %s\n", new_path);
        fs_close_system(&fs);
        return false;
    }

    fs_close_system(&fs);
    return rc;
}

bool fs_is_exfat_supported(void)
{
    bool supported = 0;
    if (R_FAILED(fsIsExFatSupported(&supported)))
        write_log("failed to check if exfat is supported...\n");
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
    write_log("sdcard event %08X\n", rc);
    return false;
}

bool fs_open_game_card_event_notifier(FsEventNotifier *out)
{
    Result rc = serviceDispatch(fsGetServiceSession(), 501, .out_num_objects = 1, .out_objects = &out->s);
    if (R_SUCCEEDED(rc))
        return true;
    write_log("gamecard event %08X\n", rc);
    return false;
}

bool fs_open_sys_update_notifier(FsEventNotifier *out)
{
    if (!hosversionAtLeast(5, 0, 0))
        return false;

    Result rc = serviceDispatch(fsGetServiceSession(), 510, .out_num_objects = 1, .out_objects = &out->s);
    if (R_SUCCEEDED(rc))
        return true;
    write_log("sdcard event %08X\n", rc);
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
        write_log("failed to mount sd card");
    return rc;
}

bool fs_mount_device(const char *name, FsFileSystem fs)
{
    int rc = fsdevMountDevice(name, fs);
    if (rc == -1)
    {
        write_log("failed to mount device %s", name);
        return false;
    }
    return true;
}

bool fs_mount_user(void)
{
    FsFileSystem fs = {0};
    if (R_FAILED(fs_open_nand_partition(&fs, FsBisPartitionId_User)))
    {
        return false;
    }

    if (!fs_mount_device("@User", fs))
    {
        fsFsClose(&fs);
        return false;
    }
    
    return true;
}

bool fs_mount_system(void)
{
    FsFileSystem fs = {0};
    if (R_FAILED(fs_open_nand_partition(&fs, FsBisPartitionId_System)))
    {
        return false;
    }

    if (!fs_mount_device("@System", fs))
    {
        fsFsClose(&fs);
        return false;
    }
    
    return true;
}

bool fs_mount_safe(void)
{
    FsFileSystem fs = {0};
    if (R_FAILED(fs_open_nand_partition(&fs, FsBisPartitionId_SafeMode)))
    {
        return false;
    }

    if (!fs_mount_device("@Safe", fs))
    {
        fsFsClose(&fs);
        return false;
    }
    
    return true;
}

bool fs_mount_gamecard_update(const FsGameCardHandle *handle)
{
    FsFileSystem fs = {0};
    if (!fs_open_gamecard(handle, FsGameCardPartition_Update, &fs))
    {
        return false;
    }
    
    if (!fs_mount_device("@upp", fs))
    {
        fsFsClose(&fs);
        return false;
    }

    return false;
}

bool fs_mount_gamecard_secure(const FsGameCardHandle *handle)
{
    FsFileSystem fs = {0};
    if (!fs_open_gamecard(handle, FsGameCardPartition_Secure, &fs))
    {
        return false;
    }
    
    if (!fs_mount_device("@GcApp", fs))
    {
        fsFsClose(&fs);
        return false;
    }

    return true;
}

bool fs_mount_gamecard_partition(char *out, const FsGameCardHandle *handle, FsGameCardPartition partition)
{
    if (!out)
    {
        write_log("missing params in mount gamecard\n");
        return false;
    }

    if (partition > FsGameCardPartition_Secure) // cannot mount logo.
    {
        write_log("trying to mount a not supported gamecard partition: %u\n", partition);
        return false;
    }

    FsFileSystem fs = {0};
    if (!fs_open_gamecard(handle, partition, &fs))
    {
        write_log("failed to open gc\n");
        return false;
    }
    
    const char partition_letters[] = { 'U', 'N', 'S' };
    snprintf(out, 0x10, "@Gc%c%08x", partition_letters[partition], handle->value);
    
    if (!fs_mount_device(out, fs))
    {
        write_log("failed to mount device\n");
        fsFsClose(&fs);
        return false;
    }

    write_log("returning true\n");
    return true;
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

bool fs_umount_all_devices(void)
{
    Result rc = fsdevUnmountAll();
    if (R_FAILED(rc))
    {
        write_log("failed to unmount all devices");
        return false;
    }
    return true;
}


/*
*   IPC device op.
*/

bool fs_get_game_card_update_partition_info(FsDeviceOperator *d, const FsGameCardHandle *handle, gamecard_update_partition_info_t *info)
{
    struct
    {
        uint32_t handle;
    } in = { handle->value };
    
    struct
    {
        uint32_t version;
        uint64_t title_id;
    } out = {0};
    
    Result rc = serviceDispatchInOut(&d->s, 203, in, out);
    
    if (R_FAILED(rc))
    {
        write_log("Failed to get gamecard update partition info\n");
        return false;
    }

    info->title_id = out.title_id;
    info->version = out.version;
    return true;
}

bool fs_finalise_game_card_driver(FsDeviceOperator *d)
{
    Result rc = serviceDispatch(&d->s, 204);
    if (R_FAILED(rc))
    {
        write_log("failed to finalise driver\n");
        return false;
    }
    return true;
}

bool fs_set_speed_emulation_mode(FsDeviceOperator *d, uint32_t mode)
{
    struct
    {
        uint32_t mode;
    } in = { mode };

    Result rc = serviceDispatchIn(&d->s, 300, in);

    if (R_FAILED(rc))
    {
        write_log("failed to set emu speed mode\n");
        return false;
    }
    return true;
}

bool fs_get_speed_emulation_mode(FsDeviceOperator *d, uint32_t *mode)
{
    struct
    {
        uint32_t mode;
    } out = {0};

    Result rc = serviceDispatchOut(&d->s, 301, out);

    if (R_FAILED(rc))
    {
        write_log("failed to get emu speed mode\n");
        return false;
    }

    *mode = out.mode;
    return true;
}

bool fs_register_update_partition(void)
{
    Result rc = serviceDispatch(fsGetServiceSession(), 1007);
    if (R_FAILED(rc))
    {
        write_log("failed to get update partition info\n");
        return false;
    }
    return true;
}

bool fs_open_registered_partition(FsFileSystem *fs)
{
    struct
    {
        FsFileSystem fs;
    } out = {0};

    Result rc = serviceDispatchOut(fsGetServiceSession(), 1008, out);

    if (R_FAILED(rc))
    {
        write_log("failed to open registered partition\n");
        return false;
    }
    fs->s = out.fs.s;
    return true;
}

bool fs_get_game_card_certificate(FsDeviceOperator *d, const FsGameCardHandle *handle, gamecard_cert_t *cert, size_t size)
{
    struct
    {
        uint32_t handle;
        uint64_t size;
    } in = { handle->value, size };

    struct
    {
        gamecard_cert_t cert;   
    } out = {0};

    Result rc = serviceDispatchInOut(&d->s, 206, in, out);

    if (R_FAILED(rc))
    {
        write_log("Failed to get gamecard cert\n");
        return false;
    }

    memcpy(cert, &out.cert, sizeof(gamecard_cert_t));

    return true;
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