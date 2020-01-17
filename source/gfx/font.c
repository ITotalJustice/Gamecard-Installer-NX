#include <SDL2/SDL_ttf.h>
#include <switch.h>

#include "gfx/font.h"


font_t create_font_file(const char *file, int font_size)
{
    font_t font;
    font.fnt = TTF_OpenFontRW(SDL_RWFromFile(file, "rb"), 1, font_size);
    return font;
}

font_t create_font_mem(void *mem, int mem_size, int font_size)
{
    font_t font;
    font.fnt = TTF_OpenFontRW(SDL_RWFromMem(mem, mem_size), 1, font_size);
    return font;
}

void free_font(font_t *font)
{
    TTF_CloseFont(font->fnt);
}

bool init_font()
{
    if (R_FAILED(plInitialize()))
    {
        return false;
    }

    PlFontData font = {0};
    PlFontData button = {0};
    if (R_FAILED(plGetSharedFontByType(&font, PlSharedFontType_Standard)))
    {
        return false;
    }
    if (R_FAILED(plGetSharedFontByType(&button, PlSharedFontType_NintendoExt)))
    {
        return false;
    }

    int font_sizes[] = { 15, 18, 20, 23, 25, 28, 30, 33, 35, 45, 48, 60, 63, 72, 170 };

    for (uint8_t i = 0; i < FONT_MAX; i++)
    {
        FONT_BUTTON[i] = create_font_mem(button.address, button.size, font_sizes[i]);
        FONT_TEXT[i] = create_font_mem(font.address, font.size, font_sizes[i]);
    }

    return true;
}

void exit_font()
{
    for (uint8_t i = 0; i < FONT_MAX; i++)
    {
        free_font(&FONT_TEXT[i]);
        free_font(&FONT_BUTTON[i]);
    }

    plExit();
}