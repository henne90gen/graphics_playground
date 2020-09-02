#include <benchmark/benchmark.h>

#include "quad_tree/QuadTree.h"

constexpr unsigned int start = 16;
constexpr unsigned int end = 65536 * 8;

template <unsigned int K> QuadTree<unsigned int, K> createTree(const unsigned int size) {
    QuadTree<unsigned int, K> tree = {};
    std::vector<std::pair<glm::vec3, unsigned int>> elements = {};
    const unsigned int dimension = std::sqrt(size);
    for (unsigned int i = 0; i < size; i++) {
        unsigned int x = i % dimension;
        unsigned int z = i / dimension;
        elements.push_back(std::make_pair(glm::vec3(x, 0, z), x * dimension + z));
    }
    tree.insert(elements);
    return tree;
}

static void K8(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<8>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        auto result = tree.get(query);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(K8)->Range(start, end);

static void K16(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<16>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        auto result = tree.get(query);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(K16)->Range(start, end);

static void K32(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<32>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        auto result = tree.get(query);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(K32)->Range(start, end);

static void K64(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<64>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        auto result = tree.get(query);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(K64)->Range(start, end);

static void K128(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<128>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        auto result = tree.get(query);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(K128)->Range(start, end);

static void K256(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<256>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        auto result = tree.get(query);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(K256)->Range(start, end);

static void K512(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<512>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        auto result = tree.get(query);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(K512)->Range(start, end);

BENCHMARK_MAIN();
