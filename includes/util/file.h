#ifndef _FILE_H_
#define _FILE_H_


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/*
*   GENERAL FILE FUNCTIONS.
*/

// open file. return NULL if error.
FILE *open_file2(const char *mode, const char *file, ...);

// return the ext of a file.
const char *get_file_name_ext(const char *filename);

//
bool check_file_ext(const char *file_name, const char *ext);

// check if file exists.
bool check_if_file_exists(const char *file);

// get the size of a file.
size_t get_file_size(const char *file);

//
void *load_file_into_mem(const char *path, size_t *out_size);

// create an empty file.
// returns 1 on success or if the file already existed.
bool create_file(const char *file);

// remove file.
// returns 1 on success or if the file didn't exist.
bool delete_file(const char *file);

// copy file.
void copy_file(const char *src, char *dest);

// move file (rename).
void move_file(const char *src, char *dest);

size_t get_file_size2(FILE *fp);
/*
*   DATA READ / DATA WRITE.
*/

// read from file.
// out: data to be written to.
// size: size of data tp be read.
// offset: starting point of the data.
// f: the file to read the data from.
size_t read_file(void *out, size_t size, uint64_t offset, FILE *f);

#endif