
create_scene(AudioVis.cpp)
target_include_directories(audio_vis PRIVATE ${LIBSOUNDIO_INCLUDE_DIR})
target_link_libraries(audio_vis ${LIBSOUNDIO_LIBRARY})
