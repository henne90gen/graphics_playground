
set(SOURCES
        util/FileUtils.cpp
        util/TimeUtils.cpp
        rubiks_cube/RubiksCubeData.cpp
        rubiks_cube/RubiksCubeLogic.cpp
        rubiks_cube/RubiksCube.cpp
        rubiks_cube/RotationCommandStack.cpp
        marching_cubes/MarchingCubes.cpp
        meta_balls/MetaBalls.cpp
        model_loading/ModelLoader.cpp
        fourier_transform/Fourier.cpp
        a_star/AStarSolver.cpp
        ray_tracing_2d/RayTracer2D.cpp
        ray_tracing/RayTracer.cpp)

add_library(logic ${SOURCES})
target_link_libraries(logic PRIVATE FastNoise pthread OpenMP::OpenMP_CXX)

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


# MarchinCubes Benchmark
add_executable(MarchingCubesBench marching_cubes/MarchingCubesBench.cpp marching_cubes/MarchingCubes.cpp)
target_link_libraries(MarchingCubesBench PRIVATE FastNoise benchmark::benchmark OpenMP::OpenMP_CXX)
target_include_directories(MarchingCubesBench PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_DIR}
        ${FAST_NOISE_DIR})
set_target_properties(
        MarchingCubesBench PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

# MetaBalls Benchmark
add_executable(MetaBallsBench meta_balls/MetaBallsBench.cpp meta_balls/MetaBalls.cpp)
target_link_libraries(MetaBallsBench PRIVATE FastNoise benchmark::benchmark)
target_include_directories(MetaBallsBench PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_DIR}
        ${FAST_NOISE_DIR})
set_target_properties(
        MetaBallsBench PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
