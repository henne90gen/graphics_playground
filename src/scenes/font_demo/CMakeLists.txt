
create_scene(font_demo FontDemo.cpp)
add_scene_resource_directory(font_demo fonts)

target_link_libraries(font_demo freetype)
target_include_directories(font_demo PRIVATE ${FREETYPE_DIR})
