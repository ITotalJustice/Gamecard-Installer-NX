#ifndef _SETINGS_H_
#define _SETTINGS_H_


#include <switch/services/ncm_types.h>


typedef enum
{
    SettingFlag_Off = 0,
    SettingFlag_On = 1,
} SettingFlag;

typedef enum
{
    SettingsInstallLocation_Default = -1,
    SettingsInstallLocation_User = NcmStorageId_BuiltInUser,
    SettingsInstallLocation_SdCard = NcmStorageId_SdCard,
} SettingsInstallLocation;


/*
*   Install Getters.
*/

SettingFlag setting_get_install_base(void);
SettingFlag setting_get_install_upp(void);
SettingFlag setting_get_install_dlc(void);
SettingsInstallLocation setting_get_install_base_location(void);
SettingsInstallLocation setting_get_install_upp_location(void);
SettingsInstallLocation setting_get_install_dlc_location(void);
SettingFlag setting_get_overwrite_newer_version(void);
SettingFlag setting_get_install_lower_key_gen(void);
SettingFlag setting_get_install_standard_crypto(void);
SettingFlag setting_get_install_backlight(void);


/*
*   Install Setters.
*/

//
void setting_set_install_base(SettingFlag flag);
void setting_set_install_upp(SettingFlag flag);
void setting_set_install_dlc(SettingFlag flag);
void setting_set_install_base_location(SettingsInstallLocation location);
void setting_set_install_upp_location(SettingsInstallLocation location);
void setting_set_install_dlc_location(SettingsInstallLocation location);
void setting_set_overwrite_newer_version(SettingFlag flag);
void setting_set_install_lower_key_gen(SettingFlag flag);
void setting_set_install_standard_crypto(SettingFlag flag);
void setting_set_install_backlight(SettingFlag flag);


/*
*   Sound Getters.
*/

//
SettingFlag setting_get_sound(void);
SettingFlag setting_get_music(void);


/*
*   Sound Setters.
*/

void setting_set_sound(SettingFlag flag);
void setting_set_music(SettingFlag flag);

#endif