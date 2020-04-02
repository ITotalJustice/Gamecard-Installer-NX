#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "nx/ivfc.h"

#include "util/log.h"


bool ivfc_check_if_magic_valid(uint32_t magic)
{
    if (magic != IVFC_MAGIC)
    {
        write_log("got wrong ivfc magic %u\n", magic);
        return false;
    }
    return true;
}