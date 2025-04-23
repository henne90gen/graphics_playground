cmake_minimum_required(VERSION 3.13)

project(Playground)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(GPLAY_ENABLE_ASAN "Enable address sanitizer" OFF)
option(GPLAY_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(GPLAY_ENABLE_CCACHE "Enables ccache, if it can be found on the system" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(GPLAY_USE_DEBUG_ASAN AND NOT WIN32)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -fno-omit-frame-pointer -fsanitize=address")
	set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

include(Ccache)
include(SceneFunc)
include(Emscripten)
include(ClangTidy)
include(FFMPEG)
include(Dependencies)
include(CompilerWarnings)

enable_testing()
configure_file(${CMAKE_SOURCE_DIR}/cmake/CTestCustom.cmake ${CMAKE_BINARY_DIR})

if (NOT WIN32)
    # disable volatile warning, because glm violates it
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-volatile")
endif()

add_subdirectory(src)
