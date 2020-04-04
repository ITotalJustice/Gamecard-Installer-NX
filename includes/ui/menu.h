#ifndef _MENU_H_
#define _MENU_H_


#include <stdint.h>
#include <stdbool.h>

#include "gfx/shape.h"
#include "gfx/image.h"
#include "gfx/text.h"
#include "gfx/SDL_easy.h"

#include "util/error.h"


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

// for colour pulsing boxes.
typedef struct
{
    SDL_Colour col;
    bool increase_blue;
    uint8_t delay;
} PulseColour_t;

typedef struct
{
    PulseColour_t pulse;
    SDL_Rect rect;
} PulseShape_t;

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

typedef struct
{
    image_t *icon;
    text_t *title;
    text_t *author;
    text_t *text_app_id;
    text_t *text_key_gen;
    text_t *text_size;
    text_t *text_entry_contents;

    uint64_t app_id;
    uint8_t key_gen;
    size_t size;
    uint32_t total_count;
    uint16_t base_count;
    uint16_t upp_count;
    uint16_t dlc_count;
} GameInfo_t;

typedef struct
{
    text_t *type;
    text_t *size;
    text_t *name;
} GameInfoEntry_t;

typedef struct
{
    text_t *text_type;
    text_t *text_id;
    text_t *text_keygen;
    text_t *text_version;
    text_t *text_content_count;
    text_t *text_content_meta_count;

    uint8_t type;
    uint64_t id;
    uint8_t keygen;
    uint32_t version;
    uint16_t content_count;
    uint16_t content_meta_count;

    GameInfoEntry_t *entry; //array.
} GameInfoDetailed_t;


//
bool init_menu(void);
void exit_menu(void);


/*
*
*/

/*
//
bool is_lower_key_gen_enabled(void);
bool is_bl_enabled(void);
void set_lower_key_gen(bool on);
void set_bl(bool on);
*/


/*
*
*/

//
void update_button_spin(void);

//
void ui_display_error_box(ErrorCodes err, const char *func);

//
bool ui_display_yes_no_box(const char *message);

//
progress_bar_t *ui_init_progress_bar(const char *name, uint64_t speed, uint16_t eta_min, uint8_t eta_sec, size_t done, size_t remaining);
void ui_free_progress_bar(progress_bar_t *p_bar);
void ui_update_progress_bar(progress_bar_t *p_bar, uint64_t speed, uint16_t eta_min, uint8_t eta_sec, size_t done, size_t remaining);
void ui_display_progress_bar(progress_bar_t *p_bar);
void ui_display_dim_background(void);
void ui_draw_highlight_box(PulseShape_t *pulse_shape, int x, int y, int w, int h);

//
void render_menu(void);

// "main" for the menu.
void start_menu(void);

#endif