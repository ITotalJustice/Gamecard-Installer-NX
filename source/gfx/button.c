#include <SDL2/SDL.h>

#include "gfx/button.h"
#include "gfx/font.h"
#include "gfx/SDL_easy.h"


button_t *create_button(font_t *font, int x, int y, uint8_t colour, uint16_t button)
{
    button_t *t = malloc(sizeof(button_t));
    t->rect.x = x;
    t->rect.y = y;

    SDL_Colour col = SDL_GetColour(colour);
    SDL_Surface *surface = TTF_RenderGlyph_Blended(font->fnt, button, col);
    t->tex = SDL_CreateTextureFromSurface(SDL_GetRenderer(SDL_GetWindow()), surface);
    SDL_QueryTexture(t->tex, NULL, NULL, &t->rect.w, &t->rect.h);
    SDL_FreeSurface(surface);
    return t;
}

void draw_button(button_t *button)
{
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), button->tex, NULL, &button->rect);
}

void draw_button_position(button_t *button, int x, int y)
{
    SDL_Rect pos = { x, y, button->rect.w, button->rect.h };
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), button->tex, NULL, &pos);
}

void draw_button_scale(button_t *button, int w, int h)
{
    SDL_Rect pos = { button->rect.x, button->rect.y, w, h };
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), button->tex, NULL, &pos);
}

void draw_button_set(button_t *button, int x, int y, int w, int h)
{
    button->rect.x = x;
    button->rect.y = y;
    button->rect.w = w;
    button->rect.h = h;
    draw_button(button);
}

void position_button(button_t *button, int x, int y)
{
    button->rect.x = x;
    button->rect.y = y;
}

void set_button()
{}

void free_button(button_t *t)
{
    SDL_DestroyTexture(t->tex);
    free(t);
    t = NULL;
}