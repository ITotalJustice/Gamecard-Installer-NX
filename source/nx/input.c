#include <stdint.h>
#include <stdbool.h>
#include <switch.h>

#include "nx/input.h"


input_t get_input(void)
{
    hidScanInput();
    input_t input = {0};
    input.down = hidKeysDown(CONTROLLER_P1_AUTO);
    input.held = hidKeysHeld(CONTROLLER_P1_AUTO);
    input.t_count = hidTouchCount();
    hidTouchRead(&input.t_pos, 0);
    return input;
}


bool check_if_touch_error(void)
{
    input_t input = get_input();
    if (input.t_count)
        if (input.t_pos.px > 455 && input.t_pos.px < 455 + 365 && input.t_pos.py > 470 && input.t_pos.py < 470 + 65)
            return true;
    if ((input.down & KEY_A || input.down & KEY_B))
        return true;
    return false;
}

bool check_if_touch_yes(void)
{
    input_t input = get_input();

    // TODO:

    return false;
}

bool check_if_touch_no(void)
{
    input_t input = get_input();

    // TODO:

    return false;
}

int check_if_option(const input_t *input)
{
    for (uint16_t i = 0, y = 300; i < 3; i++, y += 125)
    {
        if (input->t_pos.px > 475 && input->t_pos.px < 475 + 720 && input->t_pos.py > y && input->t_pos.py < y + 70)
            return i;
    }
    return -1;
}

uint32_t move_cursor_up(uint32_t cursor, uint32_t cursor_max)
{
    return cursor == 0 ? cursor_max - 1 : --cursor;
}

uint32_t move_cursor_down(uint32_t cursor, uint32_t cursor_max)
{
    return cursor == cursor_max - 1 ? 0 : ++cursor;
}