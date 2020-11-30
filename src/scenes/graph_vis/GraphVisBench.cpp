#include <benchmark/benchmark.h>
#include <vector>

#include "Graph.h"

static void BM_Spring(benchmark::State &state) {
    auto nodes = std::vector<GraphNode>(1024);
    for (unsigned int i = 0; i < nodes.size(); i++) {
        nodes[i] = {{i, i}, {1.0F, 1.0F, 1.0F}};
    }

    auto edges = std::vector<GraphEdge>(state.range(0));
    for (unsigned int i = 0; i < edges.size(); i++) {
        edges[i] = {static_cast<unsigned int>(i % nodes.size()), static_cast<unsigned int>((i + 1) % nodes.size())};
    }

    for (auto _ : state) {
        updateSpringAcceleration(edges, nodes, 0.001F, 0.1F, 0.1F, 5.0F);
        benchmark::DoNotOptimize(nodes);
    }
}
BENCHMARK(BM_Spring)->Range(8, 1024);

static void BM_Charge(benchmark::State &state) {
    auto nodes = std::vector<GraphNode>(state.range(0));
    for (unsigned int i = 0; i < nodes.size(); i++) {
        nodes[i] = {{i, i}, {1.0F, 1.0F, 1.0F}};
    }

    auto edges = std::vector<GraphEdge>(1024);
    for (unsigned int i = 0; i < edges.size(); i++) {
        edges[i] = {static_cast<unsigned int>(i % nodes.size()), static_cast<unsigned int>((i + 1) % nodes.size())};
    }

    for (auto _ : state) {
        updateChargeAcceleration(nodes, 5.0F);
        benchmark::DoNotOptimize(nodes);
    }
}
BENCHMARK(BM_Charge)->Range(8, 1024);

BENCHMARK_MAIN();
