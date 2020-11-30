
add_library(logic
        util/FileUtils.cpp
        util/TimeUtils.cpp
        util/DataReadUtils.cpp
        rubiks_cube/RubiksCubeData.cpp
        rubiks_cube/RubiksCubeLogic.cpp
        rubiks_cube/RubiksCube.cpp
        rubiks_cube/RotationCommandStack.cpp
        marching_cubes/MarchingCubes.cpp
        model_loading/ModelLoader.cpp
        fourier_transform/Fourier.cpp
        ray_tracing_2d/RayTracer2D.cpp
        terrain_erosion/RainSimulation.cpp
        gis/ShpLoader.cpp
        gis/XyzLoader.cpp
        )

target_link_libraries(logic PRIVATE FastNoise OpenMP::OpenMP_CXX)

set_target_properties(
        logic PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
elseif (APPLE)
else ()
    target_link_libraries(logic PRIVATE stdc++fs)
endif ()

target_include_directories(logic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_DIR}
        ${FAST_NOISE_DIR})

if (CLANG_TIDY_EXE AND RUN_CLANG_TIDY)
    set_target_properties(
            logic PROPERTIES
            CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif ()

set(LOGIC_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
