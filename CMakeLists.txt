cmake_minimum_required(VERSION 3.13)

project(Playground)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(SceneFunc)
include(FetchContent)

option(BUILD_OPENCV "Turn this on to build OpenCV from source" OFF)

# Clang Tidy
message(STATUS "Looking for clang-tidy")
find_program(
        CLANG_TIDY_EXE
        NAMES "clang-tidy"
        DOC "Path to clang-tidy executable"
)
if (NOT CLANG_TIDY_EXE)
    message(STATUS "Looking for clang-tidy - not found")
else ()
    message(STATUS "Looking for clang-tidy - found")
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "--fix")
endif ()

set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# GLAD
set(GLAD_DIR ${VENDOR_DIR}/glad)
add_library(glad "${GLAD_DIR}/src/glad.c")
target_include_directories(glad PUBLIC "${GLAD_DIR}/include")

# ImGui
set(IMGUI_DIR ${VENDOR_DIR}/imgui)
add_library(imgui
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        )
target_include_directories(imgui PUBLIC "${IMGUI_DIR}")
target_link_libraries(imgui PUBLIC glad glfw)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# FastNoise
set(FAST_NOISE_DIR ${VENDOR_DIR}/FastNoise)
add_subdirectory(${FAST_NOISE_DIR})

# Freetype2
set(FREETYPE_DIR ${VENDOR_DIR}/freetype2)
add_subdirectory(${FREETYPE_DIR})

# zlib
# zlib/png integration hack - removes all the errors related to export
# (zlib doesn't export; libpng wants to; in this case we don't care).
set(SKIP_INSTALL_EXPORT TRUE)
set(ZLIB_DIR ${VENDOR_DIR}/zlib)
add_subdirectory(${ZLIB_DIR})
get_directory_property(
        ZLIB_BUILD_DIRECTORY
        DIRECTORY
        ${ZLIB_DIR}
        DEFINITION
        CMAKE_CURRENT_BINARY_DIR
)
set(ZLIB_INCLUDE_DIR ${ZLIB_DIR} ${ZLIB_BUILD_DIRECTORY})
set(ZLIB_LIBRARY zlibstatic)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    include_directories(${ZLIB_INCLUDE_DIR})
endif ()
export(TARGETS ${ZLIB_LIBRARY} FILE zlibstatic.cmake)

# libpng
set(LIBPNG_DIR ${VENDOR_DIR}/libpng)
set(PNG_SHARED OFF CACHE INTERNAL "")
set(PNG_BUILD_ZLIB ON CACHE INTERNAL "")
set(PNG_TESTS OFF CACHE INTERNAL "")
add_subdirectory(${LIBPNG_DIR})
get_directory_property(
        LIBPNG_BUILD_DIR
        DIRECTORY
        ${LIBPNG_DIR}
        DEFINITION
        CMAKE_CURRENT_BINARY_DIR
)
get_directory_property(
        LIBPNG_STATIC
        DIRECTORY
        ${LIBPNG_DIR}
        DEFINITION
        PNG_LIB_NAME_STATIC
)
get_directory_property(
        LIBPNG_SHARED
        DIRECTORY
        ${LIBPNG_DIR}
        DEFINITION
        PNG_LIB_NAME
)
set(LIBPNG_LIBRARY ${LIBPNG_STATIC})
set(LIBPNG_INCLUDE_DIR ${LIBPNG_DIR} ${LIBPNG_BUILD_DIR})

# libjpeg
set(LIBJPEG_DIR ${VENDOR_DIR}/libjpeg)
add_subdirectory(${LIBJPEG_DIR})
get_directory_property(
        LIBJPEG_BUILD_DIR
        DIRECTORY
        ${LIBJPEG_DIR}
        DEFINITION
        CMAKE_CURRENT_BINARY_DIR
)
get_directory_property(
        LIBJPEG_STATIC
        DIRECTORY
        ${LIBJPEG_DIR}
        DEFINITION
        JPEG_LIB_NAME_STATIC
)
get_directory_property(
        LIBJPEG_SHARED
        DIRECTORY
        ${LIBJPEG_DIR}
        DEFINITION
        JPEG_LIB_NAME
)
set(LIBJPEG_LIBRARY ${LIBJPEG_STATIC})
set(LIBJPEG_INCLUDE_DIR ${LIBJPEG_DIR} ${LIBJPEG_BUILD_DIR})

# gif-h
set(GIF_H_DIR ${VENDOR_DIR}/gif-h)


# FFMPEG
function(find_ffmpeg_libraries)
    set(FFMPEG_FOUND TRUE)
    foreach (lib ${ARGV})
        message("-- Looking for ${lib}")
        find_library(${lib}_LIBRARY ${lib})

        if (NOT ${lib}_LIBRARY)
            message("-- Loooking for ${lib} - not found")
            set(FFMPEG_FOUND FALSE)
            break()
        endif ()

        add_library(${lib} SHARED IMPORTED)
        set_target_properties(${lib} PROPERTIES IMPORTED_LOCATION ${${lib}_LIBRARY})
        list(APPEND FFMPEG_LIBRARY ${lib})

        find_path(${lib}_INCLUDE_DIR lib${lib}/${lib}.h)
        if (${lib}_INCLUDE_DIR)
            list(APPEND FFMPEG_INCLUDE_DIR ${${lib}_INCLUDE_DIR})
        endif ()

        message("-- Looking for ${lib} - found")
    endforeach ()

    if (FFMPEG_FOUND)
        set(FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} PARENT_SCOPE)
        set(FFMPEG_LIBRARY ${FFMPEG_LIBRARY} PARENT_SCOPE)
        add_definitions(-DFFMPEG_FOUND=1)
    else ()
        add_definitions(-DFFMPEG_FOUND=0)
    endif ()
endfunction()

find_ffmpeg_libraries(x264 swscale avutil avformat avcodec)

# OpenMP
find_package(OpenMP)

# libsoundio
set(LIBSOUNDIO_DIR ${VENDOR_DIR}/libsoundio)
set(BUILD_STATIC_LIBS OFF)
set(BUILD_EXAMPLE_PROGRAMS OFF)
set(LIBSOUNDIO_LIBRARY libsoundio_shared)
set(LIBSOUNDIO_INCLUDE_DIR ${LIBSOUNDIO_DIR})
add_subdirectory(${LIBSOUNDIO_DIR})

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 7482de6071d21db77a7236155da44c172a7f6c9e # 3.3.8
)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
)
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG d7c8516a4b848c0291e3d75b627c0843f515f591 # v1.89.2
)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
)
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG 49aa374da96199d64fd3de9673b6f405bbc3de3e # some commits after v1.7.0
)

FetchContent_MakeAvailable(
        glfw
        glm
        imgui
        googletest
        benchmark
)

enable_testing()
add_subdirectory(src)
