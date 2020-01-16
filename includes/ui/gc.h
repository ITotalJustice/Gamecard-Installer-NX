#ifndef _GC_H_
#define _GC_H_

#include <stdint.h>
#include <stdbool.h>
#include <switch.h>

#include "gfx/image.h"
#include "gfx/text.h"


typedef struct
{
    image_t *icon;
    text_t *title;
    text_t *author;
    text_t *text_app_id;
    text_t *text_key_gen;
    text_t *text_size;

    char cnmt_name[0x30];
    uint64_t app_id;
    uint8_t key_gen;
    size_t size;
} gamecard_t;


//
bool init_gc(void);

//
void exit_gc(void);

//
bool poll_gc(void);

//
bool mount_gc(gamecard_t *gamecard);

//
void reset_gc(gamecard_t *gamecard);

//
void unmount_gc(gamecard_t *gamecard);

//
void swap_game_in_gc(gamecard_t *gamecard);

//
bool install_gc(gamecard_t *gamecard, NcmStorageId storage_id);

#endif