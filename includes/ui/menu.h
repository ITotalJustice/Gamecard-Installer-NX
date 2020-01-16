#ifndef _MENU_H_
#define _MENU_H_


#include <stdint.h>
#include <stdbool.h>


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


//
bool init_menu(void);

//
void exit_menu(void);

//
void update_button_spin(void);

//
void ui_display_progress_bar(const char *name, int speed, int eta_min, int eta_sec, size_t done, size_t remaining);

//
void render_menu(void);

// "main" for the menu.
void start_menu(void);

#endif