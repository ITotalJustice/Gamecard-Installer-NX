#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "nx/pfs0.h"


void read_data(void *buf_out, size_t buf_size, uint64_t offset, FILE *f)
{
    fseek(f, offset, SEEK_SET);
    fread(buf_out, buf_size, 1, f);
}

void pfs0_get_header(pfs0_struct_ptr *ptr, uint64_t offset)
{
    memset(&ptr->header, 0, sizeof(pfs0_header_t));
    read_data(&ptr->header, sizeof(pfs0_header_t), offset, ptr->file);
}

void pfs0_populate_file_table(pfs0_struct_ptr *ptr)
{
    ptr->file_table = malloc(ptr->file_table_size);
    memset(ptr->file_table, 0x0, ptr->file_table_size);
    read_data(ptr->file_table, ptr->file_table_size, ptr->file_table_offset, ptr->file);
}

void pfs0_populate_string_table(pfs0_struct_ptr *ptr)
{
    ptr->string_table = malloc(ptr->header.total_files * sizeof(pfs0_string_table_t));
    uint8_t *data_temp = malloc(ptr->header.string_table_size);
    memset(ptr->string_table, 0, ptr->header.total_files * sizeof(pfs0_string_table_t));
    memset(data_temp, 0, ptr->header.string_table_size);

    read_data(data_temp, ptr->header.string_table_size, ptr->string_table_offset, ptr->file);

    for (uint32_t i = 0; i < ptr->header.total_files; i++)
    {
        size_t offset = ptr->file_table[i].name_offset;
        for (uint8_t j = 0; ; j++, offset++)
        {
            ptr->string_table[i].name[j] = data_temp[offset];
            if (ptr->string_table[i].name[j] == 0x00)
            {
                printf("found string %s\n", ptr->string_table[i].name);
                break;
            }
        }
    }
    free(data_temp);
}

bool pfs0_check_valid_magic(uint32_t magic)
{
    if (magic != PFS0_MAGIC)
    {
        printf("\ngot wrong magic %u\n", magic);
        return false;
    }
    return true;
}

void pfs0_populate_table_size_offsets(pfs0_struct_ptr *ptr, uint64_t offset)
{
    ptr->file_table_offset = offset;
    ptr->file_table_size = ptr->header.total_files * sizeof(pfs0_file_table_t);
    ptr->string_table_offset = ptr->file_table_offset + ptr->file_table_size;
    ptr->raw_data_offset = ptr->string_table_offset + ptr->header.string_table_size;
}

size_t pfs0_get_total_raw_data_size(pfs0_struct_ptr *ptr)
{
    size_t total_size = 0;
    for (uint32_t i = 0; i < ptr->header.total_files; i++)
        total_size += ptr->file_table[i].data_size;
    return total_size;
}

int pfs0_search_string_table(pfs0_struct_ptr *ptr, const char *search_name)
{
    for (uint32_t position = 0; position < ptr->header.total_files; position++)
        if (strstr(ptr->string_table[position].name, search_name))
            return position;
    printf("failed to find %s\n", search_name);
    return -1;
}

bool pfs0_extract_file(pfs0_struct_ptr *ptr, int location)
{
    FILE *new_file = fopen(ptr->string_table[location].name, "wb");
    if (!new_file)
    {
        printf("failed to create new file %s\n", ptr->string_table[location].name);
        return false;
    }

    uint64_t curr_off = ptr->raw_data_offset + ptr->file_table[location].data_offset;
    size_t file_size = ptr->file_table[location].data_size;
    printf("extracting %s: size %lu\n", ptr->string_table[location].name, file_size);

    for (size_t offset = 0, buf_size = 0x800000; offset < file_size; offset += buf_size)
    {
        if (offset + buf_size > file_size)
            buf_size = file_size - offset;
        uint8_t *buf = malloc(buf_size);

        read_data(buf, buf_size, curr_off + offset, ptr->file);
        fwrite(buf, buf_size, 1, new_file);
        free(buf);
    }

    fclose(new_file);
    return true;
}

bool pfs0_extract_all(pfs0_struct_ptr *ptr)
{
    for (uint32_t i = 0; i < ptr->header.total_files; i++)
        if (!pfs0_extract_file(ptr, i))
            return false;
    return true;
}

void pfs0_free_structs(pfs0_struct_ptr *ptr)
{
    if (ptr->file_table != NULL)
        free(ptr->file_table);
    if (ptr->string_table != NULL)
        free(ptr->string_table);
}

bool pfs0_start(FILE *file, uint64_t offset) // poor naming, but this basically is like main() for the pfs0.
{
    pfs0_struct_ptr ptr;
    ptr.file = file;

    // now lets call my pfs0 functions!.
    pfs0_get_header(&ptr, offset);
    if (!pfs0_check_valid_magic(ptr.header.magic))
        return false;
    
    // fill out the tables.
    pfs0_populate_table_size_offsets(&ptr, offset + PFS0_HEADER_SIZE);
    pfs0_populate_file_table(&ptr);
    pfs0_populate_string_table(&ptr);
    ptr.raw_data_size = pfs0_get_total_raw_data_size(&ptr);

    // extract the contents of the pfs0.
    pfs0_extract_all(&ptr);

    // free structs that were used.
    pfs0_free_structs(&ptr);

    return true;
}