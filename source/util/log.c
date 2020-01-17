#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <threads.h>
#include <switch.h>

#include "util/log.h"


#define DEBUG
#define LOG_OUTPUT   "sdmc:/switch/gamecard_installer/log.txt"


bool g_log_flag = false;
mtx_t g_log_mtx = {0};


bool init_log(void)
{   
    FILE *fp = fopen(LOG_OUTPUT, "w");
    if (!fp)
        return false;
    fclose(fp);

    mtx_init(&g_log_mtx, mtx_plain);
    socketInitializeDefault();
    nxlinkStdio();
    
    printf("\ncan you see me?\n");
	printf("if you can, that would be great\n");
	printf("otherwise you won't see debug messages :(\n\n");

    return g_log_flag = true;
}

void write_log(const char *text, ...)
{
    if (!g_log_flag)
        return;

    // lock the mtx.
    mtx_lock(&g_log_mtx);
    
    // write the text to file.
    FILE *fp = fopen(LOG_OUTPUT, "a");

    va_list v;
    va_start(v, text);
    vfprintf(fp, text, v);
    vfprintf(stdout, text, v);
    va_end(v);

    // write a newline so that the next entry in the log will be on a...newline!
    fprintf(fp, "\n");
    fclose(fp);

    // unlock the mtx.
    mtx_unlock(&g_log_mtx);
}

void exit_log(void)
{
    if (g_log_flag)
    {
        mtx_destroy(&g_log_mtx);
        socketExit();
    }
}