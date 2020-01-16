#include <stdio.h>
#include <stdbool.h>
#include <switch.h>

#include "ns.h"


bool init_ns(void)
{
    return R_FAILED(nsInitialize()) ? false : true;
}

void exit_ns(void)
{
    nsExit();
}

size_t ns_get_storage_total_size(NcmStorageId storage_id)
{
    size_t size = 0;
    if (R_FAILED(nsGetTotalSpaceSize(storage_id, &size)))
        printf("failed to get ns total storage size\n");
    return size;
}

size_t ns_get_storage_free_space(NcmStorageId storage_id)
{
    size_t size = 0;
    if (R_FAILED(nsGetFreeSpaceSize(storage_id, &size)))
        printf("failed to get ns total free space\n");
    return size;
}

int32_t ns_list_app_record(NsApplicationRecord *out, int32_t count, int32_t offset)
{
    int32_t out_count = 0;
    if (R_FAILED(nsListApplicationRecord(out, count, offset, &out_count)))
        printf("failed to list app records\n");
    if (out_count != count)
        printf("ns_list_app_record missmatch: got %d, expected %d\n", out_count, count);
    return out_count;
}

int32_t ns_list_app_cnmt_status(NsApplicationContentMetaStatus *out, int32_t count, uint64_t app_id)
{
    int32_t out_count = 0;
    if (R_FAILED(nsListApplicationContentMetaStatus(app_id, 0, out, count, &out_count)))
        printf("failed to list cnmt status\n");
    if (out_count != count)
        printf("ns_list_cnmt_status missmatch: got %d, expected %d\n", out_count, count);
    return out_count;
}

size_t ns_get_app_control_data(NsApplicationControlData *out, uint64_t app_id)
{
    size_t out_size = 0;
    /*Result rc = */nsGetApplicationControlData(NsApplicationControlSource_Storage, app_id, out, sizeof(NsApplicationControlData), &out_size);
    //if (R_FAILED(rc))
        //printf("failed to get app control data with app_id: %lu\n", app_id);
    return out_size;
}

int32_t ns_get_app_delivery_info(NsApplicationDeliveryInfo *out, int32_t count, uint64_t app_id, uint32_t attr)
{
    int32_t out_count = 0;
    if (!hosversionAtLeast(4, 0, 0))
        return out_count;
    if (R_FAILED(nsGetApplicationDeliveryInfo(out, count, app_id, attr, &out_count)))
        printf("failed to get app delivery info\n");
    if (out_count != count)
        printf("ns_get_app_delivery_info missmatch: got %d, expected %d\n", out_count, count);
    return out_count;
}

bool ns_check_app_delivery_info(const NsApplicationDeliveryInfo *info)
{
    bool res = false;
    if (!hosversionAtLeast(4, 0, 0))
        return res;
    if (R_FAILED(nsHasAllContentsToDeliver(info, 1, &res)))
        printf("failed to check for valid app delivery info\n");
    return res;
}

int32_t ns_compare_app_delivery_info(const NsApplicationDeliveryInfo *info0, const NsApplicationDeliveryInfo *info1)
{
    int32_t res = -1;
    if (!hosversionAtLeast(4, 0, 0))
        return res;
    if (R_FAILED(nsCompareApplicationDeliveryInfo(info0, 1, info1, 1, &res)))
        printf("failed to compare app delivery infos\n");
    return res;
}

bool ns_check_if_can_deliver_app_info(NsApplicationDeliveryInfo *info0, int32_t count0, NsApplicationDeliveryInfo *info1)
{
    bool res = false;
    if (!hosversionAtLeast(4, 0, 0))
        return res;
    if (R_FAILED(nsCanDeliverApplication(info0, count0, info1, 1, &res)))
        printf("failed to check if app info can be delivered\n");
    return res;
}

int32_t ns_list_content_meta_key(NcmContentMetaKey *meta, NsApplicationDeliveryInfo *info)
{
    int32_t total_out = 0;
    if (!hosversionAtLeast(4, 0, 0))
        return total_out;
    if (R_FAILED(nsListContentMetaKeyToDeliverApplication(meta, 1, 0, info, 1, &total_out)))
        printf("failed to list content meta key\n");
    return total_out;
}

uint32_t ns_count_application_record(uint64_t app_id) // always returns 4 if app exists... is that the number of ncas?
{
    uint32_t count = 0;
    Result rc = serviceDispatchInOut(nsGetServiceSession_ApplicationManagerInterface(), 1, app_id, count, SfOutHandleAttr_None);
    if (R_FAILED(rc))
        printf("failed to count application records\n");
    return count;
}

Result ns_delete_application_entity(uint64_t app_id)
{
    Result rc = serviceDispatchIn(nsGetServiceSession_ApplicationManagerInterface(), 4, app_id, SfOutHandleAttr_None);
    if (R_FAILED(rc))
        printf("failed to delete application entity\n");
    return rc;
}

Result ns_delete_application_completely(uint64_t app_id) // always fails, even though it does delete the application......
{
    Result rc = serviceDispatchIn(nsGetServiceSession_ApplicationManagerInterface(), 5, app_id, SfOutHandleAttr_None);
    if (R_FAILED(rc))
        printf("failed to delete application completely\n");
    return rc;
}

bool ns_is_application_moveable(uint64_t app_id)
{
    bool can_move = false;
    //8
    return can_move;
}

size_t ns_get_application_occupied_size(uint64_t app_id)
{
    size_t size = 0;
    Result rc = serviceDispatchInOut(nsGetServiceSession_ApplicationManagerInterface(), 11, app_id, size, SfOutHandleAttr_None);
    if (R_FAILED(rc))
        printf("failed to delete application record\n");
    return size;
}

Result ns_push_application_record(uint64_t app_id, void *cnmt_storage_records, size_t data_size)
{
    struct
    {
        uint8_t last_modified_event;
        uint8_t padding[0x7];
        uint64_t app_id;
    } in = { NsApplicationRecordType_Installed, {0}, app_id };
    
    Result rc =  serviceDispatchIn(nsGetServiceSession_ApplicationManagerInterface(), 16, in,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { cnmt_storage_records, data_size } });

    if (R_FAILED(rc))
        printf("failed to push application record %08X\n", rc);
    return rc;
}

Result ns_list_application_record_content_meta(uint64_t offset, uint64_t app_id, void *out_buf, size_t out_buf_size, uint32_t count)
{
    struct
    {
        uint64_t offset;
        uint64_t app_id;
    } in = { offset, app_id };
    uint32_t out = 0;

    Result rc = serviceDispatchInOut(nsGetServiceSession_ApplicationManagerInterface(), 17, in, out,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { out_buf, out_buf_size } });

    if (R_FAILED(rc))
        printf("failed to list app cnmt\n");
    if (count != out)
        printf("count difference\n");
    return rc;
}

Result ns_delete_application_record(uint64_t app_id)
{
    Result rc = serviceDispatchIn(nsGetServiceSession_ApplicationManagerInterface(), 27, app_id, SfOutHandleAttr_None);
    if (R_FAILED(rc))
        printf("failed to delete application record\n");
    return rc;
}

uint32_t ns_count_application_content_meta(uint64_t app_id) // need a function to check if it has at least 1 content meta, currently will fail if non exists.
{
    uint32_t count = 0;
    Result rc = serviceDispatchInOut(nsGetServiceSession_ApplicationManagerInterface(), 600, app_id, count, SfOutHandleAttr_None);
    if (R_FAILED(rc))
        printf("failed to count app cnmt\n");
    return count;
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

void ns_get_application_record()
{
    if (!hosversionAtLeast(2, 0, 0))
        return;
    // 900
}