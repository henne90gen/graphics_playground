add_library(video VideoSaver.cpp)

set_target_properties(video PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(video
        glad
        ${FFMPEG_LIBRARY})

target_include_directories(video PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLAD_DIR}/include
        ${GIF_H_DIR}
        ${FFMPEG_INCLUDE_DIR})
