#include <stdint.h>

#include "util/error.h"


const char *get_error_type(ErrorCodes error)
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

const char *error_get_description(ErrorCodes error)
{
    switch (error)
    {
        case ErrorCode_None:
            return "None";

        case ErrorCode_Init_Gc:
            return "Failed to init Gamecard";
        case ErrorCode_Init_Fs:
            return "Failed to init FS";
        case ErrorCode_Init_Ncm:
            return "Failed to init NCM";
        case ErrorCode_Init_Spl:
            return "Failed to init SPL";
        case ErrorCode_Init_Ns:
            return "Failed to init NS";
        case ErrorCode_Init_Crypto:
            return "Failed to init Crypto";
        case ErrorCode_Init_Menu:
            return "Failed to init Menu";

        case ErrorCode_Mount_Handle:
            return "Failed to get gamecard handle";
        case ErrorCode_Mount_Secure:
            return "Failed to mount secure partition";
        case ErrorCode_Mount_Update:
            return "Failed to mount update partition";
        case ErrorCode_Mount_NoMeta:
            return "No CNMT found";

        case ErrorCode_Alloc:
            return "Failed to allocate memory";
        case ErrorCode_OpenDir:
            return "Failed to open directory";
        case ErrorCode_ReadDir:
            return "Failed to read directory";
        case ErrorCode_ChangeDir:
            return "Failed to change directory";
        case ErrorCode_DeleteDir:
            return "Failed to delete directory";
    
        case ErrorCode_OpenFile:
            return "Failed to open file";
        case ErrorCode_ReadFile:
            return "Failed to read file";
        case ErrorCode_WriteFile:
            return "Failed to write file";
        case ErrorCode_DeleteFile:
            return "Failed to delete file";

        case ErrorCode_DecryptNcaHeader:
            return "Failed to decrypt nca header";
        case ErrorCode_DecryptNcaSection:
            return "Failed to decrypt nca section";
        case ErrorCode_DecryptNcaKeak:
            return "Failed to decrypt nca keak";
        case ErrorCode_DecryptTitleKey:
            return "Failed to decrypt titlekey";

        case ErrorCode_EncryptNcaHeader:
            return "Failed to encrypt nca header";
        case ErrorCode_EncryptNcaSection:
            return "Failed to encrypt nca section";
        case ErrorCode_EncryptNcaKeak:
            return "Failed to encrypt nca section";
        case ErrorCode_EncryptTitleKey:
            return "Failed to encrypt titlekey";

        case ErrorCode_NcmDb:
            return "Failed to commit ncmDB";
        case ErrorCode_AppRecord:
            return "Failed to push application record";
        case ErrorCode_NoSpace:
            return "Not enough storage space";
        case ErrorCode_KeyGen:
            return "KeyGen too low to decrypt.";

        case ErrorCode_Unknown:
            return "Unkown Error";
        default:
            return "Unkown Error";
    }
}