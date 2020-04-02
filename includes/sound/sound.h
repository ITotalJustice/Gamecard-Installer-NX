#ifndef _SOUND_H_
#define _SOUND_H_

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL_mixer.h>


#define DEFAULT_FLAGS       (MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_MID | MIX_INIT_OPUS)
#define DEFAULT_BITRATE     48000
#define DEFAULT_FORMAT      0x8020
#define DEAFULT_CHUNK_SIZE  1024
#define DEFAULT_CHANNELS    2
#define DEFAULT_VOLUME      64

typedef Mix_Chunk sound_t;
typedef Mix_Music song_t;

typedef enum
{
    SoundBitrate_Default = DEFAULT_BITRATE,
    SoundBitrate_24000 = 24000,
    SoundBitrate_41000 = 41000,
    SoundBitrate_48000 = 48000,
} SoundBitrate;

typedef struct
{
    int flags;
    int freq;
    uint16_t format;
    int channels;
    int chunk_size;
    int volume;
} sound_cfg;


//
void create_sound_cfg_default(sound_cfg *out);

//
void init_sound(sound_cfg *cfg);

//
void init_sound_default(void);

//
song_t *load_song_from_file(const char *file);

//
song_t *load_song_from_mem(void *data, int size, int free_flag);

//
sound_t *load_sound_from_file(const char *file);

//
sound_t *load_sound_from_mem(void *data, int size, int free_flag);

//
void play_song(song_t *song, int loops);

//
bool play_sound(sound_t *sound, int channel, int loops);

//
bool play_sound_from_file(const char *file, int channel, int loops);

//
void pause_resume_song(void);

//
void pause_resume_sound(void);

//
void free_song(song_t *song);

//
void free_sound(sound_t *sound);

//
void free_sound_null(sound_t **sound);

//
void exit_sound(void);

#endif