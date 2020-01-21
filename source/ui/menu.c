#include <stdint.h>
#include <stdbool.h>
#include <switch.h>

#include "ui/menu.h"
#include "ui/gc.h"

#include "gfx/SDL_easy.h"
#include "gfx/image.h"
#include "gfx/shape.h"
#include "gfx/text.h"
#include "gfx/button.h"

#include "sound/sound.h"

#include "nx/ns.h"
#include "nx/ncm.h"
#include "nx/input.h"


#define TITLE_NAME      "GameCard Installer"
#define EMPTY_ICON_PATH "romfs:/no_icon.jpg"
#define STORAGE_BAR_W   325
#define STORAGE_BAR_H   15


typedef enum
{
    Option_Nand,
    Option_SD,
    Option_Exit,
} Option;

const char *g_option_list[] =
{
    "Nand Install",
    "Sd Card Install",
    "Exit",
};

const char *g_sound_list[] =
{
    "romfs:/sound/error.wav",
    "romfs:/sound/insert.wav",
    "romfs:/sound/select.wav",
    "romfs:/sound/popup.wav",
};

// background shapes.
typedef struct
{
    shape_t background;
    shape_t side_background;
    shape_t upper_bar;
    shape_t lower_bar;
    shape_t selected_bar;
    text_t *title;
    text_t *a_text;
    text_t *b_text;
    button_t *a_button;
    button_t *b_button;
    button_t *app_icon;
} background_t;
background_t g_background = {0};

typedef struct
{
    text_t *text;
    bool selected;
    bool avaliable;
} options_t;
options_t g_options[3] = {0};

// sound effects.
typedef struct
{
    sound_t *error;
    sound_t *insert;
    sound_t *move;
    sound_t *popup;
} sound_effects_t;
sound_effects_t g_sound_effects = {0};

// for colour pulsing boxes.
typedef struct
{
    SDL_Colour col;
    bool increase_blue;
    uint8_t delay;
} pulse_colour_t;

typedef struct
{
    pulse_colour_t pulse;
    SDL_Rect rect;
} pulsing_shape_t;

typedef struct
{
    size_t free;
    size_t total;
    float size_gb;
    shape_t bar;
    shape_t filled;
    text_t *space_text;
} storage_t;

storage_t g_nand_storage = {0};
storage_t g_sd_storage = {0};

// the gamecard struct that will contain info on the gc, as well as gfx data.
gamecard_t gamecard = {0};

// the default icon that is rendered if no gamecard is inserted.
image_t *g_empty_icon = {0};

// global cursor for the main menu.
uint8_t g_cursor = 0;

// tetris spinning piece.
button_t *g_tetris_button[0x8] = {0};
uint8_t g_tetris_spin_pos = 0;

// global flag for when the gamecard is inserted, changes render options.
bool g_gc_inserted = false;



/*
*   Init and exit.
*/

void setup_background(void)
{
    // shapes.
    g_background.background = create_shape(Colour_Nintendo_Black, 0, 0, SCREEN_W, SCREEN_H, true);
    g_background.side_background = create_shape(Colour_Nintendo_LightBlack, 30, 90, 375, 555, true);
    g_background.upper_bar = create_shape(Colour_Nintendo_White, 30, (SCREEN_H / 8), SCREEN_W - (30 * 2), 2, true);
    g_background.lower_bar = create_shape(Colour_Nintendo_White, 30, 650, SCREEN_W - (30 * 2), 2, true);
    g_background.selected_bar = create_shape(Colour_Nintendo_Cyan, 90, 230, 4, 45, true);

    // title and icon.
    g_background.title = create_text(&FONT_TEXT[QFontSize_25], 130, 40, Colour_Nintendo_White, TITLE_NAME);
    g_background.app_icon = create_button(&FONT_BUTTON[QFontSize_45], 65, 30, Colour_Nintendo_White, Font_Button_SETTINGS);

    // buttons.
    g_background.a_button = create_button(&FONT_BUTTON[QFontSize_25], 1150, 675, Colour_Nintendo_White, Font_Button_A);
    g_background.b_button = create_button(&FONT_BUTTON[QFontSize_25], 1055 - 35, 675, Colour_Nintendo_White, Font_Button_B);
    g_background.a_text = create_text(&FONT_TEXT[QFontSize_20], 1185, 675, Colour_Nintendo_White, "OK");
    g_background.b_text = create_text(&FONT_TEXT[QFontSize_20], 1055, 675, Colour_Nintendo_White, "Back");
}

