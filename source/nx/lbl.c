#include <stdbool.h>
#include <switch/services/lbl.h>

#include "nx/lbl.h"


bool init_lbl(void)
{
    return R_SUCCEEDED(lblInitialize());
}

void exit_lbl(void)
{
    lblExit();
}

LblBacklightSwitchStatus get_backlight_status(void)
{
    LblBacklightSwitchStatus status;
    lblGetBacklightSwitchStatus(&status);
    return status;
}

bool is_backlight_enabled(void)
{
    LblBacklightSwitchStatus status = get_backlight_status();
    return status == LblBacklightSwitchStatus_Enabled || status == LblBacklightSwitchStatus_Enabling;
}

bool enable_backlight(BacklightFade fade)
{
    return lblSwitchBacklightOn(fade);
}

bool disable_backlight(BacklightFade fade)
{
    return lblSwitchBacklightOff(fade);
}