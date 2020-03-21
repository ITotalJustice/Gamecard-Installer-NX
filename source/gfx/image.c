#include <stdlib.h>
#include <SDL2/SDL.h>

#include "gfx/SDL_easy.h"
#include "gfx/image.h"


image_t *__create_image(SDL_Texture *texture, int x, int y, int w, int h)
{
    image_t *image = malloc(sizeof(image_t));
    image->texture = texture;
    int curr_w;
    int curr_h;
    SDL_QueryTexture(image->texture, NULL, NULL, &curr_w, &curr_h);
    if (w) curr_w = w;
    if (h) curr_h = h;
    image->rect.x = x;  image->rect.y = y; image->rect.w = curr_w; image->rect.h = curr_h;
    return image;
}

image_t *create_image_from_file(const char *file, int x, int y, int w, int h)
{
    SDL_Texture * texture = SDL_ImageLoad(file);
    return __create_image(texture, x, y, w, h);
}

image_t *create_image_from_mem(void *mem, size_t mem_size, int x, int y, int w, int h)
{
    SDL_Texture * texture = SDL_ImageLoadMem(mem, mem_size);
    return __create_image(texture, x, y, w, h);
}

void draw_image2(image_t *image)
{
    if (!image) return;
    SDL_DrawImageScale(image->texture, image->rect.x, image->rect.y, image->rect.w, image->rect.h);
}

void draw_image_scale(image_t *image, int w, int h)
{
    if (!image) return;
    SDL_DrawImageScale(image->texture, image->rect.x, image->rect.y, w, h);
}

void draw_image_position(image_t *image, int x, int y)
{
    if (!image) return;
    SDL_DrawImageScale(image->texture, x, y, image->rect.w, image->rect.h);
}

void draw_image_set(image_t *image, int x, int y, int w, int h)
{
    if (!image) return;
    SDL_DrawImageScale(image->texture, x, y, w, h);
}

void set_image(image_t *image, int x, int y, int w, int h)
{
    if (!image) return;
    image->rect.x = x;  image->rect.y = y; image->rect.w = w; image->rect.h = h;
}

void free_image(image_t *image)
{
    if (!image) return;
    SDL_DestroyTexture(image->texture);
    free(image);
}