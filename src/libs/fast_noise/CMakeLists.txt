add_library(fast_noise ImGuiUtilsFastNoise.cpp)

set_target_properties(fast_noise PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(fast_noise FastNoise)

target_include_directories(fast_noise PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FAST_NOISE_DIR})
