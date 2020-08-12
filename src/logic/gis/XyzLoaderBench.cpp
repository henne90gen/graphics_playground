#include <benchmark/benchmark.h>

#include "XyzLoader.h"

static void BM_Load(benchmark::State &state) {
    for (auto _ : state) {
        std::vector<glm::vec3> result = {};
        loadXyzDir("../../../gis_data/dtm", result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Load);

BENCHMARK_MAIN();
