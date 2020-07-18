#include <vector>

#include <benchmark/benchmark.h>
#include <functional>
#include <glm/ext.hpp>
#include <iostream>

#include "MetaBalls.h"

static void init_metaballs(std::vector<MetaBall> &metaballs, int count) {
    metaballs.reserve(count);
    for (int i = 0; i < count; i++) {
        metaballs.push_back({glm::vec3(1.0F), 1.0F});
    }
}

static void BM_Exp(benchmark::State &state) {
    std::vector<MetaBall> metaballs = {};
    init_metaballs(metaballs, state.range(0));

    for (auto _ : state) {
        float result = exp_func(metaballs)(glm::vec3(10.0F));
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Exp)->Arg(8)->Arg(64)->Arg(512);

static void BM_InverseDist(benchmark::State &state) {
    std::vector<MetaBall> metaballs = {};
    init_metaballs(metaballs, state.range(0));

    for (auto _ : state) {
        float result = inverse_dist_func(metaballs)(glm::vec3(10.0F));
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_InverseDist)->Arg(8)->Arg(64)->Arg(512);

BENCHMARK_MAIN();
