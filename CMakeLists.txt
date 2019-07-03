cmake_minimum_required(VERSION 3.13)

project(Playground)

# zlib/png integration hack - removes all the errors related to export
# (zlib doesn't export; libpng wants to; in this case we don't care).
set(SKIP_INSTALL_EXPORT TRUE)

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
target_include_directories(glad PRIVATE "${GLAD_DIR}/include")

# GLM
set(GLM_DIR ${VENDOR_DIR}/glm)

# ImGui
set(IMGUI_DIR ${VENDOR_DIR}/imgui)
add_library(imgui "${IMGUI_DIR}/imgui.cpp" "${IMGUI_DIR}/imgui_draw.cpp" "${IMGUI_DIR}/imgui_widgets.cpp")
target_include_directories(imgui PRIVATE "${IMGUI_DIR}")

# FastNoise
set(FAST_NOISE_DIR ${VENDOR_DIR}/FastNoise)
add_subdirectory(${FAST_NOISE_DIR})

# Freetype2
set(FREETYPE_DIR ${VENDOR_DIR}/freetype2)
add_subdirectory(${FREETYPE_DIR})

# zlib
set(ZLIB_DIR ${VENDOR_DIR}/zlib)
add_subdirectory(${ZLIB_DIR})
GET_DIRECTORY_PROPERTY(
	ZLIB_BUILD_DIRECTORY
	DIRECTORY
	${ZLIB_DIR}
	DEFINITION
	CMAKE_CURRENT_BINARY_DIR
)
GET_DIRECTORY_PROPERTY(
	ZLIB_SHARED
	DIRECTORY
	${ZLIB_DIR}
	DEFINITION
	zlib
)
GET_DIRECTORY_PROPERTY(
	ZLIB_STATIC
	DIRECTORY
	${ZLIB_DIR}
	DEFINITION
	zlibstatic
)
SET(ZLIB_LIBRARY zlibstatic)
set(ZLIB_INCLUDE_DIR ${ZLIB_DIR} ${ZLIB_BUILD_DIRECTORY})

# libpng
set(LIBPNG_DIR ${VENDOR_DIR}/libpng)
set(PNG_SHARED OFF CACHE INTERNAL "")
set(PNG_BUILD_ZLIB ON CACHE INTERNAL "")
set(PNG_TESTS OFF CACHE INTERNAL "")
add_subdirectory(${LIBPNG_DIR})
GET_DIRECTORY_PROPERTY(
	LIBPNG_BUILD_DIR
	DIRECTORY
	${LIBPNG_DIR}
	DEFINITION
	CMAKE_CURRENT_BINARY_DIR
)
GET_DIRECTORY_PROPERTY(
	LIBPNG_STATIC
	DIRECTORY
	${LIBPNG_DIR}
	DEFINITION
	PNG_LIB_NAME_STATIC
)
GET_DIRECTORY_PROPERTY(
	LIBPNG_SHARED
	DIRECTORY
	${LIBPNG_DIR}
	DEFINITION
	PNG_LIB_NAME
)
set(LIBPNG_INCLUDE_DIR ${LIBPNG_DIR} ${LIBPNG_BUILD_DIR})

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# libjpeg
message(STATUS "Looking for libjpeg")
find_package(JPEG)
if (JPEG_FOUND)
    message(STATUS "Looking for libjpeg - found")
    add_compile_definitions(JPEG_FOUND)
else ()
    message(STATUS "Looking for libjpeg - not found")
endif ()

# Catch
set(CATCH_DIR ${VENDOR_DIR}/catch)
add_subdirectory(${CATCH_DIR})

enable_testing()
add_subdirectory(src)
