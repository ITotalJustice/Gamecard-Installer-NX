#ifndef _CIRCLE_H_
#define _CIRCLE_H_

#include <stdint.h>


typedef struct
{
    int x;
    int y;
    uint8_t r;
    uint8_t colour;
} circle_t;


// create a circle.
circle_t create_circle(int x, int y, int r, uint8_t colour);

// draw a circle.
void draw_circle(circle_t *circle);

// draw a circle with a new position.
// does not save the position of the circle to the struct.
void draw_circle_position(circle_t *circle, int x, int y);

//
void draw_circle_scale(circle_t *circle, int r);

// draw the circle with a new colour.
// does not save the colour to the circle struct.
void draw_circle_colour(circle_t *circle, uint8_t colour);

// draw the circle with 
void draw_circle_set(circle_t *circle, int x, int y, int r, uint8_t colour);

// re set the information of the circle.
void set_circle(circle_t *circle, int x, int y, int r, uint8_t colour);

#endif