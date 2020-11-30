
function(benchmark name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE logic benchmark::benchmark OpenMP::OpenMP_CXX)
    target_include_directories(${name} PRIVATE
            ${LOGIC_DIR}
            ${GLM_DIR})
    set_target_properties(
            ${name} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
endfunction()

benchmark(MarchingCubesBench)
target_include_directories(MarchingCubesBench PRIVATE ${FAST_NOISE_DIR})

benchmark(XyzLoaderLoadBench)
benchmark(XyzLoaderCountLinesBench)

benchmark(QuadTreeBench)
