#include <benchmark/benchmark.h>

#include "XyzLoader.h"

#include <iostream>

static void BM_Load(benchmark::State &state) {
    for (auto _ : state) {
        std::vector<glm::vec3> result = {};
        BoundingBox3 bb;
        loadXyzDir("../../../gis_data/dtm", bb, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Load);

BENCHMARK_MAIN();
