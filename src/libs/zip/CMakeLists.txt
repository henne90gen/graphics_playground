add_library(zip zip.cpp)

set_target_properties(zip PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(zip zlibstatic)

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

# copying test resources to two locations
#  - {root}/build/bin/zip_test_resources (working directory for direct execution from the IDE)
#  - {root}/build/src/libs/zip/zip_test_resources (working directory for ctest/ninja test)
add_custom_target(zip_test_copy_resources
        COMMAND cd ${CMAKE_SOURCE_DIR} && python scripts/copy_resources.py ${CMAKE_CURRENT_SOURCE_DIR} resources ${CMAKE_BINARY_DIR}/bin/zip_test_resources && python scripts/copy_resources.py ${CMAKE_CURRENT_SOURCE_DIR} resources ${CMAKE_CURRENT_BINARY_DIR}/zip_test_resources)
add_dependencies(zip_test zip_test_copy_resources)
