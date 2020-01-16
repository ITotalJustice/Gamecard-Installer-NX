#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "nx/ivfc.h"


bool ivfc_check_if_magic_valid(uint32_t magic)
{
    if (magic != IVFC_MAGIC)
    {
        printf("\ngot wrong magic %u\n", magic);
        return false;
    }
    return true;
}