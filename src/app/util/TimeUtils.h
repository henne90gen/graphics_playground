#pragma once

#define TIME_IT(code) \
{ \
    timespec start = {}, end = {}; \
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start); \
    code; \
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end); \
    timespec d = diff(start, end); \
    std::cout << d.tv_sec << ":" << d.tv_nsec << std::endl; \
}

timespec diff(timespec start, timespec end) {
    timespec temp = {};
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}
