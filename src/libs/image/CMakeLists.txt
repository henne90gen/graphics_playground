add_library(image ImageOps.cpp)

set_target_properties(image PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(image
        opengl
        jpeg
        png_static
        zlibstatic)

target_include_directories(image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
