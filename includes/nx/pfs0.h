#ifndef _PFS0_H_
#define _PFS0_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define PFS0_MAGIC          0x30534650
#define PFS0_HEADER_SIZE    0x10


typedef struct
{
    uint32_t magic;
    uint32_t total_files;
    uint32_t string_table_size;
    uint32_t padding;
} pfs0_header_t;

typedef struct
{
    uint64_t data_offset;
    uint64_t data_size;
    uint32_t name_offset;
    uint32_t padding;
} pfs0_file_table_t;

typedef struct
{
    char name[256];
} pfs0_string_table_t;

typedef struct
{
    uint8_t hash[0x20];
    uint32_t block_size;
    uint32_t always_2;
    uint64_t hash_table_offset;
    uint64_t hash_table_size; 
    uint64_t pfs0_offset;
    uint64_t pfs0_size;
    uint8_t _0x48[0xF0];
} Pfs0Superblock_t;

typedef struct
{
    FILE *file;
    pfs0_header_t header;
    pfs0_file_table_t *file_table;
    pfs0_string_table_t *string_table;

    uint64_t file_table_offset;
    size_t file_table_size;
    uint64_t string_table_offset;
    uint64_t raw_data_offset;
    size_t raw_data_size;
} pfs0_struct_ptr;


//
void read_data(void *buf_out, size_t buf_size, uint64_t offset, FILE *f);

//
void pfs0_get_header(pfs0_struct_ptr *ptr, uint64_t offset);

//
void pfs0_populate_file_table(pfs0_struct_ptr *ptr);

//
void pfs0_populate_string_table(pfs0_struct_ptr *ptr);

//
bool pfs0_check_valid_magic(uint32_t magic);

//
void pfs0_populate_table_size_offsets(pfs0_struct_ptr *ptr, uint64_t offset);

//
size_t pfs0_get_total_raw_data_size(pfs0_struct_ptr *ptr);

//
int pfs0_search_string_table(pfs0_struct_ptr *ptr, const char *search_name);

//
bool pfs0_extract_file(pfs0_struct_ptr *ptr, int location);

//
bool pfs0_extract_all(pfs0_struct_ptr *ptr);

//
void pfs0_free_structs(pfs0_struct_ptr *ptr);

//
bool pfs0_process(pfs0_struct_ptr *ptr, uint64_t offset, FILE *fp);

//
bool pfs0_start(FILE *file, uint64_t offset);

#endif