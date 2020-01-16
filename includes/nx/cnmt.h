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
    u_int16_t ext_header_size;
    u_int16_t content_count;
    u_int16_t content_meta_count;
    uint8_t attributes;
    uint8_t _0x15[3];
    uint32_t required_sys_version;
    uint8_t _0x1C[4];
} cnmt_header_t;

typedef struct
{
    char cnmt_name[1024];
    FsFile cnmt_file;
    uint32_t total_cnmt_infos;
    NcmContentInfo cnmt_info;
    NcmContentInfo *cnmt_infos;
    NcmStorageId storage_id;
} cnmt_struct_t;


// push the app record and set the data in the ncm database.
void cnmt_push_record(cnmt_header_t *cnmt_header, cnmt_struct_t *cnmt_struct, void *ext_header);

// parse the cnmt.
void cnmt_read_data(cnmt_struct_t *cnmt_struct);

// open the cnmt.
Result cnmt_open(cnmt_struct_t *cnmt_struct);

#endif