void setup_options(void)
{
    for (uint16_t i = 0, y = 300; i < 3; i++, y += 125)
    {
        g_options[i].text = create_text(&FONT_TEXT[QFontSize_23], 500, y, Colour_Nintendo_White, g_option_list[i]);
    }   
}

void setup_storage(void)
{
    // setup the nand storage.
    g_nand_storage.free = ns_get_storage_free_space(NcmStorageId_BuiltInUser);
    g_nand_storage.total = ns_get_storage_total_size(NcmStorageId_BuiltInUser);
    g_nand_storage.bar = create_shape(Colour_Nintendo_White, 480, 170, STORAGE_BAR_W, 14, false);
    g_nand_storage.filled = create_shape(Colour_Nintendo_White, 480 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_nand_storage.free / (float)g_nand_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);
    g_nand_storage.size_gb = (float)g_nand_storage.free / 0x40000000;
    g_nand_storage.space_text = create_text(&FONT_TEXT[QFontSize_23], 490, 135, Colour_Nintendo_White, "System memory %.1fGB", g_nand_storage.size_gb);

    // setup the sd card storage.
    g_sd_storage.free = ns_get_storage_free_space(NcmStorageId_SdCard);
    g_sd_storage.total = ns_get_storage_total_size(NcmStorageId_SdCard);
    g_sd_storage.bar = create_shape(Colour_White, 860, 170, STORAGE_BAR_W, 14, false);
    g_sd_storage.filled = create_shape(Colour_Nintendo_White, 860 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_sd_storage.free / (float)g_sd_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);
    g_sd_storage.size_gb = (float)g_sd_storage.free / 0x40000000;
    g_sd_storage.space_text = create_text(&FONT_TEXT[QFontSize_23], 870, 135, Colour_Nintendo_White, "microSD card %.1fGB", g_sd_storage.size_gb);
}

void setup_sound_effects(void)
{
    g_sound_effects.error = load_sound_from_file(g_sound_list[0]);
    g_sound_effects.insert = load_sound_from_file(g_sound_list[1]);
    g_sound_effects.move = load_sound_from_file(g_sound_list[2]);
    g_sound_effects.popup = load_sound_from_file(g_sound_list[3]);
}

void setup_teris(void)
{
    for (uint16_t i = 0, button = Font_Tetris_TL; button <= Font_Tetris_L; i++, button++)
    {
        g_tetris_button[i] = create_button(&FONT_BUTTON[QFontSize_33], 50, SCREEN_H - 50, Colour_Nintendo_White, button);
    }
}

void free_background(void)
{
    free_text(g_background.title);
    free_button(g_background.app_icon);
    free_button(g_background.a_button);
    free_button(g_background.b_button);
    free_text(g_background.a_text);
    free_text(g_background.b_text);
}

void free_options(void)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        free_text(g_options[i].text);
    }
}

void free_storage(void)
{
    free_text(g_nand_storage.space_text);
    free_text(g_sd_storage.space_text);
}

void free_sound_effects(void)
{
    free_sound(g_sound_effects.error);
    free_sound(g_sound_effects.insert);
    free_sound(g_sound_effects.move);
    free_sound(g_sound_effects.popup);
}

void free_tetris(void)
{
    for (uint16_t i = 0, button = Font_Tetris_B; button <= Font_Tetris_L; i++, button++)
    {
        free_button(g_tetris_button[i]);
    }
}

