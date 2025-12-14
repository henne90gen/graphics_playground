add_library(screen_recording ScreenRecorder.cpp)

set_target_properties(screen_recording PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(screen_recording PUBLIC image opengl video glm warnings)

target_include_directories(screen_recording PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
