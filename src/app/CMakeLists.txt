
set(SOURCES
        Main.cpp
        MainMenu.cpp
        util/ImGuiUtils.cpp
        util/PerlinNoise.cpp
        util/SimplexNoise.cpp
        opengl/Utils.cpp
        opengl/VertexBuffer.cpp
        opengl/IndexBuffer.cpp
        opengl/VertexBufferLayout.cpp
        opengl/VertexArray.cpp
        opengl/Texture.cpp
        opengl/Shader.cpp
        scenes/TestScene.cpp
        scenes/LegacyTriangle.cpp
        scenes/triangle/Triangle.cpp
        scenes/texture_demo/TextureDemo.cpp
        scenes/cube/Cube.cpp
        scenes/landscape/Landscape.cpp
        scenes/rubiks_cube/RubiksCubeScene.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Wno-unused-parameter)

target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

target_link_libraries(${PROJECT_NAME}
        glfw ${GLFW_LIBRARIES}
        glad
        imgui
        logic)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLFW_DIR}/include
        ${GLAD_DIR}/include
        ${GLM_DIR}
        ${IMGUI_DIR}
        ${LOGIC_DIR})
