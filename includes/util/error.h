#ifndef _ERROR_H_
#define _ERROR_H_


#include <stdint.h>


typedef enum
{
    ErrorCode_None              = 0x0000,

    ErrorCode_Init_Gc           = 0x1001,

    ErrorCode_Mount_Handle      = 0x2001,
    ErrorCode_Mount_Partition   = 0x2002,
    ErrorCode_Mount_Chdir       = 0x2004,
    ErrorCode_Mount_NoMeta      = 0x2008,
    ErrorCode_Mount_Cnmt        = 0x2010,

    ErrorCode_Dir_Setup         = 0x3001,
    ErrorCode_Dir_Cnmt          = 0x3002,

    ErrorCode_File_Setup        = 0x4001,
    ErrorCode_File_Cnmt         = 0x4002,

    ErrorCode_Install_Null      = 0x5001,
    ErrorCode_Install_Storage   = 0x5002,
    ErrorCode_Install_CnmtNca   = 0x5004,
    ErrorCode_Install_Cnmt      = 0x5008,
    ErrorCode_Install_Nca       = 0x5010,
    ErrorCode_Install_NoSpace   = 0x5020,

    ErrorCode_Unknown           = 0xFFFF,
} ErrorCodes;


//
const char *get_error_type(uint32_t error);

#endif