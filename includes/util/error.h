#ifndef _ERROR_H_
#define _ERROR_H_


typedef enum
{
    ErrorCode_None = 0x0000,

    ErrorCode_Init_Gc = 0x1001,
    ErrorCode_Init_Fs = 0x1002,
    ErrorCode_Init_Ncm = 0x1003,
    ErrorCode_Init_Spl = 0x1004,
    ErrorCode_Init_Ns = 0x1005,
    ErrorCode_Init_Crypto = 0x1006,
    ErrorCode_Init_Menu = 0x1007,

    ErrorCode_Mount_Handle = 0x2001,
    ErrorCode_Mount_Secure = 0x2002,
    ErrorCode_Mount_Update = 0x2003,
    ErrorCode_Mount_Chdir = 0x2004,
    ErrorCode_Mount_NoMeta = 0x2005,
    ErrorCode_Mount_OpenDir = 0x2006,

    ErrorCode_OpenDir = 0x3001,
    ErrorCode_ReadDir = 0x3002,
    ErrorCode_ChangeDir = 0x3003,
    ErrorCode_DeleteDir = 0x3004,

    ErrorCode_OpenFile = 0x4001,
    ErrorCode_ReadFile = 0x4002,
    ErrorCode_WriteFile = 0x4003,
    ErrorCode_DeleteFile = 0x4004,

    ErrorCode_DecryptNcaHeader = 0x5001,
    ErrorCode_DecryptNcaSection = 0x5002,
    ErrorCode_DecryptNcaKeak = 0x5003,
    ErrorCode_DecryptTitleKey = 0x5004,
    ErrorCode_EncryptNcaHeader = 0x5005,
    ErrorCode_EncryptNcaSection = 0x5006,
    ErrorCode_EncryptNcaKeak = 0x5007,
    ErrorCode_EncryptTitleKey = 0x5008,

    ErrorCode_WrongFsType = 0x6001,
    ErrorCode_Pfs0Prase = 0x6002,
    ErrorCode_CnmtPrase = 0x6003,
    ErrorCode_Alloc = 0x6004,
    ErrorCode_NcmDb = 0x6005,
    ErrorCode_AppRecord = 0x6006,
    ErrorCode_NoSpace = 0x6007,
    ErrorCode_KeyGen = 0x6008,
    ErrorCode_NoKeyFile = 0x6009,

    ErrorCode_Unknown           = 0xFFFF,
} ErrorCodes;

//
const char *get_error_type(ErrorCodes error);
const char *error_get_description(ErrorCodes error);

#endif