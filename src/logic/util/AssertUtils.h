#pragma once

#ifdef DEBUG
#ifdef WIN32
#define ASSERT(x)                                                                                                      \
    if (!(x))                                                                                                          \
    __debugbreak()
#else
#define ASSERT(x)                                                                                                      \
    if (!(x))                                                                                                          \
    __builtin_trap()
#endif
#else
#define ASSERT(x)
#endif
