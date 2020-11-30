
create_scene(MetaBallsScene.cpp MetaBalls.cpp)
target_include_directories(meta_balls PRIVATE ${FAST_NOISE_DIR})

create_scene_benchmark(MetaBallsBench.cpp MetaBalls.cpp)
target_include_directories(meta_balls_bench PRIVATE ${FAST_NOISE_DIR})
