
include("../../vendor/catch/contrib/Catch.cmake")
set(CATCH_INCLUDE_DIR ../../vendor/catch/single_include/catch2)

add_executable(Tests main.cpp
        RubiksCubeTest.cpp
        RubiksCubeLogicTest.cpp
        ModelLoaderTest.cpp
        FourierTest.cpp
        RayTracer2DTest.cpp
        gis/ShpLoaderTest.cpp
        gis/XyzLoaderTest.cpp
        QuadTreeTest.cpp)

set_target_properties(Tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_include_directories(Tests PRIVATE
        ${CATCH_INCLUDE_DIR}
        ${LOGIC_DIR}
        ${GLM_DIR}
        ${FFMPEG_INCLUDE_DIR})
target_link_libraries(Tests logic)

catch_discover_tests(Tests)
