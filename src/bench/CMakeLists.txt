
function(benchmark name)
    add_executable(${name}_bench ${name}.cpp)
    target_link_libraries(${name}_bench PRIVATE core benchmark::benchmark)
    target_include_directories(${name}_bench PRIVATE
            ${CMAKE_SOURCE_DIR}/src/core
            ${GLM_DIR})
    set_target_properties(
            ${name}_bench PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
endfunction()

benchmark(MarchingCubesBench)
target_include_directories(MarchingCubesBench_bench PRIVATE ${FAST_NOISE_DIR})
target_link_libraries(MarchingCubesBench_bench PRIVATE marching_cubes_lib)

benchmark(QuadTreeBench)
