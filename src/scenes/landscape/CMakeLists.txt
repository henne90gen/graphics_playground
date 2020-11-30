
create_scene(Landscape.cpp)
target_include_directories(landscape PRIVATE ${FAST_NOISE_DIR})
target_link_libraries(landscape FastNoise)
