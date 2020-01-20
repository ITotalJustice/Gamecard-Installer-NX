/*
* I wrote this header to be as portable as possible for other homebrew switch projects
* The only thing that needs to be changed will be the name / number of textures you want to load
* If you decide to use this header and add functions, please consider opening a pr for said functions
* I would greatly appreaciate it :)
*
* TotalJustice.
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>

#include "gfx/SDL_easy.h"


static SDL_Window   *main_window;
static SDL_Renderer *main_renderer;


SDL_Colour colours[] =
{
    { 255, 255, 255 },          //white
    { 140, 140, 140, 255 },     //grey
    { 81, 81, 81 },             //dark grey
    { 0, 0, 0 },                //black
    { 255, 192, 203 },          //pink
    { 253, 52, 131, 1 },        //neon pink
    { 255, 105, 180 },          //hotpink
    { 255, 165, 0 },            //orange
    { 255, 255, 0 },            //yellow
    { 255, 215, 0 },            //gold
    { 139, 69, 19 },            //brown
    { 255, 0, 0 },              //red
    { 139, 0, 0 },              //dark red
    { 0, 128, 0 },              //green
    { 50, 205, 50 },            //lime green
    { 0, 255, 255 },            //aqua
    { 0, 128, 128 },            //teal
    { 0, 191, 255 },            //light blue
    { 0, 0, 255 },              //blue
    { 131, 177, 218 },          //jordy blue
    { 97, 115, 255 },           //faint blue
    { 28, 33, 73 },             //dark blue
    { 160, 32, 240 },           //purple
    { 75, 0, 130 },             //indigo
    { 245, 245, 220 },          //beige
    { 45, 45, 45 },             //n_black
    { 50, 50, 50 },             //n_light_black
    { 114, 114, 114 },          //n_light_silver
    { 128, 128, 128 },          //n_silver
    { 178, 178, 178 },          //n_bright_Silver
    { 70, 70, 70 },             //n_dark_grey
    { 77, 77, 77 },             //n_grey
    { 251, 251, 251 },          //n_white
    { 0, 255, 200 },            //n_cyan
    { 143, 253, 252 },          //n_teal
    { 36, 141, 199 },           //n_blue
    { 27, 161, 255 },           //n_lightblue
    { 255, 177, 66 },           //n_yellow
    { 250, 90, 58 },            //n_red
    { 207, 36, 33 },            //glacia_red
    { 41, 30, 125 },            //glacia_blue
    { 99, 32, 160 },            //glacia_purple
    { 242, 0, 32 },             //tomi_red
    { 40, 48, 76 },             //law_blue
    { 136, 49, 57 },            //law_red
    { 37, 103, 75 },            //law_green
    { 4, 38, 37 },              //law_darkgreen
    { 28, 23, 62 },             //law_purple
    { 117, 3, 65 },             //law_fuchsia
    { 171, 61, 12 },            //law_orange
};


SDL_Colour SDL_GetColour(Colour colour_option)
{
    return colours[colour_option];
}

SDL_Window *SDL_GetWindow(void)
{
    return main_window;
}

SDL_Texture *__SDL_Image_Load(SDL_Surface *surface)
{
    if (!surface) return NULL;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(main_renderer, surface);
	SDL_FreeSurface(surface);
    return texture;
}

SDL_Texture *SDL_ImageLoad(const char *path)
{
    return IMG_LoadTexture(main_renderer, path);
}

SDL_Texture *SDL_ImageLoadMem(void *data, int size)
{
    return IMG_LoadTexture_RW(main_renderer, SDL_RWFromMem(data, size), 1);
}

void SDL_DrawText(TTF_Font *font, int x, int y, Colour colour, const char *text, ...)
{
    char full_text[0x100];
    va_list argv;
    va_start(argv, text);
    vsnprintf(full_text, sizeof(full_text), text, argv);
    va_end(argv);

    SDL_Colour col = SDL_GetColour(colour);
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, full_text, col);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(main_renderer, surface);
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = surface ->w, pos.h = surface->h };
    SDL_FreeSurface(surface);
    SDL_RenderCopy(main_renderer, tex, NULL, &pos);
    SDL_DestroyTexture(tex);
}

void SDL_DrawTextCenterX(TTF_Font *font, int y, int clip_x, int clip_w, Colour colour, const char *text, ...)
{
    char full_text[0x100];
    va_list argv;
    va_start(argv, text);
    vsnprintf(full_text, sizeof(full_text), text, argv);
    va_end(argv);

    int t_w = 0;
    int t_h = 0;
    SDL_GetTextSize(font, &t_w, &t_h, full_text);

    SDL_DrawText(font, ((clip_w - t_w) / 2) + clip_x, y, colour, full_text);
}

void SDL_DrawButton(TTF_Font *font, uint16_t btn, int x, int y, Colour colour)
{
    SDL_Colour col = SDL_GetColour(colour);
    SDL_Surface *surface = TTF_RenderGlyph_Blended(font, btn, col);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(main_renderer, surface);
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = surface ->w, pos.h = surface->h };

    SDL_FreeSurface(surface);
    SDL_RenderCopy(main_renderer, tex, NULL, &pos);
    SDL_DestroyTexture(tex);
}

void SDL_DrawImage(SDL_Texture *texture, int x, int y)
{
    SDL_Rect pos = { pos.x = x, pos.y = y };
	SDL_QueryTexture(texture, NULL, NULL, &pos.w, &pos.h);
	SDL_RenderCopy(main_renderer, texture, NULL, &pos);
}

void SDL_DrawImageRotate(SDL_Texture *texture, int x, int y, double rotate)
{
    SDL_Rect pos;
    SDL_Point center;

	SDL_QueryTexture(texture, NULL, NULL, &pos.w, &pos.h);

    pos.x = 0;
    pos.y = 0;

    int halfwidth = pos.w / 2;

    center.x = halfwidth;
    center.y = pos.h - halfwidth;

    SDL_RenderCopyEx(main_renderer, texture, NULL, &pos, rotate, &center, SDL_FLIP_NONE);
}

void SDL_DrawImageScale(SDL_Texture *texture, int x, int y, int w, int h)
{
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = w, pos.h = h };
	SDL_RenderCopy(main_renderer, texture, NULL, &pos);
}

void SDL_DrawShape(Colour colour, int x, int y, int w, int h, bool filled)
{
    SDL_Colour col = SDL_GetColour(colour);
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = w, pos.h = h };
    SDL_SetRenderDrawColor(main_renderer, col.r, col.g, col.b, col.a);
    filled ? SDL_RenderFillRect(main_renderer, &pos) : SDL_RenderDrawRect(main_renderer, &pos);
}

void SDL_DrawShapeOutline(Colour colour, int x, int y, int w, int h, int thicc)
{
    // top, right, bottom, left.
    SDL_DrawShape(colour, x, y, w, thicc, true);
    SDL_DrawShape(colour, x + w, y, thicc, h + thicc, true);
    SDL_DrawShape(colour, x, y + h, w, thicc, true);
    SDL_DrawShape(colour, x, y, thicc, h + thicc, true);
}

void SDL_DrawShapeRounded(Colour colour, int x, int y, int w, int h, int r)
{
    SDL_Colour col = SDL_GetColour(colour);
    roundedBoxRGBA(main_renderer, x, y,x + w, y + h, r, col.r, col.g, col.b, 255);
}

void SDL_DrawShapeRoundedOutline(Colour colour, int x, int y, int w, int h, int r)
{
    SDL_Colour col = SDL_GetColour(colour);
    roundedRectangleRGBA(main_renderer, x, y,x + w, y + h, r, col.r, col.g, col.b, 255);
}

void SDL_DrawCircle(Colour colour, int x, int y, int r)
{
    SDL_Colour col = SDL_GetColour(colour);
    filledCircleRGBA(main_renderer, x, y, r, col.r, col.g, col.b, 255);
}

void SDL_DrawCircleOutline(Colour colour, int x, int y, int r)
{
    SDL_Colour col = SDL_GetColour(colour);
    circleRGBA(main_renderer, x, y, r, col.r, col.g, col.b, 255);
}

void SDL_SurfaceToFile(SDL_Surface *surface, const char *path, bool to_free)
{
    SDL_SaveBMP(surface, path);
	SDL_FreeSurface(surface);
}

void SDL_TextureToFile(SDL_Texture *texture)
{}

SDL_Surface *__SDL_ScreenShot(void)
{
    SDL_Surface *surface = SDL_CreateRGBSurface(0, SCREEN_W, SCREEN_H, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_RenderReadPixels(main_renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch);
    return surface;
}

SDL_Texture *SDL_ScreenShotToTexture(void)
{
    return __SDL_Image_Load(__SDL_ScreenShot());
}

void SDL_ScreenShotToFile(const char *save_path)
{
    SDL_SurfaceToFile(__SDL_ScreenShot(), save_path, true);
}

int SDL_GetTextureWidth(SDL_Texture *texture)
{
    int w;
    SDL_QueryTexture(texture, NULL, NULL, &w, NULL);
    return w;
}

int SDL_GetTextureHeight(SDL_Texture *texture)
{
    int h;
    SDL_QueryTexture(texture, NULL, NULL, NULL, &h);
    return h;
}

void SDL_GetTextSize(TTF_Font *font, int *w, int *h, const char *text, ...)
{
    char full_text[0x100];
    va_list v;
    va_start(v, text);
    vsnprintf(full_text, sizeof(full_text), text, v);
    va_end(v);

    TTF_SizeUTF8(font, full_text, w, h);
}

void SDL_ClearRenderer(void)
{
    SDL_SetRenderDrawColor(main_renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(main_renderer);
}

void SDL_UpdateRenderer(void)
{
    SDL_RenderPresent(main_renderer);
}

void SDL_EasyInit(Window_Flags w_flags, Renderer_Flags r_flags)
{
    TTF_Init();
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    main_window = SDL_CreateWindow("totaljustice", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, w_flags);
    main_renderer = SDL_CreateRenderer(main_window, -1, r_flags);
}

void SDL_EasyExit(void)
{
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}