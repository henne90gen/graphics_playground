
set(SOURCES
        Main.cpp
        util/MainMenu.cpp
        util/ImGuiUtils.cpp
        util/OpenGLUtils.cpp
        util/Image.cpp
        opengl/VertexBuffer.cpp
        opengl/IndexBuffer.cpp
        opengl/BufferLayout.cpp
        opengl/BufferLayoutElement.cpp
        opengl/VertexArray.cpp
        opengl/Texture.cpp
        opengl/Shader.cpp
        opengl/Model.cpp
        scenes/Scene.cpp
        scenes/test_scene/TestScene.cpp
        scenes/legacy_triangle/LegacyTriangle.cpp
        scenes/triangle/Triangle.cpp
        scenes/texture_demo/TextureDemo.cpp
        scenes/gamma_calculation/GammaCalculation.cpp
        scenes/cube/Cube.cpp
        scenes/landscape/Landscape.cpp
        scenes/marching_cubes/MarchingCubesScene.cpp
        scenes/font_demo/FontDemo.cpp
        scenes/rubiks_cube/RubiksCubeScene.cpp
        scenes/model_loading/ModelLoading.cpp
        scenes/light_demo/LightDemo.cpp
        scenes/fourier_transform/FourierTransform.cpp
        scenes/normal_mapping/NormalMapping.cpp
        scenes/a_star/AStar.cpp
        scenes/surface_smoothing/SurfaceSmoothing.cpp
        scenes/water_demo/WaterDemo.cpp
        scenes/ray_casting_3d/RayCasting3D.cpp
        scenes/ray_casting_2d/RayCasting2D.cpp
        util/ScreenRecorder.cpp util/ScreenRecorder.h)

add_executable(${PROJECT_NAME} ${SOURCES})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++17)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall
            -Wextra
            -pedantic
            -Wno-unused-parameter)
    add_definitions(-std=c++17)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif ()

if (CLANG_TIDY_EXE AND RUN_CLANG_TIDY)
    set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${LIBPNG_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${LIBJPEG_LIBRARY})
target_link_libraries(${PROJECT_NAME} FastNoise)
target_link_libraries(${PROJECT_NAME} freetype)
target_link_libraries(${PROJECT_NAME} logic)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_package(OpenGL REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
else ()
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FAST_NOISE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${ZLIB_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBPNG_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBJPEG_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${GIF_H_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${LOGIC_DIR})
