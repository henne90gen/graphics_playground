add_library(marching_cubes_lib MarchingCubes.cpp)

set_target_properties(marching_cubes_lib PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(marching_cubes_lib glm FastNoise warnings)
target_include_directories(marching_cubes_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
