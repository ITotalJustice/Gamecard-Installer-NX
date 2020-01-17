#ifndef _LOG_H_
#define _LOG_H_


#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    LogFlags_File       = 0x00000001,
    LogFlags_Nxlink     = 0x00000002,
    LogFlags_Console    = 0x00000004,
    LogFlags_SDL        = 0x00000008,
} LogFlags;


//
bool init_log(void);

//
void write_log(const char *text, ...);

//
void exit_log(void);

#endif