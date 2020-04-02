#include <stdio.h>
#include <stdbool.h>
#include <switch.h>

#include "nx/ns.h"
#include "util/log.h"


Service NS_APP_SERV = {0};

Result __ns_get_application_service(void)
{
    if (hosversionBefore(3, 0, 0))
        return smGetService(&NS_APP_SERV, "ns:am");
    else
        return nsGetApplicationManagerInterface(&NS_APP_SERV);
}


bool init_ns(void)
{
    if (R_FAILED(nsInitialize())) return false;
    if (R_FAILED(__ns_get_application_service())) return false;
    return true;
}

void exit_ns(void)
{
    serviceClose(&NS_APP_SERV);
    nsExit();
}

int64_t ns_get_storage_total_size(NcmStorageId storage_id)
{
    int64_t size = 0;
    if (R_FAILED(nsGetTotalSpaceSize(storage_id, &size)))
        write_log("failed to get ns total storage size\n");
    return size;
}

int64_t ns_get_storage_free_space(NcmStorageId storage_id)
{
    int64_t size = 0;
    if (R_FAILED(nsGetFreeSpaceSize(storage_id, &size)))
        write_log("failed to get ns total free space\n");
    return size;
}

int32_t ns_list_app_record(NsApplicationRecord *out, int32_t count, int32_t offset)
{
    int32_t out_count = 0;
    if (R_FAILED(nsListApplicationRecord(out, count, offset, &out_count)))
        write_log("failed to list app records\n");
    return out_count;
}

int32_t ns_list_app_cnmt_status(NsApplicationContentMetaStatus *out, int32_t count, uint64_t app_id)
{
    int32_t out_count = 0;
    if (R_FAILED(nsListApplicationContentMetaStatus(app_id, 0, out, count, &out_count)))
        write_log("failed to list cnmt status\n");
    return out_count;
}

size_t ns_get_app_control_data(NsApplicationControlData *out, uint64_t app_id)
{
    size_t out_size = 0;
    Result rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, app_id, out, sizeof(NsApplicationControlData), &out_size);
    if (R_FAILED(rc))
        write_log("failed to get app control data with app_id: %lu\n", app_id);
    return out_size;
}

int32_t ns_get_app_delivery_info(NsApplicationDeliveryInfo *out, int32_t count, uint64_t app_id, uint32_t attr)
{
    int32_t out_count = 0;
    if (!hosversionAtLeast(4, 0, 0))
        return out_count;
    if (R_FAILED(nsGetApplicationDeliveryInfo(out, count, app_id, attr, &out_count)))
        write_log("failed to get app delivery info\n");
    if (out_count != count)
        write_log("ns_get_app_delivery_info missmatch: got %d, expected %d\n", out_count, count);
    return out_count;
}

bool ns_check_app_delivery_info(const NsApplicationDeliveryInfo *info)
{
    bool res = false;
    if (!hosversionAtLeast(4, 0, 0))
        return res;
    if (R_FAILED(nsHasAllContentsToDeliver(info, 1, &res)))
        write_log("failed to check for valid app delivery info\n");
    return res;
}

int32_t ns_compare_app_delivery_info(const NsApplicationDeliveryInfo *info0, const NsApplicationDeliveryInfo *info1)
{
    int32_t res = -1;
    if (!hosversionAtLeast(4, 0, 0))
        return res;
    if (R_FAILED(nsCompareApplicationDeliveryInfo(info0, 1, info1, 1, &res)))
        write_log("failed to compare app delivery infos\n");
    return res;
}

bool ns_check_if_can_deliver_app_info(NsApplicationDeliveryInfo *info0, int32_t count0, NsApplicationDeliveryInfo *info1)
{
    bool res = false;
    if (!hosversionAtLeast(4, 0, 0))
        return res;
    if (R_FAILED(nsCanDeliverApplication(info0, count0, info1, 1, &res)))
        write_log("failed to check if app info can be delivered\n");
    return res;
}

int32_t ns_list_content_meta_key(NcmContentMetaKey *meta, NsApplicationDeliveryInfo *info)
{
    int32_t total_out = 0;
    if (!hosversionAtLeast(4, 0, 0))
        return total_out;
    if (R_FAILED(nsListContentMetaKeyToDeliverApplication(meta, 1, 0, info, 1, &total_out)))
        write_log("failed to list content meta key\n");
    return total_out;
}

int32_t ns_count_application_record(uint64_t app_id)
{
    int32_t count = 0;
    Result rc = serviceDispatchInOut(&NS_APP_SERV, 1, app_id, count, SfOutHandleAttr_None);
    if (R_FAILED(rc))
        write_log("failed to count application records\n");
    return count;
}

