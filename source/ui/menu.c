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


// the gamecard struct that will contain info on the gc, as well as gfx data.
gamecard_t gamecard = {0};

// the default icon that is rendered if no gamecard is inserted.
image_t *g_empty_icon = {0};

// background shapes.
shape_t g_background = {0};
shape_t g_side_background = {0};
shape_t g_upper_bar = {0};
shape_t g_lower_bar = {0};
text_t *g_a_text = {0};
text_t *g_b_text = {0};
button_t *g_a_button = {0};
button_t *g_b_button = {0};
text_t *g_title = {0};
button_t *g_app_icon = {0};
shape_t g_selected_bar = {0};

// sound effects.
sound_t *error = {0};
sound_t *insert = {0};
sound_t *move = {0};
sound_t *popup = {0};

// global cursor for the main menu.
uint8_t g_cursor = 0;
uint8_t g_button_spin_colour = 0;
uint16_t g_button_spin = Font_Tetris_B;

typedef struct
{
    size_t free;
    size_t total;
    shape_t bar;
    shape_t filled;
    float size_gb;
} storage_t;

storage_t g_nand_storage = {0};
storage_t g_sd_storage = {0};

// global flag for when the gamecard is inserted, changes render options.
bool g_gc_inserted = false;



/*
*   Init and exit.
*/

bool init_menu(void)
{
    // romfs will contain the empty icon and sound effects.
    if (R_FAILED(romfsInit()))
        return false;

    // load background.
    g_background = create_shape(Colour_Nintendo_Black, 0, 0, SCREEN_W, SCREEN_H, true);
    g_side_background = create_shape(Colour_Nintendo_LightBlack, 30, 90, 375, 555, true);
    g_upper_bar = create_shape(Colour_Nintendo_White, 30, (SCREEN_H / 8), SCREEN_W - (30 * 2), 2, true);
    g_lower_bar = create_shape(Colour_Nintendo_White, 30, 650, SCREEN_W - (30 * 2), 2, true);
    g_selected_bar = create_shape(Colour_Nintendo_Cyan, 90, 230, 4, 45, true);

    // load sounds.
    error = load_sound_from_file(g_sound_list[0]);
    insert = load_sound_from_file(g_sound_list[1]);
    move = load_sound_from_file(g_sound_list[2]);
    popup = load_sound_from_file(g_sound_list[3]);

    // setup the buttons.
    g_a_button = create_button(&FONT_BUTTON[QFontSize_25], 1150, 675, Colour_Nintendo_White, Font_Button_A);
    g_b_button = create_button(&FONT_BUTTON[QFontSize_25], 1055 - 35, 675, Colour_Nintendo_White, Font_Button_B);
    g_a_text = create_text(&FONT_TEXT[QFontSize_20], 1185, 675, Colour_Nintendo_White, "OK");
    g_b_text = create_text(&FONT_TEXT[QFontSize_20], 1055, 675, Colour_Nintendo_White, "Back");

    // setup the storage bars.
    g_nand_storage.free = ns_get_storage_free_space(NcmStorageId_BuiltInUser);
    g_nand_storage.total = ns_get_storage_total_size(NcmStorageId_BuiltInUser);
    g_sd_storage.free = ns_get_storage_free_space(NcmStorageId_SdCard);
    g_sd_storage.total = ns_get_storage_total_size(NcmStorageId_SdCard);
    g_nand_storage.bar = create_shape(Colour_Nintendo_White, 480, 170, STORAGE_BAR_W, 14, false);
    g_nand_storage.filled = create_shape(Colour_Nintendo_White, 480 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_nand_storage.free / (float)g_nand_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);
    g_sd_storage.bar = create_shape(Colour_White, 860, 170, STORAGE_BAR_W, 14, false);
    g_sd_storage.filled = create_shape(Colour_Nintendo_White, 860 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_sd_storage.free / (float)g_sd_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);
    g_nand_storage.size_gb = (float)g_nand_storage.free / 0x40000000;
    g_sd_storage.size_gb = (float)g_sd_storage.free / 0x40000000;

    // load icon from romfs.
    g_empty_icon = create_image_from_file(EMPTY_ICON_PATH, 90, 130, 0, 0);
    if (!g_empty_icon)
        return false;

    // load title text.
    g_title = create_text(&FONT_TEXT[QFontSize_25], 130, 40, Colour_Nintendo_White, TITLE_NAME);
    if (!g_title)
        return false;

    g_app_icon = create_button(&FONT_BUTTON[QFontSize_45], 65, 30, Colour_Nintendo_White, Font_Button_SETTINGS);

    // unmount romfs as we only needed the empty icon picture and sound effects.
    romfsExit();
    return true;
}

