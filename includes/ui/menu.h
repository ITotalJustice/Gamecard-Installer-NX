#ifndef _MENU_H_
#define _MENU_H_


#include <stdint.h>
#include <stdbool.h>

#include "gfx/shape.h"
#include "gfx/text.h"


enum
{
    Font_Tetris_TL = 0xE020,
    Font_Tetris_T = 0xE021,
    Font_Tetris_TR = 0xE022,
    Font_Tetris_R = 0xE023,
    Font_Tetris_BR = 0xE024,
    Font_Tetris_B = 0xE025,
    Font_Tetris_BL = 0xE026,
    Font_Tetris_L = 0xE027,
};

typedef struct
{
    shape_t empty_bar;
    shape_t filled_bar;

    text_t *text_header;
    text_t *text_warning1;
    text_t *text_warning2;
    text_t *text_name;
    text_t *text_speed;
    text_t *text_time;

    size_t speed;
    uint16_t eta_min;
    uint8_t eta_sec;
} progress_bar_t;


//
bool init_menu(void);

//
void exit_menu(void);

//
bool is_lower_key_gen_enabled(void);

//
bool is_bl_enabled(void);

//
void update_button_spin(void);

//
void ui_display_error_box(uint32_t err);

//
bool ui_display_yes_no_box(const char *message);

//
progress_bar_t *ui_init_progress_bar(const char *name, uint64_t speed, uint16_t eta_min, uint8_t eta_sec, size_t done, size_t remaining);

//
void ui_free_progress_bar(progress_bar_t *p_bar);

//
void ui_update_progress_bar(progress_bar_t *p_bar, uint64_t speed, uint16_t eta_min, uint8_t eta_sec, size_t done, size_t remaining);

//
void ui_display_progress_bar(progress_bar_t *p_bar);

//
void render_menu(void);

// "main" for the menu.
void start_menu(void);

#endif