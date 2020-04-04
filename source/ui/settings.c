#include <stdint.h>
#include <stdbool.h>
#include <switch/services/ncm_types.h>

#include "ui/settings.h"

#include "util/log.h"

#include "sound/sound.h"


/*
*   Install Flags.
*/

SettingFlag g_install_base = SettingFlag_On;
SettingFlag g_install_upp = SettingFlag_On;
SettingFlag g_install_dlc = SettingFlag_On;
SettingsInstallLocation g_install_base_location = SettingsInstallLocation_Default;
SettingsInstallLocation g_install_upp_location = SettingsInstallLocation_Default;
SettingsInstallLocation g_install_dlc_location = SettingsInstallLocation_Default;
SettingFlag g_overwrite_newer_version = SettingFlag_Off;
SettingFlag g_install_lower_key_gen = SettingFlag_Off;
SettingFlag g_install_standard_crypto = SettingFlag_Off;
SettingFlag g_install_backlight = SettingFlag_On;


/*
*   Sound Flags.
*/

SettingFlag g_sounds_sound = SettingFlag_On;
SettingFlag g_sounds_music = SettingFlag_On;


/*
*   Install Getters.
*/

SettingFlag setting_get_install_base(void)
{
    return g_install_base;
}

SettingFlag setting_get_install_upp(void)
{
    return g_install_upp;
}

SettingFlag setting_get_install_dlc(void)
{
    return g_install_dlc;
}

SettingsInstallLocation setting_get_install_base_location(void)
{
    return g_install_base_location;
}

SettingsInstallLocation setting_get_install_upp_location(void)
{
    return g_install_upp_location;
}

SettingsInstallLocation setting_get_install_dlc_location(void)
{
    return g_install_dlc_location;
}

SettingFlag setting_get_overwrite_newer_version(void)
{
    return g_overwrite_newer_version;
}

SettingFlag setting_get_install_lower_key_gen(void)
{
    return g_install_lower_key_gen;
}

SettingFlag setting_get_install_standard_crypto(void)
{
    return g_install_standard_crypto;
}

SettingFlag setting_get_install_backlight(void)
{
    return g_install_backlight;
}


/*
*   Install Setters.
*/

void setting_set_install_base(SettingFlag flag)
{
    g_install_base = flag;
}

void setting_set_install_upp(SettingFlag flag)
{
    g_install_upp = flag;
}

void setting_set_install_dlc(SettingFlag flag)
{
    g_install_dlc = flag;
}

void setting_set_install_base_location(SettingsInstallLocation location)
{
    g_install_base_location = location;
}

void setting_set_install_upp_location(SettingsInstallLocation location)
{
    g_install_upp_location = location;
}

void setting_set_install_dlc_location(SettingsInstallLocation location)
{
    g_install_dlc_location = location;
}

void setting_set_overwrite_newer_version(SettingFlag flag)
{
    g_overwrite_newer_version = flag;
}

void setting_set_install_lower_key_gen(SettingFlag flag)
{
    g_install_lower_key_gen = flag;
}

void setting_set_install_standard_crypto(SettingFlag flag)
{
    g_install_standard_crypto = flag;
}

void setting_set_install_backlight(SettingFlag flag)
{
    g_install_backlight = flag;
}


/*
*   Sound Getters.
*/

SettingFlag setting_get_sound(void)
{
    return g_sounds_sound;
}

SettingFlag setting_get_music(void)
{
    return g_sounds_music;
}


/*
*   Sound Setters.
*/

void setting_set_sound(SettingFlag flag)
{
    pause_resume_sound();
    g_sounds_sound = flag;
}

void setting_set_music(SettingFlag flag)
{
    pause_resume_song();
    g_sounds_music = flag;
}