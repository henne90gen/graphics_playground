
create_scene(WebcamDemo.cpp)
target_include_directories(webcam_demo PRIVATE ${OPENCV_DIR})
target_link_libraries(webcam_demo ${OPENCV_LIBS})
