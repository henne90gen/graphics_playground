add_library(screen_recording ScreenRecorder.cpp)

set_target_properties(screen_recording PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(screen_recording image opengl video)

target_include_directories(screen_recording PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_DIR})
