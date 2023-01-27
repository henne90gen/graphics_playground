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
set(ZLIB_INCLUDE_DIRS ${ZLIB_DIR} ${ZLIB_BUILD_DIRECTORY})
set(ZLIB_LIBRARIES zlibstatic)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    include_directories(${ZLIB_INCLUDE_DIRS})
endif ()
export(TARGETS ${ZLIB_LIBRARIES} FILE zlibstatic.cmake)

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
        freetype
        GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
        GIT_TAG e8ebfe988b5f57bfb9a3ecb13c70d9791bce9ecf # 2.12.1
)
FetchContent_Declare(
        fastnoise
        GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
        GIT_TAG 95900f7372d9aad1691cfeabf45103a132a4664f # 11.11.2022
)
FetchContent_Declare(
        libsoundio
        GIT_REPOSITORY https://github.com/andrewrk/libsoundio.git
        GIT_TAG a24148e15ffd3fe3fe3c5ee22fd6bbd7351c7539 # 12.10.2022
)
FetchContent_Declare(
        libjpeg
        GIT_REPOSITORY https://github.com/henne90gen/libjpeg.git
        GIT_TAG 27391253448f7edd556aa9f25c38ac4822cf6f17
)
FetchContent_Declare(
        libpng
        GIT_REPOSITORY https://github.com/glennrp/libpng.git
        GIT_TAG 07b8803110da160b158ebfef872627da6c85cbdf # v1.6.39
)
FetchContent_Declare(
        gif
        GIT_REPOSITORY https://github.com/charlietangora/gif-h.git
        GIT_TAG 3d2657b9ad40aac9fd6f75ad079335856e94d664 # 10.11.2020
)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG b796f7d44681514f58a683a3a71ff17c94edb0c1 # v1.13.0
)
FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG d572f4777349d43653b21d6c2fc63020ab326db2 # v1.7.1
)

set(BENCHMARK_ENABLE_TESTING OFF) # benchmark
set(BENCHMARK_ENABLE_INSTALL OFF) # benchmark
set(BENCHMARK_INSTALL_DOCS OFF) # benchmark
set(BENCHMARK_ENABLE_GTEST_TESTS OFF) # benchmark
set(GLFW_BUILD_DOCS OFF) # glfw
set(GLFW_INSTALL OFF) # glfw
set(FT_DISABLE_ZLIB ON) # freetype
set(SKIP_INSTALL_ALL ON) # freetype
set(BUILD_STATIC_LIBS OFF) # libsoundio
set(BUILD_DYNAMIC_LIBS ON) # libsoundio
set(BUILD_TESTS OFF) # libsoundio
set(BUILD_EXAMPLE_PROGRAMS OFF) # libsoundio
set(PNG_STATIC OFF) # libpng
set(PNG_SHARED ON) # libpng
set(PNG_BUILD_ZLIB ON) # libpng
set(PNG_TESTS OFF) # libpng
set(PNG_EXECUTABLES OFF) # libpng

FetchContent_MakeAvailable(
        glfw
        glm
        imgui
        freetype
        fastnoise
        libsoundio
        libjpeg
        libpng
        gif
        googletest
        benchmark
)

# FastNoise
add_library(FastNoise INTERFACE)
target_include_directories(FastNoise INTERFACE "${fastnoise_SOURCE_DIR}/Cpp")

# ImGUI
add_library(imgui
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp")
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
target_link_libraries(imgui PUBLIC glad glfw)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# libsoundio
target_include_directories(libsoundio_shared PUBLIC "${libsoundio_SOURCE_DIR}")

# libpng
target_include_directories(png PUBLIC "${libpng_SOURCE_DIR}")
target_include_directories(png PUBLIC "${libpng_BINARY_DIR}")

# GIF
add_library(gif INTERFACE)
target_include_directories(gif INTERFACE ${gif_SOURCE_DIR})

# OpenMP
find_package(OpenMP)

enable_testing()
add_subdirectory(src)
