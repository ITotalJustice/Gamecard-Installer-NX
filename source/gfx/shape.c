#include <stdint.h>
#include <stdbool.h>

#include "gfx/SDL_easy.h"
#include "gfx/shape.h"


shape_t create_shape(uint8_t colour, int x, int y, int w, int h, bool filled)
{
    shape_t shape;
    shape.info.rect.x = x;
    shape.info.rect.y = y;
    shape.info.rect.w = w;
    shape.info.rect.h = h;
    shape.info.colour = colour;
    shape.info.filled = filled;
    return shape;
}

void draw_shape(shape_t *shape)
{
    SDL_DrawShape(shape->info.colour, shape->info.rect.x, shape->info.rect.y, shape->info.rect.w, shape->info.rect.h, shape->info.filled);
}

void draw_shape_position(shape_t *shape, int x, int y)
{
    SDL_DrawShape(shape->info.colour, x, y, shape->info.rect.w, shape->info.rect.h, shape->info.filled);
}

void draw_shape_scale(shape_t *shape, int w, int h)
{
    SDL_DrawShape(shape->info.colour, shape->info.rect.x, shape->info.rect.y, w, h, shape->info.filled);
}

void draw_shape_resize(shape_t *shape, int x, int y, int w, int h)
{
    SDL_DrawShape(shape->info.colour, x, y, w, h, shape->info.filled);
}

void set_shape(shape_t *shape, uint8_t colour, int x, int y, int w, int h, bool filled)
{
    shape->info.rect.x = x;
    shape->info.rect.y = y;
    shape->info.rect.w = w;
    shape->info.rect.h = h;
    shape->info.colour = colour;
    shape->info.filled = filled;
}

//void draw_shapes()
void add_shape_entry(shapes_t **shapes, shape_t new)
{}

void delete_shape_entry()
{}

void free_shapes()
{}