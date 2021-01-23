
add_library(core
        Main.cpp
        Scene.cpp
        util/Camera.cpp
        util/DataReadUtils.cpp
        util/ImGuiUtils.cpp
        util/TimeUtils.cpp
        fourier_transform/Fourier.cpp
        )

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_options(core PRIVATE /std:c++17)
else ()
    target_compile_options(core PRIVATE
            -Wall
            -Wextra
            -pedantic
            -Wno-unused-parameter)
    add_definitions(-std=c++17)
endif ()

set_target_properties(core PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif ()

if (CLANG_TIDY_EXE AND RUN_CLANG_TIDY)
    set_target_properties(core PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif ()

target_compile_definitions(core PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(core
        glfw
        glad
        imgui
        opengl
        OpenMP::OpenMP_CXX)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_package(OpenGL REQUIRED)
    target_include_directories(core PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(core ${OPENGL_gl_LIBRARY})
elseif (APPLE)
else ()
    target_link_libraries(core stdc++fs)
endif ()

target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLFW_DIR}/include
        ${GLAD_DIR}/include
        ${GLM_DIR}
        ${IMGUI_DIR})
