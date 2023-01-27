
create_scene(AudioVis.cpp WavLoader.cpp)
add_scene_resource_directory(res)
target_link_libraries(audio_vis libsoundio_shared)
