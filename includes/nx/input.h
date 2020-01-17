#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdint.h>
#include <stdbool.h>
#include <switch/services/hid.h>


typedef struct
{
    uint64_t down;
    uint64_t held;
    uint64_t up;

    uint32_t t_count;
    touchPosition t_pos;
} input_t;


//
input_t get_input(void);

//
bool check_if_touch_error(void);

//
int check_if_option(const input_t *input);

//
uint32_t move_cursor_up(uint32_t cursor, uint32_t cursor_max);

//
uint32_t move_cursor_down(uint32_t cursor, uint32_t cursor_max);

#endif