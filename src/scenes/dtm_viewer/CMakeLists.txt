
create_scene(
    DtmViewer.cpp
    DtmDownloader.cpp
    XyzLoader.cpp
    ShpLoader.cpp
)
target_link_libraries(dtm_viewer ${ZLIB_LIBRARY} CURL::libcurl zip)
add_scene_resource_directory(local)

create_scene_test(
        ShpLoader.cpp
        ShpLoaderTest.cpp
        XyzLoader.cpp
        XyzLoaderTest.cpp
        DtmDownloader.cpp
        DtmDownloaderTest.cpp
)
target_link_libraries(dtm_viewer_test CURL::libcurl)

if (NOT EMSCRIPTEN)
    create_scene_benchmark(
            BenchMain.cpp
            XyzLoaderCountLinesBench.cpp
            XyzLoaderLoadBench.cpp
            XyzLoader.cpp
            ShpLoader.cpp
    )

    if (NOT MSVC)
        target_link_libraries(dtm_viewer_bench PRIVATE stdc++fs)
    endif ()
endif ()
