
include("../../vendor/catch/contrib/Catch.cmake")
set(CATCH_INCLUDE_DIR ../../vendor/catch/single_include/catch2)

set(SOURCES
        main.cpp
        RubiksCubeTest.cpp
        RubiksCubeLogicTest.cpp
        ModelLoaderTest.cpp
        FourierTest.cpp
        AStarTest.cpp RayTracer2DTest.cpp)

add_executable(Tests ${SOURCES})

target_include_directories(Tests PRIVATE
        ${CATCH_INCLUDE_DIR}
        ${LOGIC_DIR}
        ${GLM_DIR})
target_link_libraries(Tests logic)

catch_discover_tests(Tests)
