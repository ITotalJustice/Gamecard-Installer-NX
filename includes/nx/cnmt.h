#ifndef _CNMT_H_
#define _CNMT_H_


#include <stdint.h>
#include <switch.h>


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
} CnmtHeader_t;

typedef struct
{
    uint16_t info_count;
    NcmContentInfo *content_infos;
} CnmtContentInfos_t;



// parse the cnmt.
bool cnmt_read_data(NcmContentInfo *cnmt_info, FsFile *file, NcmStorageId storage_id, CnmtContentInfos_t *infos);

// open the cnmt.
bool cnmt_open(NcmContentId *content_id, NcmStorageId storage_id, CnmtContentInfos_t *infos);

#endif