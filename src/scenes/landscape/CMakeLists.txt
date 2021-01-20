
create_scene(Landscape.cpp Sky.cpp Trees.cpp TerrainParams.cpp Terrain.cpp)
add_scene_resource_directory(assets)
target_link_libraries(landscape image model)
