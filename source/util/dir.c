#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>


#include "util/dir.h"
#include "util/file.h"
#include "util/log.h"


DIR *open_dir(const char *directory)
{
    return opendir(directory);
}

bool is_dir(const char *folder_to_check)
{
    DIR *dir = opendir(folder_to_check);
    if (!dir)
        return false;
    closedir(dir);
    return true;
}

bool check_if_dir_exists(const char *directory)
{
    DIR *dir = opendir(directory);
    if (!dir) return false;
    closedir(dir);
    return true;
}

bool change_dir(const char *path, ...)
{
    char full_path[0x301] = {0};
    va_list v;
    va_start(v, path);
    vsprintf(full_path, path, v);
    va_end(v);

    bool ret = chdir(full_path) == 0 ? true : false;
    if (!ret)
        printf("failed to change dir %s\n", full_path);
    return ret;
}

size_t get_dir_total(const char *directory)
{
    size_t number_of_files = 0;
    struct dirent *d = {0};
    DIR *dir = open_dir(directory);
    if (!dir)
        return 0;

    while ((d = readdir(dir)))
    {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
            continue;
        number_of_files++;
    }

    closedir(dir);
    return number_of_files;
}

size_t get_dir_size(const char *directory)
{
    size_t size = 0;
    DIR *dir = open_dir(directory);
    struct dirent *d = {0};
    if (!dir)
        return 0;

    while ((d = readdir(dir)))
    {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, d->d_name))
            return size;

        if (is_dir(full_path))
            size += get_dir_size(full_path);
        else
            size += get_file_size(full_path);

        free(full_path);
    }
    return size;
}

size_t get_dir_total_recursive(const char *directory)
{
    size_t num = 0;
    struct dirent *d = {0};
    DIR *dir = open_dir(directory);
    if (!dir)
        return 0;
        
    while ((d = readdir(dir)))
    {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, d->d_name))
            return num;

        if (is_dir(d->d_name))
            num += get_dir_total(d->d_name);

        num++;
        free(full_path);
    }
    closedir(dir);
    return num;
}

size_t get_dir_total_filter(const char *directory, const char *filter)
{
    size_t num = 0;
    struct dirent *d = {0};
    DIR *dir = open_dir(directory);
    if (!dir)
        return 0;
        
    while ((d = readdir(dir)))
    {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, d->d_name))
            return num;

        if (is_dir(d->d_name))
            num += get_dir_total_filter(d->d_name, filter);

        if (strstr(full_path, filter))
            num++;
        
        free(full_path);
    }
    closedir(dir);
    return num;
}

void list_dir(const char *directory)
{
    struct dirent *d = {0};
    DIR *dir = opendir(directory);

    write_log("\nlisting dir\n");

    if (!dir) return;
    while ((d = readdir(dir)))
    {
        write_log("found %s\n", d->d_name);
    }
    write_log("finished listing dir\n\n");
    closedir(dir);
}

bool create_dir(const char *dir)
{
    if (check_if_dir_exists(dir))
        return true;
    return mkdir(dir, 0777) == 0;
}

void delete_dir(const char *directory)
{
    struct dirent *d = {0};
    DIR *dir = open_dir(directory);
    if (!dir)
        return;

    while ((d = readdir(dir)))
    {
        if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, d->d_name))
            return;

        is_dir(full_path) ? delete_dir(full_path) : delete_file(full_path);

        free(full_path);
    }
    closedir(dir);
    rmdir(directory);
}

void copy_dir(const char *src, char *dest)
{
    if (!create_dir(dest))
        return;
        
    struct dirent *d = {0};
    DIR *dir = open_dir(src);
    if (!dir)
        return;

    char buffer[0x301] = {0};

    while ((d = readdir(dir)))
    {
        snprintf(buffer, sizeof(buffer), "%s/%s", dest, d->d_name);

        // check if the file is a directory.
        if (is_dir(d->d_name))
        {
            create_dir(buffer);
            copy_dir(d->d_name, d->d_name);
        }
        else
            copy_file(d->d_name, buffer);
    }
    closedir(dir);
}

void move_folder(const char *src, char *dest)
{
    DIR *dir = open_dir(src);
    struct dirent *d = {0};
    create_dir(dest);

    while ((d = readdir(dir)))
    {
        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", src, d->d_name))
            return;

        // check if the file is a directory.
        if (is_dir(d->d_name))
            create_dir(full_path);
        else
            move_file(d->d_name, full_path);

        free(full_path);
    }
    closedir(dir);
}