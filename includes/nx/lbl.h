#include <stdbool.h>
#include <switch/services/lbl.h>


typedef enum
{
    BacklightFade_Instant = 0x0,
    BacklightFade_VeryFast = 0x10000000,
    BacklightFade_Fast = 0x20000000,
    BacklightFade_Slow = 0x40000000,
    BacklightFade_VerySlow = 0x80000000,
} BacklightFade;


//
bool init_lbl(void);
void exit_lbl(void);

//
LblBacklightSwitchStatus get_backlight_status(void);

//
bool is_backlight_enabled(void);
bool enable_backlight(BacklightFade fade);
bool disable_backlight(BacklightFade fade);