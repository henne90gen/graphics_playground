
create_scene(AudioVis.cpp WavLoader.cpp)
add_scene_resource_directory(res)
target_include_directories(audio_vis PRIVATE ${LIBSOUNDIO_INCLUDE_DIR} ${LOGIC_DIR})
target_link_libraries(audio_vis ${LIBSOUNDIO_LIBRARY} logic)
