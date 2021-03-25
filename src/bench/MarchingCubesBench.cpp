#include <benchmark/benchmark.h>
#include <glm/glm.hpp>
#include <vector>

#include "MarchingCubes.h"

static void BM_MarchingCubes(benchmark::State &state) {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::ivec3> indices = {};

    implicit_surface_func func = [](const glm::vec3 &position) {
        return position.x * position.x + position.y * position.y + position.z * position.z - 4;
    };
    const glm::ivec3 dimensions = {
          10, // state.range(0),
          10, // state.range(0),
          state.range(0),
    };
    for (auto _ : state) {
        runMarchingCubes(dimensions, vertices, indices, func);
        benchmark::DoNotOptimize(vertices);
        benchmark::DoNotOptimize(indices);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_MarchingCubes)->Range(4, 512);

static void BM_MarchingCubesSequential(benchmark::State &state) {
    std::vector<glm::vec3> vertices = {};
    std::vector<glm::ivec3> indices = {};

    implicit_surface_func func = [](const glm::vec3 &position) {
        return position.x * position.x + position.y * position.y + position.z * position.z - 4;
    };
    const glm::ivec3 dimensions = {
          10, // state.range(0),
          10, // state.range(0),
          state.range(0),
    };
    for (auto _ : state) {
        runMarchingCubesSequential(dimensions, vertices, indices, func);
        benchmark::DoNotOptimize(vertices);
        benchmark::DoNotOptimize(indices);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_MarchingCubesSequential)->Range(4, 512);

BENCHMARK_MAIN();