void exit_menu(void)
{
    free_image(g_empty_icon);
    free_text(g_title);
    free_button(g_app_icon);
    free_button(g_a_button);
    free_button(g_b_button);
    free_text(g_a_text);
    free_text(g_b_text);
    free_sound(error);
    free_sound(insert);
    free_sound(move);
    free_sound(popup);
}



/*
*   Update stuff.
*/

void update_button_spin(void)
{
    g_button_spin = g_button_spin == Font_Tetris_L ? Font_Tetris_TL : g_button_spin + 1;
    g_button_spin_colour = g_button_spin_colour == COLOUR_MAX ? 0 : g_button_spin_colour + 1;
}

void update_storage_size(void)
{
    g_nand_storage.free = ns_get_storage_free_space(NcmStorageId_BuiltInUser);
    g_nand_storage.total = ns_get_storage_total_size(NcmStorageId_BuiltInUser);
    g_sd_storage.free = ns_get_storage_free_space(NcmStorageId_SdCard);
    g_sd_storage.total = ns_get_storage_total_size(NcmStorageId_SdCard);
    
    set_shape(&g_nand_storage.filled, Colour_Nintendo_White, 480 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_nand_storage.free / (float)g_nand_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);
    set_shape(&g_sd_storage.filled, Colour_Nintendo_White, 860 + 2, 170 + 2, STORAGE_BAR_W - (((float)g_sd_storage.free / (float)g_sd_storage.total) * STORAGE_BAR_W) - 4, 14 - 4, true);

    g_nand_storage.size_gb = (float)g_nand_storage.free / 0x40000000;
    g_sd_storage.size_gb = (float)g_sd_storage.free / 0x40000000;
}

void update_gamecard(void)
{
    if (poll_gc() != g_gc_inserted)
    {
        update_button_spin();
        play_sound(insert, -1, 0);
        bool ret = g_gc_inserted ? unmount_gc(&gamecard) : mount_gc(&gamecard);

        // mounting the gamecard can fail, only set flag to true if the gamecard is mounted.
        if (ret)
            g_gc_inserted = !g_gc_inserted;
    }
}


/*
*   Ui Display stuff.
*/

void ui_display_background(void)
{
    // background colour.
    draw_shape(&g_background);
    draw_shape(&g_side_background);
    draw_shape(&g_upper_bar);
    draw_shape(&g_lower_bar);

    // title and icon.
    draw_text(g_title);
    draw_button(g_app_icon);

    // buttons
    draw_button(g_a_button);
    draw_button(g_b_button);
    draw_text(g_a_text);
    draw_text(g_b_text);
}

void ui_display_storage_size(void)
{
    draw_shape(&g_nand_storage.bar);
    draw_shape(&g_nand_storage.filled);
    SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, 490, 135, Colour_Nintendo_White, "System memory %.1fGB", g_nand_storage.size_gb);
    draw_shape(&g_sd_storage.bar);
    draw_shape(&g_sd_storage.filled);
    SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, 870, 135, Colour_Nintendo_White, "microSD card %.1fGB", g_sd_storage.size_gb);
}

void ui_display_option_list(void)
{
    for (uint16_t i = 0, y = 300; i < 3; i++, y += 125)
    {
        if (i == g_cursor)
        {
            draw_shape_position(&g_selected_bar, 485, y - 10);
            SDL_DrawShapeOutline(Colour_Nintendo_Teal, 475, y - 25, 720, 70, 5);
            SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, 500, y, g_cursor == Option_Exit || g_gc_inserted ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver, g_option_list[i]);
        }
        else
        {
            SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, 500, y,  i == Option_Exit || g_gc_inserted ? Colour_Nintendo_White : Colour_Nintendo_Silver, g_option_list[i]);
        }
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
    SDL_DrawButton(FONT_BUTTON[QFontSize_33].fnt, g_button_spin, 50, SCREEN_H - 50, Colour_Nintendo_White);
}

