#ifndef _IVFC_H_
#define _IVFC_H_

#include <stdint.h>
#include <stdbool.h>

#define IVFC_MAGIC 0x43465649


typedef struct
{
    uint64_t offset;
    uint64_t size;
    uint32_t block_size_power;
    uint32_t reserved;
} ivfc_level_t;

typedef struct
{
    uint32_t magic;
    uint32_t always_2; // always 2?
    uint32_t hash_size;
    uint32_t always_7; // always 7 for program ncas?
    ivfc_level_t level_1;
    ivfc_level_t level_2;
    ivfc_level_t level_3;
    ivfc_level_t level_4;
    ivfc_level_t level_5;
    ivfc_level_t level_6;
    uint8_t _0xA0[0x20]; // empty
    uint8_t hash[0x20];
} ivfc_header_t;


//
bool ivfc_check_if_magic_valid(uint32_t magic);


#endif