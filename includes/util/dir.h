#ifndef _DIR_H_
#define _DIR_H_


#include <stdint.h>
#include <stdbool.h>
#include <dirent.h>


/*
*   GENERAL DIR FUNCTIONS.
*/

// open dir. return NULL if error.
DIR *open_dir(const char *directory);

// check if the path is a dir.
bool is_dir(const char *folder_to_check);

// check if a dir exists, same code as is_dir.
bool check_if_dir_exists(const char *directory);

// change dir.
// creates a dir if one doesnt exist.
bool change_dir(const char *path, ...);

// return size of folder.
size_t get_dir_size(const char *directory);

// return total files in a dir recursively.
size_t get_dir_total_recursive(const char *directory);

//
size_t get_dir_total_filter(const char *directory, const char *filter);

//
void list_dir(const char *directory);

// create dir.
bool create_dir(const char *dir);

// delete dir.
void delete_dir(const char *directory);

// copy folder and all of its contents.
void copy_dir(const char *src, char *dest);

// move folder and all of its contents.
void move_dir(const char *src, char *dest);

#endif