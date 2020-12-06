#pragma once

#ifdef DEBUG

#ifdef WIN32
#define ASSERT(x)                                                                                                      \
    if (!(x)) {                                                                                                        \
        __debugbreak();                                                                                                \
    }

#else
// TODO find a better way to do this
#define ASSERT(x) assert(x)
#endif

#else
#define ASSERT(x)
#endif
