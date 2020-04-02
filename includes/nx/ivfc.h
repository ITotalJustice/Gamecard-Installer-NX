#ifndef _IVFC_H_
#define _IVFC_H_

#include <stdint.h>
#include <stdbool.h>

#define IVFC_MAGIC      0x43465649
#define IVFC_LEVEL_MAX  0x6


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
    uint32_t version; // always 2?
    uint32_t hash_size;
    uint32_t level_count; // always 7 for program ncas?
    ivfc_level_t level[IVFC_LEVEL_MAX];
    uint8_t salt_seed[0x20]; // empty?
    uint8_t hash[0x20];
} ivfc_header_t;


//
bool ivfc_check_if_magic_valid(uint32_t magic);


#endif