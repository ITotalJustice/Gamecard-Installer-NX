#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    DataSize_1kb = 0x400,
    DataSize_1MiB = 0x100000,
    DataSize_2MiB = 0x200000,
    DataSize_3MiB = 0x300000,
    DataSize_4MiB = 0x400000,
    DataSize_5MiB = 0x500000,
    DataSize_6MiB = 0x600000,
    DataSize_7MiB = 0x700000,
    DataSize_8MiB = 0x800000,
    DataSize_16MiB = 0x1000000
} DataSize;


/*
* DEBUG functions.
*/

// dump data to a file.
// buf: data to dump.
// buf_size: size of data to dump.
// path: file to dump data to.
// mode: file mode. supported modes are w, wb, w+, wb+
size_t debug_dump_info(const void *buf, size_t buf_size, const char *path, const char *mode);


/*
*   MISC functions.
*/

//
void *mem_alloc(size_t size);

// memory is copied then checked to see if both src and dst is equal.
bool safe_memcpy(void *dst, const void *src, size_t size);

//
void str2hex(uint8_t *out, const char *str);

#endif