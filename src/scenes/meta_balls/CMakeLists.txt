
create_scene(MetaBallsScene.cpp MetaBalls.cpp)
target_link_libraries(meta_balls marching_cubes_lib)
target_include_directories(meta_balls PRIVATE ${FAST_NOISE_DIR})

if (NOT EMSCRIPTEN)
    create_scene_benchmark(MetaBallsBench.cpp MetaBalls.cpp)
    target_link_libraries(meta_balls_bench PRIVATE marching_cubes_lib)
    target_include_directories(meta_balls_bench PRIVATE ${FAST_NOISE_DIR})
endif ()
