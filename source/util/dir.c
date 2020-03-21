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
    if (!dir)
        return false;
    closedir(dir);
    return true;
}

bool change_dir(const char *path, ...)
{
    char full_path[0x400];
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
    struct dirent *de;
    DIR *dir = open_dir(directory);
    if (!dir)
        return number_of_files;

    while ((de = readdir(dir)))
    {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
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
    struct dirent *de;
    if (!dir)
        return size;

    while ((de = readdir(dir)))
    {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, de->d_name))
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
    struct dirent *de;
    DIR *dir = open_dir(directory);
    if (!dir)
        return num;
        
    while ((de = readdir(dir)))
    {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, de->d_name))
            return num;

        if (is_dir(de->d_name))
            num += get_dir_total(de->d_name);

        num++;
        free(full_path);
    }
    closedir(dir);
    return num;
}

size_t get_dir_total_filter(const char *directory, const char *filter)
{
    size_t num = 0;
    struct dirent *de;
    DIR *dir = open_dir(directory);
    if (!dir)
        return num;
        
    while ((de = readdir(dir)))
    {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, de->d_name))
            return num;

        if (is_dir(de->d_name))
            num += get_dir_total_filter(de->d_name, filter);

        if (strstr(full_path, filter))
            num++;
        
        free(full_path);
    }
    closedir(dir);
    return num;
}

bool create_dir(const char *dir)
{
    if (check_if_dir_exists(dir))
        return true;
    return mkdir(dir, 0777) == 0 ? true : false;
}

void delete_dir(const char *directory)
{
    struct dirent *de;
    DIR *dir = open_dir(directory);
    if (!dir)
        return;

    while ((de = readdir(dir)))
    {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
            continue;

        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", directory, de->d_name))
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
        
    struct dirent *de;
    DIR *dir = open_dir(src);
    if (!dir)
        return;

    char buffer[0x200] = {0};

    while ((de = readdir(dir)))
    {
        snprintf(buffer, sizeof(buffer), "%s/%s", dest, de->d_name);

        // check if the file is a directory.
        if (is_dir(de->d_name))
        {
            create_dir(buffer);
            copy_dir(de->d_name, de->d_name);
        }
        else
            copy_file(de->d_name, buffer);
    }
    closedir(dir);
}

void move_folder(const char *src, char *dest)
{
    DIR *dir = open_dir(src);
    struct dirent *de;
    create_dir(dest);

    while ((de = readdir(dir)))
    {
        char *full_path;
        if (!asiprintf(&full_path, "%s/%s", src, de->d_name))
            return;

        // check if the file is a directory.
        if (is_dir(de->d_name))
            create_dir(full_path);
        else
            move_file(de->d_name, full_path);

        free(full_path);
    }
    closedir(dir);
}