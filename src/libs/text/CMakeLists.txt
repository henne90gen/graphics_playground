add_library(text Text.cpp)

set_target_properties(text PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(text
        freetype
        opengl)
target_include_directories(text PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FREETYPE_DIR}
        ${GLM_DIR})
