
create_scene(LightDemo.cpp)
add_scene_resource_directory(models)

target_link_libraries(light_demo
        image
        model)
