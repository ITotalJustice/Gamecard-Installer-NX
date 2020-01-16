#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>


typedef struct
{
    SDL_Rect rect;
    uint8_t colour;
    bool filled;
} shape_info_t;

typedef struct
{
    shape_info_t info;
} shape_t;

typedef struct _shape
{
    shape_t shape;
    bool selected;
    struct _shape *next;
    struct _shape *prev;
} shapes_t;


//
shape_t create_shape(uint8_t colour, int x, int y, int w, int h, bool filled);

//
void draw_shape(shape_t *shape);

//
void draw_shape_position(shape_t *shape, int x, int y);

//
void draw_shape_scale(shape_t *shape, int w, int h);

//
void set_shape(shape_t *shape, uint8_t colour, int x, int y, int w, int h, bool filled);

#endif