
if (NOT GPLAY_ENABLE_CCACHE)
    return()
endif()

message(STATUS "Looking for program ccache")

find_program(CCACHE_FOUND ccache)

if (CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    message(STATUS "Looking for program ccache - found")
else ()
    message(STATUS "Looking for program ccache - not found")
endif()
