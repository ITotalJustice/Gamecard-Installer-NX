#include <stdarg.h>
#include <stdio.h>
#include "gfx/SDL_easy.h"
#include "gfx/text.h"
#include "gfx/font.h"


text_t *create_text(font_t *font, int x, int y, Colour colour, const char *text, ...)
{
    char full_text[0x100];
    va_list argv;
    va_start(argv, text);
    vsnprintf(full_text, sizeof(full_text), text, argv);
    va_end(argv);

    text_t *t = malloc(sizeof(text_t));
    t->rect.x = x;
    t->rect.y = y;
    
    SDL_Colour col = SDL_GetColour(colour);
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font->fnt, full_text, col);
    t->tex = SDL_CreateTextureFromSurface(SDL_GetRenderer(SDL_GetWindow()), surface);
    SDL_QueryTexture(t->tex, NULL, NULL, &t->rect.w, &t->rect.h);
    SDL_FreeSurface(surface);
    return t;

}

void draw_text(text_t *text)
{
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), text->tex, NULL, &text->rect);
}

void draw_text_position(text_t *text, int x, int y)
{
    SDL_Rect pos = { x, y, text->rect.w, text->rect.h };
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), text->tex, NULL, &pos);
}

void draw_text_scale(text_t *text, int w, int h)
{
    SDL_Rect pos = { text->rect.x, text->rect.y, w, h };
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), text->tex, NULL, &pos);
}

void draw_text_set(text_t *text, int x, int y, int w, int h)
{
    text->rect.x = x;
    text->rect.y = y;
    text->rect.w = w;
    text->rect.h = h;
    draw_text(text);
}

void position_text(text_t *text, int x, int y)
{
    text->rect.x = x;
    text->rect.y = y;
}

void set_text()
{}

void free_text(text_t *t)
{
    if (!t) return;
    SDL_DestroyTexture(t->tex);
    free(t);
}

