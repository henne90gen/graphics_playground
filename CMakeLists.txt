cmake_minimum_required(VERSION 3.13)

project(Playground)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCES 
    "${SRC_DIR}/Main.cpp"
    "${SRC_DIR}/ImGuiUtils.cpp"
    "${SRC_DIR}/OpenGLUtils.cpp"
    "${SRC_DIR}/MainMenu.cpp"
    "${SRC_DIR}/opengl/VertexBuffer.cpp"
    "${SRC_DIR}/opengl/IndexBuffer.cpp"
    "${SRC_DIR}/opengl/VertexBufferLayout.cpp"
    "${SRC_DIR}/opengl/VertexArray.cpp"
    "${SRC_DIR}/opengl/Texture.cpp"
    "${SRC_DIR}/opengl/Shader.cpp"
    "${SRC_DIR}/scenes/TestScene.cpp"
    "${SRC_DIR}/scenes/GammaCorrection.cpp"
    "${SRC_DIR}/scenes/LegacyTriangle.cpp"
    "${SRC_DIR}/scenes/triangle/Triangle.cpp"
    "${SRC_DIR}/scenes/texture_demo/TextureDemo.cpp"
    "${SRC_DIR}/scenes/cube/Cube.cpp"
    "${SRC_DIR}/scenes/landscape/Landscape.cpp"
        src/PerlinNoise.cpp src/PerlinNoise.h src/SimplexNoise.cpp src/SimplexNoise.h)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "${SRC_DIR}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter)

# GLFW
set(GLFW_DIR glfw)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(${PROJECT_NAME} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

# GLAD
set(GLAD_DIR glad)
add_library(glad "${GLAD_DIR}/src/glad.c")
target_include_directories(glad PRIVATE "${GLAD_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${GLAD_DIR}/include")
target_link_libraries(${PROJECT_NAME} glad)

# GLM
set(GLM_DIR glm)
target_include_directories(${PROJECT_NAME} PRIVATE "${GLM_DIR}")

# ImGui
set(IMGUI_DIR imgui)
add_library(imgui "${IMGUI_DIR}/imgui.cpp" "${IMGUI_DIR}/imgui_draw.cpp" "${IMGUI_DIR}/imgui_widgets.cpp")
target_include_directories(imgui PRIVATE "${IMGUI_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${IMGUI_DIR}")
target_link_libraries(${PROJECT_NAME} imgui)
