#ifndef _BUTTON_H_
#define _BUTTON_H_


#include <stdint.h>
#include <SDL2/SDL.h>

#include "gfx/font.h"


typedef struct
{
    SDL_Texture *tex;
    SDL_Rect rect;
} button_t;


//
button_t *create_button(font_t *font, int x, int y, uint8_t colour, uint16_t button);

//
void draw_button(button_t *button);

//
void draw_button_position(button_t *button, int x, int y);

//
void draw_button_scale(button_t *button, int w, int h);

//
void draw_button_set(button_t *button, int x, int y, int w, int h);

//
void position_button(button_t *button, int x, int y);

//
void free_button(button_t *t);

#endif