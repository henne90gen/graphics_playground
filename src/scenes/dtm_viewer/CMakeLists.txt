
create_scene(
    DtmViewer.cpp
    DtmDownloader.cpp
    XyzLoader.cpp
    ShpLoader.cpp
)
target_link_libraries(dtm_viewer zlibstatic zip)
if (NOT EMSCRIPTEN)
    target_link_libraries(dtm_viewer CURL::libcurl)
endif ()
add_scene_resource_directory(local)

create_scene_test(
        ShpLoader.cpp
        ShpLoaderTest.cpp
        XyzLoader.cpp
        XyzLoaderTest.cpp
        DtmDownloader.cpp
        DtmDownloaderTest.cpp
)
if (NOT EMSCRIPTEN)
    target_link_libraries(dtm_viewer_test CURL::libcurl)
endif ()

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
