add_library(image ImageOps.cpp)

set_target_properties(image PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(image
        opengl
        ${LIBPNG_LIBRARY}
        ${LIBJPEG_LIBRARY}
        ${ZLIB_LIBRARY})

target_include_directories(image PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ZLIB_INCLUDE_DIR}
        ${LIBPNG_INCLUDE_DIR}
        ${LIBJPEG_INCLUDE_DIR})
