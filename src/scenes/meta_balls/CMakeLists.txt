
create_scene(MetaBallsScene.cpp MetaBalls.cpp)
target_include_directories(meta_balls PRIVATE ${FAST_NOISE_DIR})
target_link_libraries(meta_balls FastNoise logic)

create_scene_benchmark(MetaBallsBench.cpp MetaBalls.cpp)
target_include_directories(meta_balls_bench PRIVATE ${FAST_NOISE_DIR})
