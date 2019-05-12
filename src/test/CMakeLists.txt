
include("../../catch/contrib/Catch.cmake")

add_executable(RubiksCubeTest
        main.cpp
        RubiksCubeTest.cpp)

target_include_directories(RubiksCubeTest PRIVATE
        ../../catch/single_include/catch2
        ${LOGIC_DIR}
        ${GLM_DIR})
target_link_libraries(RubiksCubeTest logic)
