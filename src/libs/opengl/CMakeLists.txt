add_library(opengl
        gl/BufferLayout.cpp
        gl/BufferLayoutElement.cpp
        gl/IndexBuffer.cpp
        gl/Shader.cpp
        gl/Texture.cpp
        gl/VertexArray.cpp
        gl/VertexBuffer.cpp
        util/FileUtils.cpp
        util/OpenGLUtils.cpp
        util/RenderUtils.cpp)

set_target_properties(opengl PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(opengl glad glm)

target_include_directories(opengl PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_DIR})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_package(OpenGL REQUIRED)
    target_include_directories(opengl PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(opengl ${OPENGL_gl_LIBRARY})
    target_compile_options(opengl PRIVATE /std:c++17)
elseif (APPLE)
else ()
    target_link_libraries(opengl stdc++fs)
    target_compile_options(opengl PRIVATE
            -Wall
            -Wextra
            -pedantic
            -Wno-unused-parameter)
    add_definitions(-std=c++17)
endif ()
