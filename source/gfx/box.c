#include <stdint.h>
#include <stdbool.h>

#include "gfx/box.h"
#include "gfx/shape.h"
#include "gfx/text.h"
#include "gfx/SDL_easy.h"


box_t *create_box(int x, int y, int w, int h, _Bool is_on, const char *title)
{
    box_t *box = malloc(sizeof(box_t));
    box->rect.x = x; box->rect.y = y; box->rect.w = w; box->rect.h = h;
    box->is_on = is_on;
    box->on_text = create_text(&FONT_TEXT[QFontSize_20], 1150, box->rect.y + 30, Colour_Nintendo_Cyan, "On");
    box->off_text = create_text(&FONT_TEXT[QFontSize_20], 1150, box->rect.y + 30, Colour_Nintendo_Grey, "Off");
    box->title = create_text(&FONT_TEXT[QFontSize_20], box->rect.x + 15, box->rect.y + 25, Colour_Nintendo_White, title);
    box->spacer_top = create_shape(Colour_Nintendo_Silver, box->rect.x, box->rect.y, box->rect.w, box->rect.h, true);
    box->spacer_bottom = create_shape(Colour_Nintendo_Silver, box->rect.x, box->rect.y + 75, box->rect.w, box->rect.h, true);
    return box;
}

void draw_box(box_t *box)
{
    draw_shape(&box->spacer_top);
    draw_text(box->title);
    draw_text(box->is_on == true ? box->on_text : box->off_text);
    draw_shape(&box->spacer_bottom);
}

void free_box(box_t *box)
{
    free_text(box->title);
    free_text(box->on_text);
    free_text(box->off_text);
    free(box);
}