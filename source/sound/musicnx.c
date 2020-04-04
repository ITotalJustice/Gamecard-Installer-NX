#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include <threads.h>

#include "sound/musicnx.h"
#include "sound/sound.h"

#include "util/dir.h"
#include "util/file.h"


#define SONG_PATH   "sdmc:/switch/gamecard_installer"
#define SONG_MAX    100


// the working thread that will be playing the songs. 
thrd_t g_song_thrd;

// continue flag, if set to false, it cleans up the exits.
bool g_continue_flag = false;

// 100 songs max.
song_t *g_songs[SONG_MAX] = {0};
uint8_t g_song_total = 0;


void _scan_for_songs(const char *directory)
{
    struct dirent *d = {0};
    DIR *dir = open_dir(directory);
    if (!dir)
        return;

    char name_buf[0x200] = {0};

    while ((d = readdir(dir)))
    {
        // we don't want to scan the previous dir by accident!
        if (!strcmp(d->d_name, "..") || !strcmp(d->d_name, "."))
            continue;

        snprintf(name_buf, 0x200, "%s/%s", directory, d->d_name);

        // recursion > recursion.
        if (is_dir(name_buf))
            _scan_for_songs(name_buf);

        if (check_file_ext(name_buf, "mp3"))
        {
            g_songs[g_song_total] = load_song_from_file(name_buf);
            g_song_total++;

            // check if we are at the song max.
            if (g_song_total == SONG_MAX)
                return;
        }
    }
}

int play_song_thrd(void *in)
{
    // setup random seed.
    srand(time(NULL));

    // 100th of a second
    struct timespec sleep_time = { 0, 10000000 };

    // loop until the flag is set to false.
    while (g_continue_flag)
    {
        // if music isn't playing, then its the end of the song.
        // or the user deleted the song, but we assume that *didn't* happen.
        if (!Mix_PlayingMusic() && !Mix_PausedMusic())
        {
            play_song(g_songs[rand() % g_song_total], 0);
        }

        // sleep for a 100th of a second. No need to burn cycles!
        thrd_sleep(&sleep_time, NULL);
    }
    return 0;
}

void init_musicnx(void)
{
    // scan for songs.
    _scan_for_songs(SONG_PATH);

    // check if at least one song exists.
    if (g_song_total)
    {
        g_continue_flag = true;
        thrd_create(&g_song_thrd, play_song_thrd, NULL);
    }
}

void exit_musicnx(void)
{
    if (g_song_total)
    {
        // set the flag to false so that the thread closes.
        g_continue_flag = false;

        // wait for the thread to exit before freeing.
        thrd_join(g_song_thrd, NULL);

        // free songs.
        for (uint8_t i = 0; i < g_song_total; i++)
        {
            free_song(g_songs[i]);
        }
    }
}