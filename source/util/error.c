#include <stdint.h>

#include "util/error.h"


const char *get_error_type(uint32_t error)
{
    switch (error / 1000)
    {
        case 0: return "None";
        case 1: return "Init";
        case 2: return "Mount";
        case 3: return "Dir";
        case 4: return "File";
        case 5: return "Install";
        default: return "Unknown";
    }
}