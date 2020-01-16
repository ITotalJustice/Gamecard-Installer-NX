#include <stdint.h>
#include <switch.h>

#include "nx/input.h"


input_t get_input(void)
{
    hidScanInput();
    input_t input;
    input.down = hidKeysDown(CONTROLLER_P1_AUTO);
    input.held = hidKeysHeld(CONTROLLER_P1_AUTO);
    input.t_count = hidTouchCount();
    hidTouchRead(&input.t_pos, 1);
    return input;
}

uint32_t move_cursor_up(uint32_t cursor, uint32_t cursor_max)
{
    return cursor == 0 ? cursor_max - 1 : --cursor;
}

uint32_t move_cursor_down(uint32_t cursor, uint32_t cursor_max)
{
    return cursor == cursor_max - 1 ? 0 : ++cursor;
}