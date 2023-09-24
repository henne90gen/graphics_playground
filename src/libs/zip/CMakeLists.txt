add_library(zip zip.cpp)

set_target_properties(zip PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(zip ${ZLIB_LIBRARY})

target_include_directories(zip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (EMSCRIPTEN)
    return()
endif ()

include(GoogleTest)

add_executable(zip_test zip_test.cpp)
target_link_libraries(zip_test GTest::gtest_main zip)
gtest_discover_tests(zip_test)
set_target_properties(zip_test PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)
