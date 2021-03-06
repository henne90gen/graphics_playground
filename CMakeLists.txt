cmake_minimum_required(VERSION 3.13)

project(Playground)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(SceneFunc)

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

# GLFW
set(GLFW_DIR ${VENDOR_DIR}/glfw)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")

# GLAD
set(GLAD_DIR ${VENDOR_DIR}/glad)
add_library(glad "${GLAD_DIR}/src/glad.c")
target_include_directories(glad PUBLIC "${GLAD_DIR}/include")

# GLM
set(GLM_DIR ${VENDOR_DIR}/glm)

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

# Catch
set(CATCH_DIR ${VENDOR_DIR}/catch)
set(CATCH_INCLUDE_DIR ${CATCH_DIR}/single_include/catch2)
add_subdirectory(${CATCH_DIR})

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

# Google Benchmark
set(BENCHMARK_ENABLE_TESTING OFF)
add_subdirectory(${VENDOR_DIR}/benchmark)

# OpenMP
find_package(OpenMP)

# OpenCV
if (BUILD_OPENCV)
    set(BUILD_TESTS OFF CACHE BOOL "Disable tests" FORCE)
    set(BUILD_PERF_TESTS OFF CACHE BOOL "Disable perf tests" FORCE)
    set(BUILD_JAVA OFF CACHE BOOL "Disable Java bindings" FORCE)
    set(BUILD_ZLIB OFF CACHE BOOL "Build zlib" FORCE)
    set(BUILD_opencv_python2 OFF CACHE BOOL "Disable python2" FORCE)
    set(BUILD_opencv_python3 OFF CACHE BOOL "Disable python3" FORCE)
    set(BUILD_opencv_python_bindings_generator OFF CACHE BOOL "Disable python bindings generator" FORCE)
    set(BUILD_opencv_python_tests OFF CACHE BOOL "Disable python tests" FORCE)
    set(BUILD_opencv_java OFF CACHE BOOL "Disable java" FORCE)
    set(BUILD_opencv_java_bindings_generator OFF CACHE BOOL "Disable java bindings generator" FORCE)
    set(OPENCV_DIR ${VENDOR_DIR}/opencv)
    set(OPENCV_INCLUDE_DIR
            ${OPENCV_DIR}/include
            ${OPENCV_DIR}/modules/core/include
            ${OPENCV_DIR}/modules/imgproc/include
            ${OPENCV_DIR}/modules/videoio/include
            ${CMAKE_BINARY_DIR}
            )
    add_subdirectory(${OPENCV_DIR})
else ()
    find_package(OpenCV REQUIRED)
endif ()
set(OPENCV_LIBS opencv_core opencv_imgproc opencv_videoio)

# libsoundio
# TODO this produces an error when using Ninja to build
#set(LIBSOUNDIO_DIR ${VENDOR_DIR}/libsoundio)
#set(BUILD_EXAMPLE_PROGRAMS OFF)
#set(LIBSOUNDIO_LIBRARY libsoundio_shared)
#set(LIBSOUNDIO_INCLUDE_DIR ${LIBSOUNDIO_DIR})
#add_subdirectory(${LIBSOUNDIO_DIR})

enable_testing()
add_subdirectory(src)
