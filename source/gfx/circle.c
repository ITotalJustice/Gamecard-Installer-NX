#include <stdint.h>
#include "circle.h"
#include "SDL_easy.h"


circle_t create_circle(int x, int y, int r, uint8_t colour)
{
    circle_t circle = { x, y, r, colour };
    return circle;
}

void draw_circle(circle_t *circle)
{
    SDL_DrawCircle(circle->colour, circle->x, circle->y, circle->r);
}

void draw_circle_position(circle_t *circle, int x, int y)
{
    SDL_DrawCircle(circle->colour, x, y, circle->r);
}

void draw_circle_scale(circle_t *circle, int r)
{
    SDL_DrawCircle(circle->colour, circle->x, circle->y, r);
}

void draw_circle_colour(circle_t *circle, uint8_t colour)
{
    SDL_DrawCircle(colour, circle->x, circle->y, circle->r);
}

void draw_circle_set(circle_t *circle, int x, int y, int r, uint8_t colour)
{
    circle->colour = colour;
    circle->x = x;
    circle->y = y;
    circle->r = r;
    draw_circle(circle);
}

void set_circle(circle_t *circle, int x, int y, int r, uint8_t colour)
{
    circle->colour = colour;
    circle->x = x;
    circle->y = y;
    circle->r = r;
}