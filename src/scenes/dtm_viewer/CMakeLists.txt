
create_scene(DtmViewer.cpp XyzLoader.cpp ShpLoader.cpp)

# TODO make scene test work again
#create_scene_test(
#        TestMain.cpp
#        ShpLoaderTest.cpp
#        XyzLoaderTest.cpp
#        XyzLoader.cpp
#        ShpLoader.cpp
#)

create_scene_benchmark(
        BenchMain.cpp
        XyzLoaderCountLinesBench.cpp
        XyzLoaderLoadBench.cpp
        XyzLoader.cpp
        ShpLoader.cpp
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
elseif (APPLE)
else ()
    target_link_libraries(dtm_viewer_bench PRIVATE stdc++fs)
endif ()
