cmake_minimum_required(VERSION 3.13)

project(Playground)

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
    set(DO_CLANG_TIDY
            "${CLANG_TIDY_EXE}"
            "-fix"
            "-checks=*,-fuchsia-default-arguments,-cppcoreguidelines-pro-type-union-access,-hicpp-vararg,-cppcoreguidelines-pro-type-vararg,-cppcoreguidelines-owning-memory,-readability-magic-numbers,-google-runtime-int,-readability-braces-around-statements,-cppcoreguidelines-pro-bounds-constant-array-index")
endif ()


# GLFW
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")

# GLAD
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glad)
add_library(glad "${GLAD_DIR}/src/glad.c")
target_include_directories(glad PRIVATE "${GLAD_DIR}/include")

# GLM
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)

# ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(imgui "${IMGUI_DIR}/imgui.cpp" "${IMGUI_DIR}/imgui_draw.cpp" "${IMGUI_DIR}/imgui_widgets.cpp")
target_include_directories(imgui PRIVATE "${IMGUI_DIR}")

# FastNoise
set(FAST_NOISE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FastNoise)
add_subdirectory(${FAST_NOISE_DIR})

# Freetype2
set(FREETYPE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/freetype2)
add_subdirectory(${FREETYPE_DIR})

# libpng
set(LIBPNG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libpng)
add_subdirectory(${LIBPNG_DIR})

# Catch
set(CATCH_DIR ${CMAKE_CURRENT_SOURCE_DIR}/catch)
add_subdirectory(${CATCH_DIR})

enable_testing()
add_subdirectory(src)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(${PROJECT_NAME} glfw "${GLFW_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} FastNoise)
target_link_libraries(${PROJECT_NAME} freetype)
target_link_libraries(${PROJECT_NAME} logic)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FAST_NOISE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_DIR})
