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
        GIT_TAG v1.89.8
)
FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://git.savannah.gnu.org/git/freetype/freetype2.git
        GIT_TAG VER-2-13-2
)
FetchContent_Declare(
        fastnoise
        GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
        GIT_TAG 95900f7372d9aad1691cfeabf45103a132a4664f # 11.11.2022
)
FetchContent_Declare(
        libsoundio
        GIT_REPOSITORY https://github.com/andrewrk/libsoundio.git
        GIT_TAG 49a1f78b50eb0f5a49d096786a95a93874a2592a # 20.07.2023
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
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG 04f42ceca40f73e2978b50e93806c2a18c1281fc # v1.2.13
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

FetchContent_MakeAvailable(zlib)

# zlib configuration for libpng
set(ZLIB_LIBRARY zlibstatic) # zlib
set(ZLIB_LIBRARIES zlibstatic) # zlib
set(ZLIB_INCLUDE_DIR "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}") # zlib
set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR}) # zlib
target_include_directories(${ZLIB_LIBRARY} PUBLIC ${ZLIB_INCLUDE_DIR}) # zlib
if (EMSCRIPTEN)
    target_include_directories(example PUBLIC ${ZLIB_INCLUDE_DIR})
    target_include_directories(example64 PUBLIC ${ZLIB_INCLUDE_DIR})
    target_include_directories(minigzip PUBLIC ${ZLIB_INCLUDE_DIR})
    target_include_directories(minigzip64 PUBLIC ${ZLIB_INCLUDE_DIR})
endif ()

set(SKIP_INSTALL_EXPORT ON CACHE BOOL "" FORCE) # libpng
set(PNG_STATIC OFF CACHE BOOL "" FORCE) # libpng
set(PNG_SHARED ON CACHE BOOL "" FORCE) # libpng
set(PNG_BUILD_ZLIB ON CACHE BOOL "" FORCE) # libpng
set(PNG_TESTS OFF CACHE BOOL "" FORCE) # libpng
set(PNG_EXECUTABLES OFF CACHE BOOL "" FORCE) # libpng
FetchContent_MakeAvailable(libpng)

# libpng
set(PNG_LIBRARY png)
set(PNG_PNG_INCLUDE_DIR "${libpng_SOURCE_DIR}" "${libpng_BINARY_DIR}")
target_include_directories(png PUBLIC ${PNG_PNG_INCLUDE_DIR})

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE) # benchmark
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE) # benchmark
set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "" FORCE) # benchmark
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE) # benchmark
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE) # glfw
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE) # glfw
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE) # freetype
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE) # freetype
set(BUILD_STATIC_LIBS OFF CACHE BOOL "" FORCE) # libsoundio
set(BUILD_DYNAMIC_LIBS ON CACHE BOOL "" FORCE) # libsoundio
set(BUILD_TESTS OFF CACHE BOOL "" FORCE) # libsoundio
set(BUILD_EXAMPLE_PROGRAMS OFF CACHE BOOL "" FORCE) # libsoundio

FetchContent_MakeAvailable(
        glm
        imgui
        freetype
        fastnoise
        libsoundio
        libjpeg
        gif
        zlib
)

if (NOT EMSCRIPTEN)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(
            glfw
            googletest
            benchmark
    )
    find_package(OpenGL REQUIRED)
    include(GoogleTest)
endif ()


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

# GIF
add_library(gif INTERFACE)
target_include_directories(gif INTERFACE ${gif_SOURCE_DIR})

# OpenMP
find_package(OpenMP)