Result ns_delete_application_entity(uint64_t app_id)
{
    Result rc = nsDeleteApplicationEntity(app_id);
    if (R_FAILED(rc))
        write_log("failed to delete application entity\n");
    return rc;
}

Result ns_delete_application_completely(uint64_t app_id)
{
    Result rc = nsDeleteApplicationCompletely(app_id);
    if (R_FAILED(rc))
        write_log("failed to delete application completely\n");
    return rc;
}

bool ns_is_application_moveable(uint64_t app_id, NcmStorageId storage_id)
{
    bool can_move = false;
    if (R_FAILED(nsIsApplicationEntityMovable(app_id, storage_id, &can_move)))
        write_log("failed to check if application %lu is moveable to storage_id %u\n", app_id, storage_id);
    return can_move;
}

Result ns_move_application(uint64_t app_id, NcmStorageId storage_id)
{
    Result rc = nsMoveApplicationEntity(app_id, storage_id);
    if (R_FAILED(rc))
        write_log("failed to move application %lu to storage_id %u\n", app_id, storage_id);
    return rc;
}

NsApplicationOccupiedSize ns_get_application_occupied_size(uint64_t app_id)
{
    NsApplicationOccupiedSize size = {0};
    Result rc = nsCalculateApplicationOccupiedSize(app_id, &size);
    if (R_FAILED(rc))
        write_log("failed to delete application record\n");
    return size;
}

bool ns_push_application_record(uint64_t app_id, const NcmContentStorageRecord *records, uint32_t count)
{
    const struct
    {
        uint8_t last_modified_event;
        uint8_t padding[0x7];
        uint64_t app_id;
    } in = { NsApplicationRecordType_Installed, {0}, app_id };
    
    Result rc =  serviceDispatchIn(&NS_APP_SERV, 16, in,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { records, sizeof(NcmContentStorageRecord) * count } });

    if (R_FAILED(rc))
    {
        write_log("failed to push application record\n");
        return false;
    }
    return true;
}

bool ns_list_application_record_content_meta(uint64_t offset, uint64_t app_id, NcmContentStorageRecord *records, uint32_t count)
{
    const struct
    {
        uint64_t offset;
        uint64_t app_id;
    } in = { offset, app_id };
    uint32_t out = 0;

    Result rc = serviceDispatchInOut(&NS_APP_SERV, 17, in, out,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { records, sizeof(NcmContentStorageRecord) * count } });

    if (R_FAILED(rc))
    {
        write_log("failed to list app cnmt\n");
        return false;
    }
    return true;
}

bool ns_delete_application_record(uint64_t app_id)
{
    Result rc = serviceDispatchIn(&NS_APP_SERV, 27, app_id, SfOutHandleAttr_None);
    if (R_FAILED(rc))
    {
        write_log("failed to delete application record\n");
    }
    return true;
}

int32_t ns_count_application_content_meta(uint64_t app_id)
{
    int32_t count = 0;
    Result rc = nsCountApplicationContentMeta(app_id, &count);

    if (rc == 0x410)    // not an error. Just means no cnmt found.
    {
        return 0;
    }

    if (R_FAILED(rc))
    {
        write_log("failed to count app cnmt\n");
        return 0;
    }

    return count;
}

//Result ncm_get_latest_key
bool ns_application_overwrite_previous_entity(uint64_t id)
{
    if (!id)
    {
        write_log("missing params in ns_application_pverwrite_previous_entity\n");
        return false;
    }

    Result rc = nsDeleteApplicationEntity(id);  // this func wraps around ncmContentMetaDatabaseRemove.
    if (R_FAILED(rc))
    {
        write_log("failed to delete application_entity\n");
        return false;
    }

    return true;
}

#include <string.h>
bool ns_get_gamecard_info(gamecard_info_t *info)
{
    struct
    {
        uint8_t d[0x10];
    } out = {0};

    Result rc = serviceDispatchOut(&NS_APP_SERV, 46, out);
    if (R_FAILED(rc))
    {
        write_log("Failed to get gamecard info\n");
        return false;
    }
    memcpy(info, &out, 0x10);
    return true;
}

bool ns_has_application_record(uint64_t app_id) //5.0.0
{
    bool has_record = false;
    //910
    // or
    // ReadOnlyApplicationRecordInterface.
    // 0.
    return has_record;
}

bool ns_get_application_record(NcmContentStorageRecord *record_out)
{
    if (!hosversionAtLeast(2, 0, 0))
        return false;

    struct
    {
        uint8_t d[0x18];
    } out = {0};

    Result rc = serviceDispatchOut(&NS_APP_SERV, 900, out);
    if (R_FAILED(rc))
    {
        write_log("Failed to get gamecard info\n");
        return false;
    }
    memcpy(record_out, &out, sizeof(out));
    return true;
}