#include <stdint.h>
#include <stdbool.h>
#include <switch.h>

#include "nx/set.h"


bool init_set(void)
{
    return R_FAILED(setsysInitialize()) ? false : true;
}

void exit_set(void)
{
    setsysExit();
}

uint16_t get_sys_fw_version(void)
{
    SetSysFirmwareVersion fw_ver = {0};
    setsysGetFirmwareVersion(&fw_ver);
    return (fw_ver.major * 100) + (fw_ver.micro * 10) + (fw_ver.minor);
}