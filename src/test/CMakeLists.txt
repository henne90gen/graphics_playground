
include("../../catch/contrib/Catch.cmake")

add_executable(Tests
        main.cpp
        RubiksCubeTest.cpp)

target_include_directories(Tests PRIVATE
        ../../catch/single_include/catch2
        ${LOGIC_DIR}
        ${GLM_DIR})
target_link_libraries(Tests logic)

catch_discover_tests(Tests)
