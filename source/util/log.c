#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <threads.h>
//#include <switch.h>

#include "util/log.h"


#define DEBUG
#define LOG_OUTPUT   "sdmc:/log.txt"


uint32_t g_flag = 0;
mtx_t log_mtx = {0};
mtx_t stdio_mtx = {0};


bool init_log(uint32_t flags)
{
    if (!(g_flag = flags))
        return false;
    
    if (g_flag & LogFlags_File)
    {
        FILE *fp = fopen(LOG_OUTPUT, "w");
        fclose(fp);
        mtx_init(&log_mtx, mtx_plain);
    }
    if ((g_flag & LogFlags_Nxlink) >> 1)
    {
        //socketInitializeDefault();
        //nxlinkStdio();
    }

    return true;
}

void write_log(const char *text, ...)
{
    // lock the mtx.
    mtx_lock(&log_mtx);
    
    // write the text to file.
    FILE *fp = fopen(LOG_OUTPUT, "a");

    va_list v;
    va_start(v, text);
    vfprintf(fp, text, v);
    va_end(v);

    // write a newline so that the next entry in the log will be on a...newline!
    fprintf(fp, "\n");
    fclose(fp);

    // unlock the mtx.
    mtx_unlock(&log_mtx);
}

void exit_log(void)
{
    if (g_flag & LogFlags_File)
        mtx_destroy(&log_mtx);
}