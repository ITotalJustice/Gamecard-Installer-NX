#include <stdint.h>
#include <stdbool.h>
#include <switch.h>

#include "ui/menu.h"
#include "ui/gc.h"
#include "ui/settings.h"

#include "gfx/SDL_easy.h"
#include "gfx/image.h"
#include "gfx/shape.h"
#include "gfx/text.h"
#include "gfx/button.h"

#include "sound/sound.h"

#include "nx/ns.h"
#include "nx/ncm.h"
#include "nx/input.h"
#include "nx/lbl.h"

#include "util/log.h"

#ifdef DEBUG
#define TITLE_NAME      "GameCard Installer (DEBUG)"
#else
#define TITLE_NAME      "GameCard Installer"
#endif
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
    "SD Card Install",
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
    text_t *x_text;
    text_t *y_text;
    text_t *l_text;
    text_t *r_text;
    button_t *a_button;
    button_t *b_button;
    button_t *x_button;
    button_t *y_button;
    button_t *l_button;
    button_t *r_button;
    button_t *app_icon;
} Background_t;
Background_t g_background = {0};

typedef struct
{
    text_t *text;
    bool selected;
    bool avaliable;
} Options_t;
Options_t g_options[3] = {0};

// sound effects.
typedef struct
{
    sound_t *error;
    sound_t *insert;
    sound_t *move;
    sound_t *popup;
} SoundEffects_t;
SoundEffects_t g_sound_effects = {0};

typedef struct
{
    size_t free;
    size_t total;
    float size_gb;
    shape_t bar;
    shape_t filled;
    text_t *space_text;
} Storage_t;

Storage_t g_nand_storage = {0};
Storage_t g_sd_storage = {0};

// the gamecard struct that will contain info on the gc, as well as gfx data.
GameInfo_t g_game_info = {0};

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
    g_background.x_button = create_button(&FONT_BUTTON[QFontSize_25], 870, 675, Colour_Nintendo_White, Font_Button_X);
    g_background.y_button = create_button(&FONT_BUTTON[QFontSize_25], 700, 675, Colour_Nintendo_White, Font_Button_Y);
    g_background.l_button = create_button(&FONT_BUTTON[QFontSize_25], 90 - 40, 130 + (255/2), Colour_Nintendo_White, Font_Button_L);
    g_background.r_button = create_button(&FONT_BUTTON[QFontSize_25], 90 + 255 + 15, 130 + (255/2), Colour_Nintendo_White, Font_Button_R);

    // button text.        
    g_background.a_text = create_text(&FONT_TEXT[QFontSize_20], 1185, 675, Colour_Nintendo_White, "OK");
    g_background.b_text = create_text(&FONT_TEXT[QFontSize_20], 1055, 675, Colour_Nintendo_White, "Back");
    g_background.x_text = create_text(&FONT_TEXT[QFontSize_20], 905, 675, Colour_Nintendo_White, "Settings");
    g_background.y_text = create_text(&FONT_TEXT[QFontSize_20], 735, 675, Colour_Nintendo_White, "Game Info");
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
    free_button(g_background.x_button);
    free_button(g_background.y_button);
    free_button(g_background.l_button);
    free_button(g_background.r_button);
    free_text(g_background.a_text);
    free_text(g_background.b_text);
    free_text(g_background.x_text);
    free_text(g_background.y_text);
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

void free_game_info(GameInfo_t *game_info)
{
    if (!game_info)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }

    free_image(game_info->icon);
    free_text(game_info->title);
    free_text(game_info->author);
    free_text(game_info->text_size);
    free_text(game_info->text_key_gen);
    free_text(game_info->text_app_id);
    free_text(game_info->text_entry_contents);
    memset(game_info, 0, sizeof(GameInfo_t));
}

void free_game_info_detailed(GameInfoDetailed_t *info)
{
    if (!info)
    {
        write_log("missing params in %s\n", __func__);
        return;
    }

    free_text(info->text_type);
    free_text(info->text_id);
    free_text(info->text_keygen);
    free_text(info->text_version);
    free_text(info->text_content_count);
    free_text(info->text_content_meta_count);

    for (uint16_t i = 0; i < info->content_count; i++)
    {
        free_text(info->entry[i].name);
        free_text(info->entry[i].type);
        free_text(info->entry[i].size);
    }

    info->id = 0;
    info->type = 0;
    info->keygen = 0;
    info->version = 0;
    info->content_count = 0;
    info->content_meta_count = 0;

    memset(info, 0, sizeof(GameInfoDetailed_t));
}

