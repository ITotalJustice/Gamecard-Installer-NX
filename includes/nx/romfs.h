#ifndef _ROMFS_H_
#define _ROMFS_H_

#include <stdint.h>

#include "ivfc.h"


typedef struct
{
    uint8_t _0x0[0x2]; //empty?
    ivfc_header_t ivfc_header;
} romfs_superblock_t;

#endif