bool init_menu(void)
{
    // delete all exiting placeholders.
    ncm_delete_all_placeholders();

    // romfs will contain the empty icon and sound effects.
    if (R_FAILED(romfsInit()))
        return false;

    // load background.
    setup_background();

    // setup menu options.
    setup_options();

    // load sounds.
    setup_sound_effects();

    // setup the storage.
    setup_storage();

    // create the spinning tetris.
    setup_teris();

    // load icon from romfs.
    g_empty_icon = create_image_from_file(EMPTY_ICON_PATH, 90, 130, 0, 0);

    // unmount romfs as we only needed the empty icon picture and sound effects.
    romfsExit();
    return true;
}

void exit_menu(void)
{
    free_image(g_empty_icon);
    free_background();
    free_options();
    free_storage();
    free_sound_effects();
    free_tetris();
    unmount_gc(&gamecard);
}



/*
*   Update stuff.
*/

void update_button_spin(void)
{
    g_tetris_spin_pos = g_tetris_spin_pos == 7 ? 0 : g_tetris_spin_pos + 1;
}

void update_options(void)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (i == g_cursor)
        {
            g_options[i].selected = true;
            SDL_SetTextureColour(g_options[i].text->tex,  g_cursor == Option_Exit || g_gc_inserted ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver);
        }
        else
        {
            g_options[i].selected = false;
            SDL_SetTextureColour(g_options[i].text->tex, i == Option_Exit || g_gc_inserted ? Colour_Nintendo_White : Colour_Nintendo_Silver);
        }
        g_options[i].avaliable = g_gc_inserted;
    }
}

void update_storage_size(void)
{
    // get new free space.
    g_nand_storage.free = ns_get_storage_free_space(NcmStorageId_BuiltInUser);
    g_nand_storage.total = ns_get_storage_total_size(NcmStorageId_BuiltInUser);
    g_sd_storage.free = ns_get_storage_free_space(NcmStorageId_SdCard);
    g_sd_storage.total = ns_get_storage_total_size(NcmStorageId_SdCard);
    
    // update shapes.
    set_shape(&g_nand_storage.filled, Colour_Nintendo_White, 480 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_nand_storage.free / (float)g_nand_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);
    set_shape(&g_sd_storage.filled, Colour_Nintendo_White, 860 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_sd_storage.free / (float)g_sd_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);

    // calculate new sizes / 1GiB.
    g_nand_storage.size_gb = (float)g_nand_storage.free / 0x40000000;
    g_sd_storage.size_gb = (float)g_sd_storage.free / 0x40000000;

    // update text.
    update_text(g_nand_storage.space_text, "System memory %.1fGB", g_nand_storage.size_gb);
    update_text(g_sd_storage.space_text, "microSD card %.1fGB", g_sd_storage.size_gb);
}

void update_gamecard(void)
{
    if (poll_gc() != g_gc_inserted)
    {
        update_button_spin();
        play_sound(g_sound_effects.insert, -1, 0);
        bool ret = g_gc_inserted ? unmount_gc(&gamecard) : mount_gc(&gamecard);

        // mounting the gamecard can fail, only set flag to true if the gamecard is mounted.
        if (ret)
            g_gc_inserted = !g_gc_inserted;
    }
}

void update_pulse_colour(pulse_colour_t *pulse)
{
    if (pulse->col.g == 255) pulse->increase_blue = true;
    else if (pulse->col.b == 255 && pulse->delay == 10)
    {
        pulse->increase_blue = false;
        pulse->delay = 0;
    }

    if (pulse->col.b == 255 && pulse->increase_blue == true)
    {
        pulse->delay++;
    }
    else
    {
        pulse->col.b = pulse->increase_blue ? pulse->col.b + 2 : pulse->col.b - 2;
        pulse->col.g = pulse->increase_blue ? pulse->col.g - 2 : pulse->col.g + 2;
    }
}



/*
*   Ui Display stuff.
*/

void ui_display_background(void)
{
    // background colour.
    draw_shape(&g_background.background);
    draw_shape(&g_background.side_background);
    draw_shape(&g_background.upper_bar);
    draw_shape(&g_background.lower_bar);

    // title and icon.
    draw_text(g_background.title);
    draw_button(g_background.app_icon);

    // buttons
    draw_button(g_background.a_button);
    draw_button(g_background.b_button);
    draw_text(g_background.a_text);
    draw_text(g_background.b_text);
}