bool init_menu(void)
{
    //ncm_delete_all_placeholders_id();

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
    if (gc_poll() != g_gc_inserted)
    {
        update_button_spin();
        play_sound(g_sound_effects.insert, -1, 0);

        if (g_gc_inserted)
        {
            g_gc_inserted = false;
            gc_unmount();
            free_game_info(&g_game_info);
        }
        else
        {
            if (gc_mount())
            {
                gc_setup_game_info(&g_game_info, 0);
                g_gc_inserted = true;
            }
        }
    }
}

void update_pulse_colour(PulseColour_t *pulse)
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
    draw_button(g_background.x_button);
    draw_button(g_background.y_button);
    draw_text(g_background.a_text);
    draw_text(g_background.b_text);
    draw_text(g_background.x_text);
    draw_text(g_background.y_text);
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

PulseShape_t g_options_pulse_bar = { { {0, 255, 187, 255} , false, 0 }, { 0 } };
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
        // if theres more than one game on the gamecard, show the user how to swap.
        if (gc_get_game_count() > 1)
        {
            draw_button(g_background.l_button);
            draw_button(g_background.r_button);
        }
        if (g_game_info.icon)
        {
            draw_image2(g_game_info.icon);
        }
        else
        {
            draw_image2(g_empty_icon);
        }
        draw_text(g_game_info.title);
        draw_text(g_game_info.author);
        draw_text(g_game_info.text_size);
        draw_text(g_game_info.text_key_gen);
        draw_text(g_game_info.text_app_id);   
        draw_text(g_game_info.text_entry_contents);
    }
}

void ui_display_detailed_gamecard(void)
{
    // initial info setup.
    uint16_t cursor = 0;
    GameInfoDetailed_t info = {0};
    if (!gc_setup_detailed_game_info(&info, cursor))
    {
        return;
    }

    // the size of the pop-up box.
    const SDL_Rect box = { 255 / 1.5, 145 / 1.5, SCREEN_W - (box.x  * 2), SCREEN_H - (box.y * 2)};

    button_t *l_button = create_button(&FONT_BUTTON[QFontSize_23], 845, box.y + 480, Colour_Nintendo_White, Font_Button_L);
    button_t *r_button = create_button(&FONT_BUTTON[QFontSize_23], 875, box.y + 480, Colour_Nintendo_White, Font_Button_R);
    text_t *swap_text = create_text(&FONT_TEXT[QFontSize_23], 910, box.y + 480, Colour_Nintendo_White, "Swap Entry");
    text_t *title = create_text(&FONT_TEXT[QFontSize_28], box.x + 60, box.y + 30, Colour_Nintendo_White, "Game-Info");

    while (appletMainLoop())
    {
        input_t input = get_input();

        if (input.down)
        {
            play_sound(g_sound_effects.move, -1, 0);
        }

        if (input.down & KEY_B)
        {
            break;
        }

        if (input.down & KEY_L)
        {
            // ensure that we have at least one entry (we always should) then check if we have more than one.
            if (g_game_info.total_count && g_game_info.total_count -1)
            {
                cursor = move_cursor_up(cursor, g_game_info.total_count);
                free_game_info_detailed(&info);
                if (!gc_setup_detailed_game_info(&info, cursor))
                {
                    break;
                }
            }
        }
        if (input.down & KEY_R)
        {
            // same as above.
            if (g_game_info.total_count && g_game_info.total_count -1)
            {
                cursor = move_cursor_down(cursor, g_game_info.total_count);
                free_game_info_detailed(&info);
                if (!gc_setup_detailed_game_info(&info, cursor))
                {
                    break;
                }
            }
        }

        ui_display_dim_background();
        SDL_DrawShape(Colour_Nintendo_DarkGrey, box.x, box.y, box.w, box.h, true);
        SDL_DrawShape(Colour_Nintendo_LightBlack, box.x + 375, box.y + 72, box.w - 375, box.h - 72 - 75, true);

        // title / boarder / bottom.
        draw_text(title);
        SDL_DrawShape(Colour_Nintendo_White, box.x + 25, box.y + 75, box.w - 50, 2, true);
        SDL_DrawShape(Colour_Nintendo_White, box.x + 25, box.y + box.h - 75, box.w - 50, 2, true);
        SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 60, box.y + 480, Colour_Nintendo_White, "Page: %u / %u", cursor + 1, g_game_info.total_count);

        draw_button(l_button);
        draw_button(r_button);
        draw_text(swap_text);

        if (g_game_info.icon)
        {
            draw_image_set(g_game_info.icon, 290, 200, g_game_info.icon->rect.w / 2, g_game_info.icon->rect.h / 2);
        }
        else
        {
            draw_image_set(g_empty_icon, 290, 200, g_empty_icon->rect.w / 2, g_empty_icon->rect.h / 2);
        }
        draw_text(info.text_type);
        draw_text(info.text_id);
        draw_text(info.text_keygen);
        draw_text(info.text_version);
        draw_text(info.text_content_count);
        draw_text(info.text_content_meta_count);

        for (uint16_t i = 0; i < info.content_count; i++)
        {
            draw_text(info.entry[i].name);
            draw_text(info.entry[i].type);
            draw_text(info.entry[i].size);
        }

        SDL_UpdateRenderer();
    }

    free_button(l_button);
    free_button(r_button);
    free_text(swap_text);
    free_text(title);
    free_game_info_detailed(&info);
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

