#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#include "util/file.h"


FILE *open_file2(const char *mode, const char *file, ...)
{
    char full_path[0x400] = {0};
    va_list v;
    va_start(v, file);
    vsnprintf(full_path, 0x400, file, v);
    va_end(v);

    printf("tried file %s\n", full_path);
    return fopen(full_path, mode);
}

bool create_file(const char *file)
{
    if (check_if_file_exists(file))
        return true;
    FILE *f = open_file2(file, "wb");
    if (!f)
        return false;
    fclose (f);
    return true;
}

bool check_if_file_exists(const char *file)
{
    FILE *f = fopen(file, "r");
    if (!f)
        return false;
    fclose(f);
    return true;
}

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    return !dot || dot == filename ? "" : dot + 1;
}

bool check_file_ext(const char *file_name, const char *ext)
{
    return strcmp(get_filename_ext(file_name), ext) == 0 ? true : false; 
}

size_t get_file_size(const char *file)
{
    size_t size = 0;
    FILE *f = open_file2(file, "r");
    if (!f)
        return size;
    
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
    return size;
}

bool delete_file(const char *file)
{
    return remove(file) == 0 ? true : false;
}

void copy_file(const char *src, char *dest)
{
    FILE *srcfile = fopen(src, "rb");
    FILE *newfile = fopen(dest, "wb");

    if (srcfile && newfile)
    {
        void *buf = malloc(0x800000);
        size_t bytes; // size of the file to write (8MiB or filesize max)

        while (0 < (bytes = fread(buf, 1, 0x800000, srcfile)))
            fwrite(buf, bytes, 1, newfile);
        free(buf);
    }
    fclose(srcfile);
    fclose(newfile);
}

void move_file(const char *src, char *dest)
{
    rename(src, dest);
}

void read_file(void *out, size_t size, uint64_t offset, FILE *f)
{
    fseek(f, offset, SEEK_SET);
    fread(out, size, 1, f);
}