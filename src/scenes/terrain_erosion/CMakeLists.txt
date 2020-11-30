
create_scene(TerrainErosion.cpp RainSimulation.cpp)
target_include_directories(terrain_erosion PRIVATE ${FAST_NOISE_DIR})
target_link_libraries(terrain_erosion FastNoise)
