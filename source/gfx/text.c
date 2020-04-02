#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "gfx/SDL_easy.h"
#include "gfx/text.h"
#include "gfx/font.h"


bool __create_text(text_t *t, font_t *font, int x, int y, Colour colour, const char *text)
{
    t->font = font;
    t->colour = colour;
    t->rect.x = x;
    t->rect.y = y;
    
    SDL_Colour col = SDL_GetColour(colour);
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font->fnt, text, col);
    if (!surface)
    {
        return false;
    }

    t->tex = SDL_CreateTextureFromSurface(SDL_GetRenderer(SDL_GetWindow()), surface);
    SDL_FreeSurface(surface);
    if (!t->tex)
    {
        return false;
    }

    SDL_QueryTexture(t->tex, NULL, NULL, &t->rect.w, &t->rect.h);
    return true;
}

text_t *create_text(font_t *font, int x, int y, Colour colour, const char *text, ...)
{
    text_t *t = calloc(1, sizeof(text_t));

    char full_text[0x100];
    va_list argv;
    va_start(argv, text);
    vsnprintf(full_text, sizeof(full_text), text, argv);
    va_end(argv);

    return __create_text(t, font, x, y, colour, full_text) ? t : NULL;
}

void enable_text_clip(text_t *text, int start_x, int end_x)
{
    if (!text) return;
    text->clip.enabled = true;
    text->clip.start_x = start_x;
    text->clip.end_x = end_x;   
}

void disable_text_clip(text_t *text)
{
    if (!text) return;
    text->clip.enabled = false;
}

void draw_text(text_t *text)
{
    if (!text) return;
    SDL_Rect clip = { 0, 0, text->rect.w, text->rect.h };
    SDL_Rect dest = { text->rect.x, text->rect.y, text->rect.w, text->rect.h };
    if (text->clip.enabled)
    {
        clip.x = text->clip.start_x;
        clip.w = text->clip.end_x;
        dest.w = dest.w < text->clip.end_x ? dest.w : text->clip.end_x;
    }
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), text->tex, &clip, &dest);
}

void draw_text_position(text_t *text, int x, int y)
{
    if (!text) return;
    SDL_Rect clip = { 0, 0, text->rect.w, text->rect.h };
    SDL_Rect dest = { x, y, text->rect.w, text->rect.h };
    if (text->clip.enabled)
    {
        clip.x = text->clip.start_x;
        clip.w = text->clip.end_x;
        dest.w = dest.w < text->clip.end_x ? dest.w : text->clip.end_x;
    }
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), text->tex, &clip, &dest);
}

void draw_text_right_align(text_t *text, int end_x, int y)
{
    if (!text) return;
    SDL_Rect pos = { end_x - text->rect.w, y, text->rect.w, text->rect.h };
    SDL_Rect clip = { 0, 0, text->rect.w, text->rect.h };
    if (text->clip.enabled)
    {
        clip.x = text->clip.start_x;
        clip.w = text->clip.end_x;
    }
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), text->tex, &clip, &pos);
}

void draw_text_scale(text_t *text, int w, int h)
{
    if (!text) return;
    SDL_Rect pos = { text->rect.x, text->rect.y, w, h };
    SDL_Rect clip = { 0, 0, text->rect.w, text->rect.h };
    if (text->clip.enabled)
    {
        clip.x = text->clip.start_x;
        clip.w = text->clip.end_x;
    }
    SDL_RenderCopy(SDL_GetRenderer(SDL_GetWindow()), text->tex, &clip, &pos);
}

void draw_text_set(text_t *text, int x, int y, int w, int h)
{
    if (!text) return;
    text->rect.x = x;
    text->rect.y = y;
    text->rect.w = w;
    text->rect.h = h;
    draw_text(text);
}

void position_text(text_t *text, int x, int y)
{
    if (!text) return;
    text->rect.x = x;
    text->rect.y = y;
}

void set_text()
{}

void update_text(text_t *t, const char *text, ...)
{
    if (!t) return;
    SDL_DestroyTexture(t->tex);
    t->tex = NULL;

    char full_text[0x100];
    va_list argv;
    va_start(argv, text);
    vsnprintf(full_text, sizeof(full_text), text, argv);
    va_end(argv);

    __create_text(t, t->font, t->rect.x, t->rect.y, t->colour, full_text);
}

void free_text(text_t *t)
{
    if (!t)
        return;
    if (t->tex)
        SDL_DestroyTexture(t->tex);
    free(t);
}

