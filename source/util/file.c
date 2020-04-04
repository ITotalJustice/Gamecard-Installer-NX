#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#include "util/file.h"
#include "util/log.h"


FILE *open_file2(const char *mode, const char *file, ...)
{
    char full_path[0x301] = {0};
    va_list v;
    va_start(v, file);
    vsnprintf(full_path, 0x301, file, v);
    va_end(v);

    printf("tried file %s\n", full_path);
    return fopen(full_path, mode);
}

FILE *open_temp_file(const char *mode)
{
    return fopen("sdmc:/temp", mode);
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
    return strcmp(get_filename_ext(file_name), ext) == 0; 
}

size_t get_file_size(const char *file)
{
    size_t size = 0;
    FILE *f = open_file2("r", file);
    if (!f)
        return 0;
    
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);
    return size;
}

size_t get_file_size2(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    return size;
}

void *load_file_into_mem(const char *path, size_t *out_size)
{
    if (!path || !out_size)
    {
        write_log("missing args in %s\n", __func__);
        return NULL;
    }

    FILE *fp = fopen(path, "rb");
    if (!fp)
    {
        write_log("failed to open file %s %s\n", path, __func__);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size_t f_size = ftell(fp);
    rewind(fp);

    if (!f_size)
    {
        write_log("no size %s\n", __func__);
        return NULL;
    }

    void *out_buf = malloc(f_size);
    if (!out_buf)
    {
        write_log("failed to alloc %s\n", __func__);
        return NULL;
    }

    size_t read_size = fread(out_buf, 1, f_size, fp);
    fclose(fp);

    if (read_size != f_size)
    {
        write_log("read size does not match %s\n", __func__);
        free(out_buf);
        return NULL;
    }

    *out_size = f_size;
    return out_buf;
}

bool delete_file(const char *file)
{
    return remove(file) == 0;
}

bool delete_temp_file(void)
{
    return remove("sdmc:/temp") == 0;
}

void copy_file(const char *src, char *dest)
{
    FILE *srcfile = fopen(src, "rb");
    if (!srcfile)
    {
        return;
    }

    FILE *newfile = fopen(dest, "wb");
    if (!newfile)
    {
        fclose(srcfile);
        return;
    }
    
    void *buf = malloc(0x800000);
    if (!buf)
    {
        return;
    }

    size_t bytes = 0; // size of the file to write (8MiB or filesize max)

    while ((bytes = fread(buf, 1, 0x800000, srcfile)))
        fwrite(buf, 1, bytes, newfile);

    free(buf);
    fclose(srcfile);
    fclose(newfile);
}

void move_file(const char *src, char *dest)
{
    rename(src, dest);
}

size_t read_file(void *out, size_t size, uint64_t offset, FILE *f)
{
    fseek(f, offset, SEEK_SET);
    return fread(out, 1, size, f);
}