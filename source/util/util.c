#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <malloc.h>
#include <dirent.h>
#include <unistd.h>
#include <switch.h>

#include "util/util.h"


size_t debug_dump_info(const void *buf, size_t buf_size, const char *path, const char *mode)
{
    size_t size_written = 0;
    FILE *f = fopen(path, mode);
    if (!f)
        return size_written;
    size_written = fwrite(buf, 1, buf_size, f);
    fclose(f);
    return size_written;
}

void *mem_alloc(size_t size)
{
    void *mem = malloc(size);
    if (mem == NULL)
        printf("failed to alloc mem with size %lu\n", size);
    memset(mem, 0, size);
    return mem;
}

void str2hex(uint8_t *out, const char *str)
{
    char b[0x12] = {0};
    snprintf(b, 0x10, str);
    long long num = __bswap64(strtol(b, NULL, 0x10));
    memcpy(out, &num, 0x8);
    printf("%lx", __bswap64(num));
    snprintf(b, 0x10, str + 0x10);
    num = 0;
    num = __bswap64(strtol(b, NULL, 0x10));
    memcpy(out + 0x8 + 1, &num, 0x8);
    printf("%lx\n", __bswap64(num));
}