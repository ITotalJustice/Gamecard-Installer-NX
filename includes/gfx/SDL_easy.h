/*
* I wrote this header to be as portable as possible for other homebrew switch projects
* The only thing that needs to be changed will be the name / number of textures you want to load
* If you decide to use this header and add functions, please consider opening a pr for said functions
* I would greatly appreaciate it :)
*
* TotalJustice.
*/

#ifndef _SDL_EASY_H_
#define _SDL_EASY_H_

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_W    1280    //width of the window
#define SCREEN_H    720     //hight of the window
#define COLOUR_MAX  50

#define DEFAULT_FPS     60
#define TICKS_PER_FRAME (1000 / DEFAULT_FPS)


typedef enum
{
    Colour_White,
    Colour_Grey,
    Colour_DarkGrey,
    Colour_Black,
    Colour_Pink,
    Colour_NeonPink,
    Colour_HotPink,
    Colour_Orange,
    Colour_Yellow,
    Colour_Gold,
    Colour_Brown,
    Colour_Red,
    Colour_DarkRed,
    Colour_Green,
    Colour_LimeGreen,
    Colour_Aqua,
    Colour_Teal,
    Colour_LightBlue,
    Colour_Blue,
    Colour_JordyBlue,
    Colour_FaintBlue,
    Colour_DarkBlue,
    Colour_Purple,
    Colour_Indigo,
    Colour_Beige,
    Colour_Nintendo_Black,
    Colour_Nintendo_LightBlack,
    Colour_Nintendo_LightSilver,
    Colour_Nintendo_Silver,
    Colour_Nintendo_BrightSilver,
    Colour_Nintendo_DarkGrey,
    Colour_Nintendo_Grey,
    Colour_Nintendo_White,
    Colour_Nintendo_Cyan,
    Colour_Nintendo_Teal,
    Colour_Nintendo_Blue,
    Colour_Nintendo_LightBlue,
    Colour_Nintendo_Yellow,
    Colour_Nintendo_Red,
    Colour_Glacia_Red,
    Colour_Glacia_Blue,
    Colour_Glacia_Purple,
    Colour_Tomi_Red,
    Colour_Law_Blue,
    Colour_Law_Red,
    Colour_Law_Green,
    Colour_Law_DarkGreen,
    Colour_Law_Purple,
    Colour_Law_Fuchsia,
    Colour_Law_Orange,
} Colour;

typedef enum
{
    Font_Button_POWER = 0xE0B8,
    Font_Button_A = 0xE0E0,
    Font_Button_B = 0xE0E1,
    Font_Button_X = 0xE0E2,
    Font_Button_Y = 0xE0E3,
    Font_Button_L = 0xE0E4,
    Font_Button_R = 0xE0E5,
    Font_Button_ZL = 0xE0E6,
    Font_Button_ZR = 0xE0E7,
    Font_Button_SL = 0xE0E8,
    Font_Button_SR = 0xE0E9,
    Font_Button_UP = 0xE0EB,
    Font_Button_DOWN = 0xE0EC,
    Font_Button_LEFT = 0xE0ED,
    Font_Button_RIGHT = 0xE0EE,
    Font_Button_PLUS = 0xE0EF,
    Font_Button_MINUS = 0xE0F0,
    Font_Button_HOME = 0xE0F4,
    Font_Button_SCREENSHOT = 0xE0F5,
    Font_Button_LS = 0xE101,
    Font_Button_RS = 0xE102,
    Font_Button_L3 = 0xE104,
    Font_Button_R3 = 0xE105,
    Font_Button_SWITCH = 0xE121,
    font_Button_DUALCONS = 0xE122,
    Font_Button_SETTINGS = 0xE130,
    Font_Button_NEWS = 0xE132,
    Font_Button_ESHOP = 0xE133,
    Font_Button_ALBUM = 0xE134,
    Font_Button_ALLAPPS = 0xE135,
    Font_Button_CONTROLLER = 0xE136,
} Font; // buttons from NintendoExt


typedef uint32_t Window_Flags;
typedef uint32_t Renderer_Flags;


SDL_Colour SDL_GetColour(Colour colour_option);                                         //pass the name of colour, returns the colour
SDL_Window* SDL_GetWindow(void);                                                        //get sdl window

SDL_Texture *SDL_ImageLoad(const char *path);                                           //load image from texture
SDL_Texture *SDL_ImageLoadMem(void *data, int size);                                    //load image from memory

void SDL_DrawText(TTF_Font *font, int x, int y, Colour colour, const char *text, ...);  //draw text to screen
void SDL_DrawTextCenterX(TTF_Font *font, int y, int clip_x, int clip_w, Colour colour, const char *text, ...);
void SDL_DrawButton(TTF_Font *font, uint16_t btn, int x, int y, Colour colour);         //draw button to screen
void SDL_DrawImage(SDL_Texture *texture, int x, int y);                                 //draw image to screen from texture
void SDL_DrawImageRotate(SDL_Texture *texture, int x, int y, double rotate);
void SDL_DrawImageScale(SDL_Texture *texture, int x, int y, int w, int h);              //scale the image drawn to screen
void SDL_DrawShape(Colour colour, int x, int y, int w, int h, bool filled);             //draw shap (rect)
void SDL_DrawShapeEX(uint8_t r, uint8_t g, uint8_t b, uint8_t a, int x, int y, int w, int h, bool filled);
void SDL_DrawShapeOutline(Colour colour, int x, int y, int w, int h, int thicc);
void SDL_DrawShapeOutlineEX(uint8_t r, uint8_t g, uint8_t b, uint8_t a, int x, int y, int w, int h, int thicc);
void SDL_DrawShapeRounded(Colour colour, int x, int y, int w, int h, int r);
void SDL_DrawShapeRoundedOutline(Colour colour, int x, int y, int w, int h, int r);
void SDL_DrawCircle(Colour colour, int x, int y, int r);
void SDL_DrawCircleOutline(Colour colour, int x, int y, int r);

void SDL_SurfaceToFile(SDL_Surface *surface, const char *path, bool to_free);
SDL_Texture *SDL_ScreenShotToTexture(void);
void SDL_ScreenShotToFile(const char *save_path);

int SDL_GetTextureWidth(SDL_Texture *texture);
int SDL_GetTextureHeight(SDL_Texture *texture);
void SDL_GetTextSize(TTF_Font *font, int *w, int *h, const char *text, ...);

void SDL_SetTextureColour(SDL_Texture *texture, Colour colour);

void SDL_ClearRenderer(void);                                                           //clear the screen
void SDL_UpdateRenderer(void);                                                          //update the screen

void SDL_EasyInit(Window_Flags w_flags, Renderer_Flags r_flags);                        //init all sdl stuff
void SDL_EasyExit(void);                                                                //clean and exit

#endif