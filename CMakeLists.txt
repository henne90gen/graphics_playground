cmake_minimum_required(VERSION 3.13)

project(Playground)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

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
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-volatile")
endif()

add_subdirectory(src)
