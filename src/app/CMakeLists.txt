
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCES
        ${SRC_DIR}/Main.cpp
        ${SRC_DIR}/MainMenu.cpp
        ${SRC_DIR}/util/ImGuiUtils.cpp
        ${SRC_DIR}/util/PerlinNoise.cpp
        ${SRC_DIR}/util/SimplexNoise.cpp
        ${SRC_DIR}/opengl/Utils.cpp
        ${SRC_DIR}/opengl/VertexBuffer.cpp
        ${SRC_DIR}/opengl/IndexBuffer.cpp
        ${SRC_DIR}/opengl/VertexBufferLayout.cpp
        ${SRC_DIR}/opengl/VertexArray.cpp
        ${SRC_DIR}/opengl/Texture.cpp
        ${SRC_DIR}/opengl/Shader.cpp
        ${SRC_DIR}/scenes/TestScene.cpp
        ${SRC_DIR}/scenes/LegacyTriangle.cpp
        ${SRC_DIR}/scenes/triangle/Triangle.cpp
        ${SRC_DIR}/scenes/texture_demo/TextureDemo.cpp
        ${SRC_DIR}/scenes/cube/Cube.cpp
        ${SRC_DIR}/scenes/landscape/Landscape.cpp
        ${SRC_DIR}/scenes/rubiks_cube/RubiksCube.cpp
        )

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)

target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} logic)

target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${LOGIC_DIR})
