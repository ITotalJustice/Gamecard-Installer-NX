#ifndef _CNMT_H_
#define _CNMT_H_


#include <stdint.h>
#include <switch/services/ncm.h>
#include <switch/services/ncm_types.h>


typedef struct 
{
    uint64_t title_id;
    uint32_t title_version;
    uint8_t meta_type;
    uint8_t _0xD;
    uint16_t extended_header_size;
    uint16_t content_count;
    uint16_t content_meta_count;
    uint8_t attributes;
    uint8_t _0x15[0x3];
    uint32_t required_sys_version;
    uint8_t _0x1C[0x4];
} CnmtFullHeader_t;

typedef struct
{
    NcmContentMetaHeader header;
    NcmContentMetaKey key;
    uint8_t *extended_header;   // variable size;
    NcmContentInfo *content_infos;
} Cnmt_t;   // i don't have a good name for this yet.


//
bool cnmt_push_record(const NcmContentMetaKey *key, NcmStorageId storage_id);

//
bool cnmt_set_db(const NcmContentMetaKey *key, const NcmContentMetaHeader *header, const void *extended_header, const NcmContentInfo *info, NcmStorageId storage_id);

// this will set required app version and required system version to 0.
void cnmt_set_extended_header(void *extended_header, NcmContentMetaType type);

//
bool cnmt_get_header_and_key(const uint8_t *cnmt_data, NcmContentMetaHeader *header_out, NcmContentMetaKey *key_out, int64_t offset);

// parse the cnmt.
// cnmt_data: the data to parse.
// offset: starting offset of the data.
// cnmt_info: the cnmt.nca info.
// cnmt_out: the output.
bool cnmt_parse(const uint8_t *cnmt_data, uint64_t offset, const NcmContentInfo *cnmt_info, Cnmt_t *cnmt_out);

// open the installed cnmt.nca.
bool cnmt_open_installed_file(const NcmContentId *content_id, const NcmContentInfo *cnmt_info, Cnmt_t *cnmt_out, NcmStorageId storage_id);


/*
*   Debug
*/

//
void cnmt_print_header(const NcmContentMetaHeader *header, const NcmContentMetaKey *key);

//
void cnmt_print_extended_header(const void *extended_header, NcmContentMetaType type);

//
void cnmt_print_content_info(const NcmContentInfo *info);

//
void cnmt_print_content_infos(const NcmContentInfo *info, uint16_t total);

#endif