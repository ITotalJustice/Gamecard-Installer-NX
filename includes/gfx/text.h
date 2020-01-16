#ifndef _TEXT_H_
#define _TEXT_H_

#include <SDL2/SDL.h>
#include "gfx/SDL_easy.h"
#include "gfx/font.h"


typedef struct
{
    SDL_Texture *tex;
    SDL_Rect rect;
} text_t;


//
text_t *create_text(font_t *font, int x, int y, Colour colour, const char *text, ...);

//
void draw_text(text_t *text);

//
void draw_text_position(text_t *text, int x, int y);

//
void draw_text_scale(text_t *text, int w, int h);

//
void draw_text_set(text_t *text, int x, int y, int w, int h);

//
void position_text(text_t *text, int x, int y);

//
void set_text();

//
void free_text(text_t *t);

#endif