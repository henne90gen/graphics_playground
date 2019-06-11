
set(SOURCES
        rubiks_cube/RubiksCubeData.cpp
        rubiks_cube/RubiksCubeLogic.cpp
        rubiks_cube/RubiksCube.cpp
        rubiks_cube/RotationCommandStack.cpp
        marching_cubes/MarchingCubes.cpp
        model_loading/ModelLoader.cpp)

add_library(logic ${SOURCES})
target_link_libraries(logic PRIVATE FastNoise)
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