void ui_display_storage_size(void)
{
    draw_shape(&g_nand_storage.bar);
    draw_shape(&g_nand_storage.filled);
    draw_text(g_nand_storage.space_text);
    draw_shape(&g_sd_storage.bar);
    draw_shape(&g_sd_storage.filled);
    draw_text(g_sd_storage.space_text);
    
}

pulsing_shape_t g_options_pulse_bar = { { {0, 255, 187, 255} , false, 0 }, { 0 } };
void ui_display_option_list(void)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (g_options[i].selected)
        {
            draw_shape_position(&g_background.selected_bar, 485, g_options[i].text->rect.y - 10);
            SDL_DrawShapeOutlineEX(g_options_pulse_bar.pulse.col.r, g_options_pulse_bar.pulse.col.g, g_options_pulse_bar.pulse.col.b, g_options_pulse_bar.pulse.col.a, 475, g_options[i].text->rect.y - 25, 720, 70, 5);
        }
        draw_text(g_options[i].text);
    }
}

void ui_display_gamecard(void)
{
    if (!g_gc_inserted)
    {
        draw_image2(g_empty_icon);
    }
    else
    {
        draw_image2(gamecard.icon);
        draw_text(gamecard.title);
        draw_text(gamecard.author);
        draw_text(gamecard.text_size);
        draw_text(gamecard.text_key_gen);
        draw_text(gamecard.text_app_id);   
    }
}

void ui_display_button_spin(void)
{
    draw_button(g_tetris_button[g_tetris_spin_pos]);
}

void ui_display_dim_background(void)
{
    // clear the renderer.
    SDL_ClearRenderer();

    // re draw the main menu.
    render_menu();

    // draw an opaque shape over the entire screen.
    SDL_SetRenderDrawColor(SDL_GetRenderer(SDL_GetWindow()), 20, 20, 20, 225);
    SDL_Rect rect = { 0, 0, SCREEN_W, SCREEN_H };
    SDL_RenderFillRect(SDL_GetRenderer(SDL_GetWindow()), &rect);
}

void ui_display_popup_box(void)
{
    ui_display_dim_background();

    // The pop up shape.
    SDL_Rect box = { 255, 145, 770, 430 };
    SDL_DrawShape(Colour_Nintendo_DarkGrey, box.x, box.y, box.w, box.h, true);
}

bool ui_display_yes_no_box(const char *message)
{
    uint8_t cursor = 1;
    bool flag = false;
    pulsing_shape_t pulse_shape = { { {0, 255, 187, 255} , false, 0 }, { 0 } };

    while (appletMainLoop())
    {
        input_t input = get_input();
        if (input.down & KEY_RIGHT)
            cursor = 1;
        if (input.down & KEY_LEFT)
            cursor = 0;
        
        int ret = check_if_touch_yesno(&input);
        if (ret != -1)
        {
            cursor = ret;
            input.down |= KEY_A;
        }

        if (input.down & KEY_A)
        {
            flag = cursor;
            break;
        }

        if (input.down & KEY_B)
        {
            flag = false;
            break;
        }

        ui_display_dim_background();

        // The pop up shape.
        SDL_Rect box = { 255, 210, 770, 295 };
        SDL_DrawShape(Colour_Nintendo_DarkGrey, box.x, box.y, box.w, box.h, true);

        // display the message.
        SDL_DrawTextCenterX(FONT_TEXT[QFontSize_25].fnt, box.y + 110, box.x, box.w, Colour_Nintendo_White, message);

        // 
        SDL_DrawShape(Colour_Nintendo_LightSilver, box.x, (box.y + box.h) - 72, 770, 2, true);
        SDL_DrawShapeOutlineEX(pulse_shape.pulse.col.r, pulse_shape.pulse.col.g, pulse_shape.pulse.col.b, pulse_shape.pulse.col.a, cursor ? box.x + 380 : 250, box.y + 220, 390, 75, 5);
        SDL_DrawTextCenterX(FONT_TEXT[QFontSize_25].fnt, (box.y + box.h) - 50, box.x + (box.w / 2), box.w / 2, Colour_Nintendo_Cyan, "OK");
        SDL_DrawTextCenterX(FONT_TEXT[QFontSize_25].fnt, (box.y + box.h) - 50, box.x, box.w / 2, Colour_Nintendo_Cyan, "Back");

        update_pulse_colour(&pulse_shape.pulse);
        SDL_UpdateRenderer();
    }
    return flag;
}

