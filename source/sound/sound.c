#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>
#include <switch.h>

#include "sound/sound.h"


void create_sound_cfg_default(sound_cfg *out)
{
    out->flags = MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_OPUS;
    out->freq = DEFAULT_BITRATE;
    out->format = DEFAULT_FORMAT;
    out->channels = DEFAULT_CHANNELS;
    out->chunk_size = DEAFULT_CHUNK_SIZE;
    out->volume = 32;
}

void init_sound(sound_cfg *cfg)
{
    Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_OPUS);
    Mix_OpenAudio(cfg->freq, cfg->format, cfg->channels, cfg->chunk_size);
    // the music is very loud by default, so i reduce the volume by /4.
    Mix_VolumeMusic(cfg->volume);
}

void init_sound_default(void)
{
    sound_cfg cfg;
    create_sound_cfg_default(&cfg);
    init_sound(&cfg);
}

song_t *load_song_from_file(const char *file)
{
    return Mix_LoadMUS(file);
}

song_t *load_song_from_mem(void *data, int size, int free_flag)
{
    return Mix_LoadMUS_RW(SDL_RWFromMem(data, size), free_flag);
}

sound_t *load_sound_from_file(const char *file)
{
    return Mix_LoadWAV(file);
}

sound_t *load_sound_from_mem(void *data, int size, int free_flag)
{
    return Mix_LoadWAV_RW(SDL_RWFromMem(data, size), free_flag);
}

void play_song(song_t *song, int loops)
{
    Mix_HaltMusic();
    Mix_PlayMusic(song, loops);
}

bool play_sound(sound_t *sound, int channel, int loops)
{
    return Mix_PlayChannel(channel, sound, loops) ? true : false;
}

bool play_sound_from_file(const char *file, int channel, int loops)
{
    sound_t *sound = load_sound_from_file(file);
    return sound ? play_sound(sound, channel, loops) : false;
    //free_sound(sound);
}

void pause_resume_song(void)
{
    Mix_PausedMusic() ? Mix_ResumeMusic() : Mix_PauseMusic();
}

void pause_resume_sound(int channel)
{
    Mix_Paused(channel) ? Mix_Resume(channel) : Mix_Pause(channel);
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
    free_sound(*sound);
    *sound = NULL;
}

void exit_sound()
{
    Mix_HaltChannel(-1);
    Mix_CloseAudio();
    Mix_Quit();
}