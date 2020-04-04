#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

#include "sound/sound.h"


bool g_play_sound = true;


void create_sound_cfg_default(sound_cfg *out)
{
    if (!out)
    {
        return;
    }

    out->flags = MIX_INIT_MP3 | MIX_INIT_OGG;
    out->freq = MIX_DEFAULT_FREQUENCY;
    out->format = MIX_DEFAULT_FORMAT;
    out->channels = DEFAULT_CHANNELS;
    out->chunk_size = 1024;
    out->volume = 32;
}

void init_sound(sound_cfg *cfg)
{
    if (!cfg)
    {
        return;
    }

    Mix_Init(cfg->flags);
    Mix_OpenAudio(cfg->freq, cfg->format, cfg->channels, cfg->chunk_size);
    // the music is very loud by default, so i reduce the volume by /4.
    Mix_VolumeMusic(cfg->volume);
}

void init_sound_default(void)
{
    sound_cfg cfg = {0};
    create_sound_cfg_default(&cfg);
    init_sound(&cfg);
}

song_t *load_song_from_file(const char *file)
{
    if (!file)
    {
        return NULL;
    }

    return Mix_LoadMUS(file);
}

song_t *load_song_from_mem(void *data, int size, int free_flag)
{
    if (!data || !size)
    {
        return NULL;
    }

    return Mix_LoadMUS_RW(SDL_RWFromMem(data, size), free_flag);
}

sound_t *load_sound_from_file(const char *file)
{
    if (!file)
    {
        return NULL;
    }

    return Mix_LoadWAV(file);
}

sound_t *load_sound_from_mem(void *data, int size, int free_flag)
{
    if (!data || !size)
    {
        return NULL;
    }

    return Mix_LoadWAV_RW(SDL_RWFromMem(data, size), free_flag);
}

void play_song(song_t *song, int loops)
{
    if (!song)
    {
        return;
    }

    Mix_HaltMusic();
    Mix_PlayMusic(song, loops);
}

bool play_sound(sound_t *sound, int channel, int loops)
{
    if (!sound)
    {
        return false;
    }

    if (!g_play_sound) return true;
    return Mix_PlayChannel(channel, sound, loops) ? true : false;
}

bool play_sound_from_file(const char *file, int channel, int loops)
{
    if (!file)
    {
        return false;
    }

    sound_t *sound = load_sound_from_file(file);
    return sound ? play_sound(sound, channel, loops) : false;
    //free_sound(sound);
}

void pause_resume_song(void)
{
    Mix_PausedMusic() ? Mix_ResumeMusic() : Mix_PauseMusic();
}

void pause_resume_sound(void)
{
    g_play_sound = !g_play_sound;
}

int get_volume(void)
{
    return Mix_Volume(-1, -1);
}

void set_volume(int volume)
{
    Mix_Volume(-1, volume);
}

void free_song(song_t *song)
{
    if (song) Mix_FreeMusic(song);
}

void free_sound(sound_t *sound)
{
    if (sound) Mix_FreeChunk(sound);
}

void free_sound_null(sound_t **sound)
{
    if (!sound || !*sound)
    {
        return;
    }

    free_sound(*sound);
    *sound = NULL;
}

void exit_sound()
{
    Mix_HaltChannel(-1);
    Mix_CloseAudio();
    Mix_Quit();
}