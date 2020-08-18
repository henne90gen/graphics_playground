#include <benchmark/benchmark.h>

#include "XyzLoader.h"

static void BM_Load(benchmark::State &state) {
    for (auto _ : state) {
        std::vector<glm::vec3> result = {};
        BoundingBox3 bb;
        loadXyzDir("../../../src/test/gis/dtm", bb, result);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Load);

static void BM_CountLines(benchmark::State &state) {
    for (auto _ : state) {
        int result = countLinesInDir("../../../src/test/gis/dtm");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CountLines);

static void BM_CountLinesReal(benchmark::State &state) {
    for (auto _ : state) {
        int result = countLinesInDir("../../../gis_data/dtm");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CountLinesReal);

BENCHMARK_MAIN();
