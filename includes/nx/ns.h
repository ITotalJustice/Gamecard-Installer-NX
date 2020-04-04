#ifndef _NS_H_
#define _NS_H_

#include <stdint.h>
#include <stdbool.h>
#include <switch.h>

#include "nx/ncm.h"


typedef enum
{
    NsApplicationRecordType_Installed       = 0x3,
    NsApplicationRecordType_GamecardMissing = 0x5,
    NsApplicationRecordType_Archived        = 0xB,
} NsApplicationRecordType;

/*
* The first value is the number of times the gc has been inserted / removed.
* Inserted is set to true when called with a gc inserted, set to zero otherwise.
* Reboot value is changed on every reboot.
*
* Calling this func more than once results in strange values returned.
*
* The reboot value becomes zero if called again with a gc inserted.
* The value is something different when called again without a gc inserted.
* Sometimes the inserted value is set to one if called again and the gc isn't inserted.
*
* Overall, idk what this func does.
*/
typedef struct
{
    uint32_t num_gc_state_change;
    uint8_t unk1[0x4];
    uint8_t inserted;
    uint8_t unk2[0x3];
    uint8_t reboot_value;    //this value is changed every reboot.
    uint8_t unk3[0x3];
} gamecard_info_t;


//
bool init_ns(void);

//
void exit_ns(void);

// returns the total size of the sd card.
int64_t ns_get_storage_total_size(NcmStorageId storage_id);

// returns the free space on the sd card.
int64_t ns_get_storage_free_space(NcmStorageId storage_id);

// store data found to out.
// make sure to set the size of the out large enough.
// returns the total stored.
int32_t ns_list_app_record(NsApplicationRecord *out, int32_t count, int32_t offset);

//
int32_t ns_list_app_cnmt_status(NsApplicationContentMetaStatus *out, int32_t count, uint64_t app_id);

// store data to out.
size_t ns_get_app_control_data(NsApplicationControlData *out, uint64_t app_id);

//
int32_t ns_get_app_delivery_info(NsApplicationDeliveryInfo *out, int32_t count, uint64_t app_id, uint32_t attr);

//
bool ns_check_app_delivery_info(const NsApplicationDeliveryInfo *info);

//
int32_t ns_compare_app_delivery_info(const NsApplicationDeliveryInfo *info0, const NsApplicationDeliveryInfo *info1);

//
bool ns_check_if_can_deliver_app_info(NsApplicationDeliveryInfo *info0, int32_t count0, NsApplicationDeliveryInfo *info1);

//
int32_t ns_list_content_meta_key(NcmContentMetaKey *meta, NsApplicationDeliveryInfo *info);

//
int32_t ns_count_application_record(uint64_t app_id);

//
Result ns_delete_application_entity(uint64_t app_id);

//
Result ns_delete_application_completely(uint64_t app_id);

//
bool ns_is_application_moveable(uint64_t app_id, NcmStorageId storage_id);

//
Result ns_move_application(uint64_t app_id, NcmStorageId storage_id);

//
NsApplicationOccupiedSize ns_get_application_occupied_size(uint64_t app_id);

// push an application record.
bool ns_push_application_record(uint64_t app_id, const NcmContentStorageRecord *records, uint32_t count);

// delete an application record using the app_id.
bool ns_delete_application_record(uint64_t app_id);

// count the amount of content already installed.
int32_t ns_count_application_content_meta(uint64_t app_id);

// write all existing content to void *out_buf. Call this after count_out > 1.
bool ns_list_application_record_content_meta(uint64_t offset, uint64_t app_id, NcmContentStorageRecord *records, uint32_t count);


/*
* testing
*/

//
bool ns_get_gamecard_info(gamecard_info_t *out);

#endif