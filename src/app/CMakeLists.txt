
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

add_executable(${PROJECT_NAME}
        Main.cpp
        util/Image.cpp
        util/ImGuiUtils.cpp
        util/MainMenu.cpp
        util/OpenGLUtils.cpp
        util/RenderUtils.cpp
        util/ScreenRecorder.cpp
        util/VideoSaver.cpp
        opengl/BufferLayout.cpp
        opengl/BufferLayoutElement.cpp
        opengl/IndexBuffer.cpp
        opengl/Model.cpp
        scenes/Scene.cpp
        opengl/Shader.cpp
        opengl/Texture.cpp
        opengl/VertexBuffer.cpp
        opengl/VertexArray.cpp
        scenes/test_scene/TestScene.cpp
        scenes/legacy_triangle/LegacyTriangle.cpp
        scenes/triangle/Triangle.cpp
        scenes/texture_demo/TextureDemo.cpp
        scenes/gamma_calculation/GammaCalculation.cpp
        scenes/cube/Cube.cpp
        scenes/landscape/Landscape.cpp
        scenes/marching_cubes/MarchingCubesScene.cpp
        scenes/font_demo/FontDemo.cpp
        scenes/framebuffer_demo/FramebufferDemo.cpp
        scenes/rubiks_cube/RubiksCubeScene.cpp
        scenes/model_loading/ModelLoading.cpp
        scenes/light_demo/LightDemo.cpp
        scenes/fourier_transform/FourierTransform.cpp
        scenes/normal_mapping/NormalMapping.cpp
        scenes/a_star/AStar.cpp
        scenes/surface_smoothing/SurfaceSmoothing.cpp
        scenes/water_demo/WaterDemo.cpp
        scenes/ray_tracing/RayTracing.cpp
        scenes/shadows_2d/Shadows2D.cpp
        scenes/bloom_effect/BloomEffect.cpp
        scenes/spot_light/SpotLight.cpp
        scenes/meta_balls/MetaBallsScene.cpp
        scenes/terrain_erosion/TerrainErosion.cpp
        scenes/dtm_viewer/DtmViewer.cpp
        scenes/graph_vis/GraphVis.cpp
        scenes/webcam_demo/WebcamDemo.cpp
        ${SHADERS}
        scenes/audio_vis/AudioVis.cpp
        )

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

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif ()

if (CLANG_TIDY_EXE AND RUN_CLANG_TIDY)
    set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(${PROJECT_NAME}
        logic
        glfw
        glad
        imgui
        FastNoise
        freetype
        OpenMP::OpenMP_CXX
        ${GLFW_LIBRARIES}
        ${ZLIB_LIBRARY}
        ${LIBPNG_LIBRARY}
        ${LIBJPEG_LIBRARY}
        ${FFMPEG_LIBRARY}
        ${OPENCV_LIBS}
        ${LIBSOUNDIO_LIBRARY}
        )

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_package(OpenGL REQUIRED)
    target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
elseif (APPLE)
else ()
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE
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
        ${LIBSOUNDIO_INCLUDE_DIR}
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
add_dependencies(${PROJECT_NAME} copy-resources)
