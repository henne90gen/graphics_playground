
create_scene(FontDemo.cpp)
add_scene_resource_directory(fonts)

target_link_libraries(font_demo freetype)
target_include_directories(font_demo PRIVATE ${FREETYPE_DIR})
