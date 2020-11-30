
file(GLOB_RECURSE SHADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/scenes "*.glsl")
foreach (INPUT_FILE ${SHADER_FILES})
    get_filename_component(INPUT_FILE_DIR ${INPUT_FILE} DIRECTORY)
    get_filename_component(INPUT_FILE_NAME ${INPUT_FILE} NAME)
    set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    set(OUTPUT_FILE_NAME ${INPUT_FILE_DIR}_${INPUT_FILE_NAME}.cpp)
    set(OUTPUT_FILE ${SHADER_DIR}/${OUTPUT_FILE_NAME})
    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND cd ${CMAKE_SOURCE_DIR} && python -m scripts compile-shader ${CMAKE_CURRENT_SOURCE_DIR}/scenes/${INPUT_FILE} ${OUTPUT_FILE}
            COMMENT "Building shader object ${OUTPUT_FILE_NAME}"
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/scenes/${INPUT_FILE})
    list(APPEND SHADERS ${OUTPUT_FILE})
endforeach ()

add_library(core
        Main.cpp
        Scene.cpp
        util/Image.cpp
        util/ImGuiUtils.cpp
        util/OpenGLUtils.cpp
        util/RenderUtils.cpp
        util/ScreenRecorder.cpp
        util/VideoSaver.cpp
        opengl/BufferLayout.cpp
        opengl/BufferLayoutElement.cpp
        opengl/IndexBuffer.cpp
        opengl/Model.cpp
        opengl/Shader.cpp
        opengl/Texture.cpp
        opengl/VertexBuffer.cpp
        opengl/VertexArray.cpp
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
        logic
        glfw
        glad
        imgui
        ${LIBPNG_LIBRARY}
        ${LIBJPEG_LIBRARY}
        ${FFMPEG_LIBRARY}
        ${ZLIB_LIBRARY}
        )

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_package(OpenGL REQUIRED)
    target_include_directories(core PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(core ${OPENGL_gl_LIBRARY})
elseif (APPLE)
else ()
    target_link_libraries(core stdc++fs)
endif ()

target_include_directories(core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLFW_DIR}/include
        ${GLAD_DIR}/include
        ${GLM_DIR}
        ${IMGUI_DIR}
        ${FAST_NOISE_DIR}
        ${ZLIB_INCLUDE_DIR}
        ${LIBPNG_INCLUDE_DIR}
        ${LIBJPEG_INCLUDE_DIR}
        ${GIF_H_DIR}
        ${FREETYPE_DIR}
        ${FFMPEG_INCLUDE_DIR}
        ${OPENCV_INCLUDE_DIR}
        ${LOGIC_DIR}
        )

set(RESOURCES
        scenes/bloom_effect/models
        scenes/font_demo/fonts
        scenes/light_demo/models
        scenes/model_loading/models
        scenes/normal_mapping/models
        )

add_custom_target(copy-resources
        COMMAND cd ${CMAKE_SOURCE_DIR} && python -m scripts copy-resources ${CMAKE_CURRENT_SOURCE_DIR} ${RESOURCES} ${CMAKE_BINARY_DIR}/bin)
add_dependencies(core copy-resources)
