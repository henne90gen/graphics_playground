add_library(text Text.cpp)

set_target_properties(text PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(text
        freetype
        opengl
        glm)
target_include_directories(text PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FREETYPE_DIR})
