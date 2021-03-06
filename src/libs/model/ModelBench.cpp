#include <benchmark/benchmark.h>
#include <glm/glm.hpp>
#include <vector>

#include "Model.h"

static void BM_LoadMonkey(benchmark::State &state) {
    for (auto _ : state) {
        auto rawModel = RawModel();
        unsigned int error = Model::loadRawModelFromFile("../../src/libs/model/models/monkey.obj", rawModel);
        benchmark::DoNotOptimize(rawModel);
        benchmark::DoNotOptimize(error);
    }
}
BENCHMARK(BM_LoadMonkey);

BENCHMARK_MAIN();