void ui_draw_spacers(int x, int y, int txt_size)
{
    SDL_DrawShape(Colour_Nintendo_Silver, x - 25, y-18, 500 + 50, 1, true);
    SDL_DrawShape(Colour_Nintendo_Silver, x - 25, y+txt_size+18, 500 + 50, 1, true);
}

void ui_draw_highlight_box(PulseShape_t *pulse_shape, int x, int y, int w, int h)
{
    SDL_DrawShapeOutlineEX(pulse_shape->pulse.col.r, pulse_shape->pulse.col.g, pulse_shape->pulse.col.b, pulse_shape->pulse.col.a, x - 20, y - 20, w, h + 20, 5);
    update_pulse_colour(&pulse_shape->pulse);
}

// this code should be made illegal.
// this is what happens when you want to bang out a ui menu in 1 hour.
// TODO: burn the code.
// ACTUAL TODO: write a usable ui framework.
// - one that doesn't leak mem and that actually caches stuff.
void ui_display_options(void)
{
    PulseShape_t pulse_shape = { { {0, 255, 187, 255} , false, 0 }, { 0 } };
    
    const char *title_base = "Install Base";
    const char *title_update = "Install Update";
    const char *title_dlc = "Install DLC";
    const char *title_music = "Enable Music";
    const char *title_sound = "Sound Effects";

    const char *hdr[] = 
    {
        "Installation",
        "Sounds",
        "Exit",
    };

    const char *expl_keygen[] =
    {
        "This will re-encrypt the key area of all the nca's with keygen 0.",
        "Allows some games to be launched on lower firmware versions.",
    };

    const char *expl_mus[] =
    {
        "Enable / disable the music playback.",
    };

    const char *expl_snd[] =
    {
        "Enable / disable the sound effects.",
    };

    SDL_Rect box = { 255 / 1.5, 145 / 1.5, SCREEN_W - (box.x  * 2), SCREEN_H - (box.y * 2)};

    SettingFlag install_base = setting_get_install_base();
    SettingFlag install_upp = setting_get_install_upp();
    SettingFlag install_dlc = setting_get_install_dlc();
    SettingFlag install_lower_key_gen = setting_get_install_lower_key_gen();

    SettingFlag sound_sound = setting_get_sound();
    SettingFlag sound_music = setting_get_music();

    uint8_t cursor = 0;
    uint8_t r_cursor = 0;
    bool left_column = true;

    uint8_t cursor_max[] = { 4, 2, 0 };

    while (appletMainLoop())
    {
        input_t input = get_input();

        if (input.down)
        {
            play_sound(g_sound_effects.move, -1, 0);
        }

        if (input.down & KEY_B)
        {
            if (left_column)
                break;
            left_column = true;
        }

        else if (input.down & KEY_LEFT)
        {
            left_column = true;
        }
        else if (input.down & KEY_RIGHT)
        {
            left_column = false;
        }

        else if (input.down & KEY_DOWN)
        {
            if (left_column)
            {
                r_cursor = 0;
                cursor = move_cursor_down(cursor, 3);
            }
            else
            {
                r_cursor = move_cursor_down(r_cursor, cursor_max[cursor]);
            }
        }
        else if (input.down & KEY_UP)
        {
            if (left_column)
            {
                r_cursor = 0;
                cursor = move_cursor_up(cursor, 3);
            }
            else
            {
                r_cursor = move_cursor_up(r_cursor, cursor_max[cursor]);
            }
        }

        else if (input.down & KEY_A)
        {
            if (cursor == 2 && left_column) break;
            if (left_column) left_column = !left_column;
            else
            {
                switch (cursor)
                {
                    case 0:
                    {
                        switch (r_cursor)
                        {
                            case 0:
                            {
                                install_base = !install_base;
                                setting_set_install_base(install_base);
                                break;
                            }
                            case 1:
                            {
                                install_upp = !install_upp;
                                setting_set_install_upp(install_upp);
                                break;
                            }
                            case 2:
                            {
                                install_dlc = !install_dlc;
                                setting_set_install_dlc(install_dlc);
                                break;
                            }
                            case 3:
                            {
                                install_lower_key_gen = !install_lower_key_gen;
                                setting_set_install_lower_key_gen(install_lower_key_gen);
                                break;
                            }
                        }
                        break;
                    }
                    case 1:
                    {
                        switch (r_cursor)
                        {
                            case 0:
                            {
                                sound_music = !sound_music;
                                setting_set_music(sound_music);
                                break;
                            }
                            case 1:
                            {
                                sound_sound = !sound_sound;
                                setting_set_sound(sound_sound);
                                break;
                            }
                        }
                        break;
                    }
                    default:
                    {
                        return;
                    }
                }
            }
        }
        
        ui_display_dim_background();
        SDL_DrawShape(Colour_Nintendo_DarkGrey, box.x, box.y, box.w, box.h, true);
        SDL_DrawShape(Colour_Nintendo_LightBlack, box.x + 275, box.y + 72, box.w - 275, box.h - 72 - 75, true);

        // title
        SDL_DrawShape(Colour_Nintendo_White, box.x + 25, box.y + 75, box.w - 50, 2, true);
        SDL_DrawShape(Colour_Nintendo_White, box.x + 25, box.y + box.h - 75, box.w - 50, 2, true);
        SDL_DrawText(FONT_TEXT[QFontSize_28].fnt, box.x + 60, box.y + 30, Colour_Nintendo_White, "Settings");
        

        for (int i = 0, y = box.y + 155; i < 3; i++, y += 100)
        {
            SDL_DrawText(FONT_TEXT[QFontSize_25].fnt, box.x + 50, y, cursor == i ? Colour_Nintendo_Cyan : Colour_Nintendo_White, "%s", hdr[i]);
            if (cursor == i)
            {
                SDL_DrawShape(Colour_Nintendo_Cyan, box.x + 40, y - 4, 4, 25 + 8, true);
                if (left_column)
                {
                    ui_draw_highlight_box(&pulse_shape, box.x + 50, y, 200, 40);
                }
            }
        }
        
        switch (cursor)
        {
            case 0:
            {
                //base
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 350, box.y + 135, Colour_Nintendo_White, title_base);
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 800, box.y + 135, install_base ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver, "%s", install_base ? "On" : "Off");
                ui_draw_spacers(box.x + 350, box.y + 135, 23);
                if (!left_column && r_cursor == 0)
                    ui_draw_highlight_box(&pulse_shape, box.x + 350, box.y + 135, 550, 40);
                
                //upp
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 350, box.y + 195, Colour_Nintendo_White, title_update);
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 800, box.y + 195, install_upp ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver, "%s", install_upp ? "On" : "Off");
                ui_draw_spacers(box.x + 350, box.y + 195, 23);
                if (!left_column && r_cursor == 1)
                    ui_draw_highlight_box(&pulse_shape, box.x + 350, box.y + 195, 550, 40);
                
                //dlc
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 350, box.y + 255, Colour_Nintendo_White, title_dlc);
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 800, box.y + 255, install_dlc ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver, "%s", install_dlc ? "On" : "Off");
                ui_draw_spacers(box.x + 350, box.y + 255, 23);
                if (!left_column && r_cursor == 2)
                    ui_draw_highlight_box(&pulse_shape, box.x + 350, box.y + 255, 550, 40);

                //keygen
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 350, box.y + 335, Colour_Nintendo_White, "Lower Keygen Version");
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 800, box.y + 335, install_lower_key_gen ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver, "%s", install_lower_key_gen ? "On" : "Off");
                ui_draw_spacers(box.x + 350, box.y + 335, 23);
                if (!left_column && r_cursor == 3)
                    ui_draw_highlight_box(&pulse_shape, box.x + 350, box.y + 335, 550, 40);
                for (int i = 0, y = box.y + 390; i < 2; i++, y+= 25)
                {
                    SDL_DrawText(FONT_TEXT[QFontSize_15].fnt, box.x + 350, y, Colour_Nintendo_BrightSilver, "%s", expl_keygen[i]);
                }
                break;
            }
            case 1:
            {
                //music
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 350, box.y + 135, Colour_Nintendo_White, title_music);
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 800, box.y + 135, sound_music ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver, "%s", sound_music ? "On" : "Off");
                ui_draw_spacers(box.x + 350, box.y + 135, 23);
                if (!left_column && r_cursor == 0)
                    ui_draw_highlight_box(&pulse_shape, box.x + 350, box.y + 135, 550, 40);
                for (int i = 0, y = box.y + 185; i < 1; i++, y+= 25)
                {
                    SDL_DrawText(FONT_TEXT[QFontSize_15].fnt, box.x + 350, y, Colour_Nintendo_BrightSilver, "%s", expl_mus[i]);
                }

                //sound
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 350, box.y + 265, Colour_Nintendo_White, title_sound);
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 800, box.y + 265, sound_sound ? Colour_Nintendo_Cyan : Colour_Nintendo_Silver, "%s", sound_sound ? "On" : "Off");
                ui_draw_spacers(box.x + 350, box.y + 265, 23);
                if (!left_column && r_cursor == 1)
                    ui_draw_highlight_box(&pulse_shape, box.x + 350, box.y + 265, 550, 40);
                for (int i = 0, y = box.y + 315; i < 1; i++, y+= 25)
                {
                    SDL_DrawText(FONT_TEXT[QFontSize_15].fnt, box.x + 350, y, Colour_Nintendo_BrightSilver, "%s", expl_snd[i]);
                }
                break;
            }
            case 2:
            {
                SDL_DrawText(FONT_TEXT[QFontSize_23].fnt, box.x + 350, box.y + 135, Colour_Nintendo_BrightSilver, "Exit the settings menu.");
            }
            default:
            {
                break;
            }
        }

        SDL_UpdateRenderer();
    }
}

