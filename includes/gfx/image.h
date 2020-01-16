#ifndef _IMAGE_H_
#define _IMAGE_H_


#include <stdint.h>
#include <SDL2/SDL.h>
//#include "SDL_easy.h"


typedef struct
{
    SDL_Texture *texture;
    SDL_Rect rect;
} image_t;


//
image_t *create_image_from_file(const char *file, int x, int y, int w, int h);

//
image_t *create_image_from_mem(void *mem, size_t mem_size, int x, int y, int w, int h);

//
void draw_image2(image_t *image);

//
void draw_image_scale(image_t *image, int w, int h);

//
void draw_image_position(image_t *image, int x, int y);

//
void draw_image_set(image_t *image, int x, int y, int w, int h);

//
void set_image(image_t *image, int x, int y, int w, int h);

//
void free_image(image_t *image);

#endif