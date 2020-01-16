#ifndef _BOX_H_
#define _BOX_H_

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "gfx/shape.h"
#include "gfx/text.h"


typedef struct
{
    SDL_Rect rect;
    text_t *title;
    shape_t spacer_top;
    shape_t spacer_bottom;
    _Bool is_on; // flag to show either on or off text.
    text_t *on_text;
    text_t *off_text;
} box_t;


//
box_t *create_box(int x, int y, int w, int h, _Bool is_on, const char *title);

//
void draw_box(box_t *box);

//
void free_box(box_t *box);


#endif