bool ui_display_yes_no_box(const char *message)
{
    uint8_t cursor = 1;
    bool flag = false;
    PulseShape_t pulse_shape = { { {0, 255, 187, 255} , false, 0 }, { 0 } };

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

void ui_display_error_box(ErrorCodes err, const char *func)
{
    if (!is_backlight_enabled())
    {
        enable_backlight(BacklightFade_Instant);
    }

    // play the error sound effect.
    play_sound(g_sound_effects.error, -1, 0);
    
    // display the popup box.
    ui_display_popup_box();

    SDL_Rect box = { 455, 470, 365, 65 };
    SDL_DrawButton(FONT_BUTTON[QFontSize_63].fnt, 0xE140, 608, 180, Colour_Nintendo_Red);
    SDL_DrawText(FONT_TEXT[QFontSize_25].fnt, 520, 270, Colour_Nintendo_White, "Error code: 0x%04X", err);
    SDL_DrawTextCenterX(FONT_TEXT[QFontSize_23].fnt, 325, box.x, box.w, Colour_Nintendo_White, "%s in %s()", error_get_description(err), func);
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 340, 380, Colour_Nintendo_BrightSilver, "If this message appears repeatedly, please open an issue.");
    SDL_DrawText(FONT_TEXT[QFontSize_20].fnt, 320, 415, Colour_Nintendo_BrightSilver, "https://github.com/ITotalJustice/Gamecard-Installer-NX/issues");
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
    if (!p_bar) return NULL;

    // shapes.
    SDL_Rect prog_bar = { 400, 470, 480, 12 };
    p_bar->empty_bar = create_shape(Colour_Nintendo_LightSilver, prog_bar.x, prog_bar.y, prog_bar.w, prog_bar.h, true);
    p_bar->filled_bar = create_shape(Colour_Nintendo_Cyan, prog_bar.x, prog_bar.y, ((float)done / (float)remaining) * prog_bar.w, prog_bar.h, true);

    // text.
    p_bar->text_header = create_text(&FONT_TEXT[QFontSize_25], 575, 200, Colour_Nintendo_White, "Installing...");
    p_bar->text_warning1 = create_text(&FONT_TEXT[QFontSize_20], 500, 260, Colour_Nintendo_BrightSilver, "Please do not remove the gamecard or");
    p_bar->text_warning2 = create_text(&FONT_TEXT[QFontSize_20], 500, 295, Colour_Nintendo_BrightSilver, "power off the system whilst installing.");
    p_bar->text_speed = create_text(&FONT_TEXT[QFontSize_20], 325, 360, Colour_Nintendo_White, "%.2f MiB/s", speed);
    p_bar->text_name = create_text(&FONT_TEXT[QFontSize_20], 410, 420, Colour_Nintendo_White, "%s", name);
    p_bar->text_time = create_text(&FONT_TEXT[QFontSize_20], prog_bar.x + 85, prog_bar.y + 40, Colour_Nintendo_White, "%d %s %d seconds remaining", eta_min, eta_min != 1 ? "minutes" : "minute", eta_sec);

    // vars.
    p_bar->speed = speed;
    p_bar->eta_min = eta_min;
    p_bar->eta_sec = eta_sec;

    return p_bar;
}

