#pragma once

#include <time.h>

#define TIME_IT(code) \
{ \
    timespec start = {}, end = {}; \
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start); \
    code; \
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end); \
    timespec d = diff(start, end); \
    double secs = (double) d.tv_sec + ((double) d.tv_nsec / 1000000000.0); \
    std::cout << secs << "s " << secs * 1000.0 << "ms" << std::endl; \
}

timespec diff(timespec start, timespec end);
