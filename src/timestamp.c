#include "timestamp.h"

#include <time.h>
#include <stdio.h>

#define TIMESTAMP_NANO_BUF_SIZE 32
static char timestamp_nano_buf[TIMESTAMP_NANO_BUF_SIZE];

char* timestamp_nano()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    size_t rc =
        strftime(&timestamp_nano_buf[0], TIMESTAMP_NANO_BUF_SIZE,
                 "%T", localtime(&ts.tv_sec));
    snprintf(&timestamp_nano_buf[rc], TIMESTAMP_NANO_BUF_SIZE, ".%09ld",
             ts.tv_nsec);

    return &timestamp_nano_buf[0];
}

