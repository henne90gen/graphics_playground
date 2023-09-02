
add_library(core
        Main.cpp
        Scene.cpp
        camera/Camera.cpp
        util/DataReadUtils.cpp
        util/ImGuiUtils.cpp
        util/TimeUtils.cpp
        fourier_transform/Fourier.cpp)

if (MSVC)
else ()
    target_compile_options(core PRIVATE
            -Wall
            -Wextra
            -pedantic
            -Wno-unused-parameter)
endif ()

set_target_properties(core PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif ()

if (CLANG_TIDY_EXE AND RUN_CLANG_TIDY)
    set_target_properties(core PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif ()

target_compile_definitions(core PRIVATE GLFW_INCLUDE_NONE)

set(WITH_SCREEN_RECORDING 1)
if (NOT WITH_SCREEN_RECORDING)
    message("-- Screen Recording: disabled")
else ()
    message("-- Screen Recording: enabled")
    target_compile_definitions(core PUBLIC WITH_SCREEN_RECORDING)
    target_link_libraries(core PUBLIC screen_recording)
endif ()

target_link_libraries(core PUBLIC
        glfw
        imgui
        opengl)

if (OpenMP_FOUND)
    target_link_libraries(core PUBLIC OpenMP::OpenMP_CXX)
endif ()

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
