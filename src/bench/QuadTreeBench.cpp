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

template <unsigned int K> void runBenchmark(benchmark::State &state) {
    const unsigned int size = state.range(0);
    auto tree = createTree<K>(size);

    for (auto _ : state) {
        glm::vec3 query = {0, 0, 0};
        unsigned int result = 0;
        tree.get(query, result);
        benchmark::DoNotOptimize(result);
    }
}

#define K(num)                                                                                                         \
    static void K##num(benchmark::State &state) { runBenchmark<num>(state); }                                          \
    BENCHMARK(K##num)->Range(start, end);

K(8)
K(16)
K(32)
K(64)
K(128)
K(256)
K(512)

BENCHMARK_MAIN();
