#ifndef _TEXT_H_
#define _TEXT_H_

#include <SDL2/SDL.h>

#include "gfx/SDL_easy.h"
#include "gfx/font.h"


typedef struct
{
    SDL_Texture *tex;
    SDL_Rect rect;
    font_t *font;
    Colour colour;
    struct
    {
        bool enabled;
        int start_x;
        int end_x;
    } clip;
} text_t;


//
text_t *create_text(font_t *font, int x, int y, Colour colour, const char *text, ...);

//
void enable_text_clip(text_t *text, int start_x, int end_x);
void disable_text_clip(text_t *text);

//
void draw_text(text_t *text);
void draw_text_position(text_t *text, int x, int y);
void draw_text_right_align(text_t *text, int end_x, int y);
void draw_text_scale(text_t *text, int w, int h);
void draw_text_set(text_t *text, int x, int y, int w, int h);

//
void position_text(text_t *text, int x, int y);

//
void set_text();

//
void update_text(text_t *t, const char *text, ...);

//
void free_text(text_t *t);

#endif