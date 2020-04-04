#ifndef _ROMFS_H_
#define _ROMFS_H_

#include <stdint.h>

#include "nx/ivfc.h"


typedef struct
{
    ivfc_header_t ivfc_header;
    uint8_t _0xE0[0x58];
} RomfsSuperblock_t;

#endif