#ifndef _GC_H_
#define _GC_H_

#include <stdint.h>
#include <stdbool.h>
#include <switch.h>

#include "ui/menu.h"    // for GameInfo_t. Changing this soon.

#include "nx/cnmt.h"
#include "nx/nca.h"
#include "gfx/image.h"
#include "gfx/text.h"


typedef struct
{
    uint16_t file_count;
    uint16_t cnmt_count;    
    uint16_t nca_count;     // num of ncas found.
    uint16_t tik_count;     // num tickets on gamecard.
    uint16_t cert_count;    // num cert on gamecard.
    uint16_t game_count;    // num of games found. this is the total cnmt found with a unique ID.
    uint64_t total_size;    // total size of all files on the gamecard.
} GameCardFileTable_t;

// maybe change this so that each entry gets a file entry
// so cert/nca/tik/cnmt etc...
typedef struct
{
    char name[0x30];
} GameCardStringTable_t;

typedef struct
{
    uint8_t key_gen;        // keygen of this entry (only cnmt is checked).
    uint64_t size;          // size of all ncas in this entry.
    Cnmt_t cnmt;            // ncm data, such as cnmt header, cnmt extended header, ncm key, content infos.
} GameCardEntry_t;

typedef struct
{
    uint64_t total_size;    // total install size.
    uint64_t base_size;     // total base size.
    uint64_t upp_size;      // total upp size.
    uint64_t dlc_size;      // total dlc size.

    uint32_t total_count;
    uint16_t base_count;    // should always be 1.
    uint16_t upp_count;     // i'm not sure if games can come with multiple updates for the same game, wouldn't make sense. This is here for compatibilty should that happen (cxi).
    uint16_t dlc_count;     // games can come bundled with multiple dlc.

    GameCardEntry_t *base;  // array for base entry.
    GameCardEntry_t *upp;   // array for upp entry.
    GameCardEntry_t *dlc;   // array for dlc entry.
} GameCardGameEntries_t;

typedef struct
{
    GameCardGameEntries_t *entries;
    GameCardFileTable_t file_table;
    GameCardStringTable_t *string_table;
} GameCard_t;


//
bool init_gc(void);
void exit_gc(void);

//
bool gc_poll(void);
bool gc_mount(void);
bool gc_unmount(void);

//
uint16_t gc_get_game_count(void);
uint16_t gc_get_base_count(uint16_t game_pos);
uint16_t gc_get_upp_count(uint16_t game_pos);
uint16_t gc_get_dlc_count(uint16_t game_pos);
uint16_t gc_get_current_base_count(void);
uint16_t gc_get_current_upp_count(void);
uint16_t gc_get_current_dlc_count(void);

//
bool gc_setup_game_info(GameInfo_t *out_info, uint16_t game_pos);
bool gc_setup_detailed_game_info(GameInfoDetailed_t *info_out, uint16_t entry);
bool gc_next_game(GameInfo_t *info_out);
bool gc_prev_game(GameInfo_t *info_out);
bool gc_change_game(GameInfo_t *info_out, uint16_t game_pos);

//
bool gc_install_ex(uint16_t game_pos, NcmStorageId storage_id);
bool gc_install(NcmStorageId storage_id);

#endif