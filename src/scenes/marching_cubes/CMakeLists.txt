
create_scene(MarchingCubesScene.cpp)
target_include_directories(marching_cubes PRIVATE ${FAST_NOISE_DIR})
target_link_libraries(marching_cubes FastNoise logic)
