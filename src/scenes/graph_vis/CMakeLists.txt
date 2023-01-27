
create_scene(GraphVis.cpp Graph.cpp)

if (NOT EMSCRIPTEN)
    create_scene_benchmark(GraphVisBench.cpp Graph.cpp)
endif ()