void ui_display_error_box(uint32_t err)
{
    // play the error sound effect.
    play_sound(g_sound_effects.error, -1, 0);
    
    // display the popup box.
    ui_display_popup_box();

    SDL_Rect box = { 455, 470, 365, 65 };
    SDL_DrawButton(FONT_BUTTON[QFontSize_63].fnt, 0xE140, 608, 190, Colour_Nintendo_Red);
    SDL_DrawText(FONT_TEXT[QFontSize_25].fnt, 520, 290, Colour_Nintendo_White, "Error code: 0x%04X", err);
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 340, 360, Colour_Nintendo_BrightSilver, "If this message appears repeatedly, please open an issue.");
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 320, 405, Colour_Nintendo_BrightSilver, "https://github.com/ITotalJustice/Gamecard-Installer-NX/issues");
    SDL_DrawShapeOutline(Colour_Nintendo_Cyan, box.x, box.y, box.w, box.h, 5);
    SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, 620, box.y + 25, Colour_Nintendo_White, "OK");

    // update the screen.
    SDL_UpdateRenderer();

    // delay for 1 second so that users will see the error, even if they're mashing buttons.
    SDL_Delay(1000);

    // loop until the user selects okay!
    while (appletMainLoop())
    {
        if (check_if_touch_error())
            break;
    }
}

progress_bar_t *ui_init_progress_bar(const char *name, uint64_t speed, uint16_t eta_min, uint8_t eta_sec, size_t done, size_t remaining)
{
    progress_bar_t *p_bar = calloc(1, sizeof(progress_bar_t));

    // shapes.
    SDL_Rect prog_bar = { 400, 470, 480, 12 };
    p_bar->empty_bar = create_shape(Colour_Nintendo_LightSilver, prog_bar.x, prog_bar.y, prog_bar.w, prog_bar.h, true);
    p_bar->filled_bar = create_shape(Colour_Nintendo_Cyan, prog_bar.x, prog_bar.y, ((float)done / (float)remaining) * prog_bar.w, prog_bar.h, true);

    // text.
    p_bar->text_header = create_text(&FONT_TEXT[QFontSize_25], 575, 200, Colour_Nintendo_White, "Installing...");
    p_bar->text_warning1 = create_text(&FONT_TEXT[QFontSize_20], 500, 260, Colour_Nintendo_BrightSilver, "Please do not remove the gamecard or");
    p_bar->text_warning2 = create_text(&FONT_TEXT[QFontSize_20], 500, 295, Colour_Nintendo_BrightSilver, "power off the system whilst installing.");
    p_bar->text_speed = create_text(&FONT_TEXT[QFontSize_20], 325, 360, Colour_Nintendo_White, "%.2f MiB/s", speed);
    p_bar->text_name = create_text(&FONT_TEXT[QFontSize_20], 410, 420, Colour_Nintendo_White, "%s.nca", name);
    p_bar->text_time = create_text(&FONT_TEXT[QFontSize_20], prog_bar.x + 85, prog_bar.y + 40, Colour_Nintendo_White, "%d %s %d seconds remaining", eta_min, eta_min != 1 ? "minutes" : "minute", eta_sec);

    // vars.
    p_bar->speed = speed;
    p_bar->eta_min = eta_min;
    p_bar->eta_sec = eta_sec;

    return p_bar;
}

void ui_free_progress_bar(progress_bar_t *p_bar)
{
    free_text(p_bar->text_header);
    free_text(p_bar->text_warning1);
    free_text(p_bar->text_warning2);
    free_text(p_bar->text_name);
    free_text(p_bar->text_speed);
    free_text(p_bar->text_time);
    free(p_bar);
    p_bar = NULL;
}