void ui_free_progress_bar(progress_bar_t *p_bar)
{
    if (!p_bar) return;
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
    if (!p_bar) return;

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
    if (g_game_info.icon)
    {
        draw_image_set(g_game_info.icon, 320, 200, g_game_info.icon->rect.w / 2, g_game_info.icon->rect.h / 2);
    }
    else
    {
        draw_image_set(g_empty_icon, 320, 200, g_empty_icon->rect.w / 2, g_empty_icon->rect.h / 2);
    }

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
    {
        update_button_spin();
    }

    int ret = check_if_option(&input);
    if (ret != -1)
    {
        g_cursor = ret;
        update_options();
        input.down |= KEY_A;
    }

    if (input.down & KEY_DOWN)
    {
        g_cursor = move_cursor_down(g_cursor, 3);
        play_sound(g_sound_effects.move, -1, 0);
    }

    else if (input.down & KEY_UP)
    {
        g_cursor = move_cursor_up(g_cursor, 3);
        play_sound(g_sound_effects.move, -1, 0);
    }

    else if (input.down & KEY_L)
    {
        gc_prev_game(&g_game_info);
    }

    else if (input.down & KEY_R)
    {
        gc_next_game(&g_game_info);
    }

    else if (input.down & KEY_B)
    {
        if (ui_display_yes_no_box("Would you like to exit?"))
            return Option_Exit;  
    }

    else if (input.down & KEY_X)
    {
        play_sound(g_sound_effects.move, -1, 0);
        ui_display_options();
    }

    else if (input.down & KEY_Y)
    {
        if (g_gc_inserted)
        {
            play_sound(g_sound_effects.move, -1, 0);
            ui_display_detailed_gamecard();
        }
    }

    else if (input.down & KEY_A)
    {
        play_sound(g_sound_effects.popup, -1, 0);
        switch (g_cursor)
        {
            case Option_Nand:
                if (g_gc_inserted)
                {
                    if (ui_display_yes_no_box("Install to the Nand?"))
                    {
                        gc_install(NcmStorageId_BuiltInUser);
                        update_storage_size();
                    }
                }
                break;
            case Option_SD:
                if (g_gc_inserted)
                {
                    if (ui_display_yes_no_box("Install to the SD Card?"))
                    {
                        gc_install(NcmStorageId_SdCard);
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
        {
            break;
        }

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