void ui_display_popup_box(void)
{
    // clear the renderer.
    SDL_ClearRenderer();

    // re draw the main menu.
    render_menu();

    // draw an opaque shape over the entire screen.
    SDL_SetRenderDrawBlendMode(SDL_GetRenderer(SDL_GetWindow()), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(SDL_GetRenderer(SDL_GetWindow()), 20, 20, 20, 225);
    SDL_Rect rect = { 0, 0, SCREEN_W, SCREEN_H };
    SDL_RenderFillRect(SDL_GetRenderer(SDL_GetWindow()), &rect);
    SDL_SetRenderDrawBlendMode(SDL_GetRenderer(SDL_GetWindow()), SDL_BLENDMODE_NONE);

    // The pop up shape.
    SDL_Rect box = { 255, 145, 770, 430 };
    SDL_DrawShape(Colour_Nintendo_DarkGrey, box.x, box.y, box.w, box.h, true);
}

void ui_display_yes_no_box(const char *message)
{
    ui_display_popup_box();

    // TODO.

    SDL_UpdateRenderer();
}

void ui_display_error_box(uint32_t err)
{
    // display the popup box.
    ui_display_popup_box();

    // play the error sound effect.
    play_sound(error, -1, 0);

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

void ui_display_progress_bar(const char *name, int speed, int eta_min, int eta_sec, size_t done, size_t remaining)
{
    // display the popup box.
    ui_display_popup_box();

    // Title and warning text.
    SDL_DrawText(FONT_TEXT[QFontSize_25].fnt, 575, 200, Colour_Nintendo_White, "Installing...");
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 500, 260, Colour_Nintendo_BrightSilver, "Please do not remove the gamecard or");
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 500, 295, Colour_Nintendo_BrightSilver, "power off the system whilst installing.");

    // game icon and speed.
    draw_image_set(gamecard.icon, 320, 200, gamecard.icon->rect.w / 2, gamecard.icon->rect.h / 2);
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 325, 360, Colour_Nintendo_White, "%.2f MiB/s", (float)speed / 0x100000);

    // nca name and progress bar.
    SDL_Rect bar = { 400, 470, 480, 12 };
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 410, 420, Colour_Nintendo_White, "%s.nca", name);
    SDL_DrawShape(Colour_Nintendo_LightSilver, bar.x, bar.y, bar.w, bar.h, true);
    SDL_DrawShape(Colour_Nintendo_Cyan, bar.x, bar.y, ((float)done / (float)remaining) * bar.w, bar.h, true);
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, bar.x + 85, bar.y + 40, Colour_Nintendo_White, "%d %s %d seconds remaining", eta_min, eta_min != 1 ? "minutes" : "minute", eta_sec);

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
        play_sound(move, -1, 0);
    }

    if (input.down & KEY_UP)
    {
        g_cursor = move_cursor_up(g_cursor, 3);
        play_sound(move, -1, 0);
    }

    if (input.down & KEY_L || input.down & KEY_R)
    {
        swap_game_in_gc(&gamecard);
    }

    if (input.down & KEY_B)
        return Option_Exit;
        
    if (input.t_count)
    {
        int ret = check_if_option(&input);
        if (ret != -1)
        {
            g_cursor = ret;
            input.down |= KEY_A;
        }
    }

    if (input.down & KEY_A)
    {
        play_sound(popup, -1, 0);
        switch (g_cursor)
        {
            case Option_Nand:
                if (!install_gc(&gamecard, NcmStorageId_BuiltInUser))
                    ncm_delete_all_placeholders();
                update_storage_size();
                break;
            case Option_SD:
                if (!install_gc(&gamecard, NcmStorageId_SdCard))
                    ncm_delete_all_placeholders();
                update_storage_size();
                    break;
            case Option_Exit:
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
    // delete all exiting placeholders.
    // this ensures that no temp files are left over, should the app ever crash.
    ncm_delete_all_placeholders();

    // muh loop.
    while (appletMainLoop())
    {
        // get input.
        if (handle_input() == Option_Exit)
            break;

        // check for gc
        update_gamecard();

        // now render the menu.
        SDL_ClearRenderer();
        render_menu();
        SDL_UpdateRenderer();
    }

    unmount_gc(&gamecard);
}