void ui_update_progress_bar(progress_bar_t *p_bar, uint64_t speed, uint16_t eta_min, uint8_t eta_sec, size_t done, size_t remaining)
{
    // shapes.
    p_bar->filled_bar.info.rect.w = ((float)done / (float)remaining) * p_bar->empty_bar.info.rect.w;

    // text.
    if (p_bar->speed != speed)
        update_text(p_bar->text_speed, "%.2f MiB/s", (float)speed / 0x100000);
    if (p_bar->eta_sec != eta_sec || p_bar->eta_min != eta_min)
        update_text(p_bar->text_time, "%d %s %d seconds remaining", eta_min, eta_min != 1 ? "minutes" : "minute", eta_sec);

    // vars.
    p_bar->speed = speed;
    p_bar->eta_min = eta_min;
    p_bar->eta_sec = eta_sec;
}

void ui_display_progress_bar(progress_bar_t *p_bar)
{
    // display the popup box.
    ui_display_popup_box();

    // Title and warning text.
    draw_text(p_bar->text_header);
    draw_text(p_bar->text_warning1);
    draw_text(p_bar->text_warning2);

    // game icon and speed.
    draw_image_set(gamecard.icon, 320, 200, gamecard.icon->rect.w / 2, gamecard.icon->rect.h / 2);
    draw_text(p_bar->text_speed);
    draw_text(p_bar->text_name);
    draw_text(p_bar->text_time);

    // nca name and progress bar.
    draw_shape(&p_bar->empty_bar);
    draw_shape(&p_bar->filled_bar);
    
    // now update the renderer with the progress bar.
    SDL_UpdateRenderer();
}

void render_menu(void)
{
    ui_display_background();
    ui_display_storage_size();
    ui_display_option_list();
    ui_display_gamecard();
    ui_display_button_spin();
}



/*
*   Basic input handling.
*/
uint8_t handle_input(void)
{
    input_t input = get_input();

    if (input.down)
        update_button_spin();

    if (input.down & KEY_DOWN)
    {
        g_cursor = move_cursor_down(g_cursor, 3);
        play_sound(g_sound_effects.move, -1, 0);
    }

    if (input.down & KEY_UP)
    {
        g_cursor = move_cursor_up(g_cursor, 3);
        play_sound(g_sound_effects.move, -1, 0);
    }

    if (input.down & KEY_L || input.down & KEY_R)
    {
        swap_game_in_gc(&gamecard);
    }

    if (input.down & KEY_B)
    {
        if (ui_display_yes_no_box("Would you like to exit?"))
            return Option_Exit;  
    }

    int ret = check_if_option(&input);
    if (ret != -1)
    {
        g_cursor = ret;
        input.down |= KEY_A;
    }

    if (input.down & KEY_A)
    {
        play_sound(g_sound_effects.popup, -1, 0);
        switch (g_cursor)
        {
            case Option_Nand:
                if (g_gc_inserted)
                {
                    if (ui_display_yes_no_box("Install to the Nand?"))
                    {
                        if (!install_gc(&gamecard, NcmStorageId_BuiltInUser))
                            ncm_delete_all_placeholders();
                        update_storage_size();
                    }
                }
                break;
            case Option_SD:
                if (g_gc_inserted)
                {
                    if (ui_display_yes_no_box("Install to the Sd Card?"))
                    {
                        if (!install_gc(&gamecard, NcmStorageId_SdCard))
                            ncm_delete_all_placeholders();
                        update_storage_size();
                    }
                }
                break;
            case Option_Exit:
                if (ui_display_yes_no_box("Would you like to exit?"))
                    return Option_Exit;
        }
    }
    return 0;
}



/*
*   Main.
*/

void start_menu(void)
{
    while (appletMainLoop())
    {
        // get input.
        if (handle_input() == Option_Exit)
            break;

        // update stuff.
        update_gamecard();
        update_pulse_colour(&g_options_pulse_bar.pulse);
        update_options();

        // now render the menu.
        SDL_ClearRenderer();
        render_menu();
        SDL_UpdateRenderer();
    }
}
