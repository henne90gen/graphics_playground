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
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(opengl PUBLIC glad glm)
target_include_directories(opengl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    find_package(OpenGL REQUIRED)
    target_include_directories(opengl PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(opengl PUBLIC ${OPENGL_gl_LIBRARY})
elseif (APPLE)
else ()
    target_link_libraries(opengl PUBLIC stdc++fs)
    target_compile_options(opengl PRIVATE
            -Wall
            -Wextra
            -pedantic
            -Wno-unused-parameter)
endif ()
