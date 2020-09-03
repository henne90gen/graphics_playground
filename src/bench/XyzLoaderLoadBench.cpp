#include <benchmark/benchmark.h>

#include "gis/XyzLoader.h"
#include "util/BoundingBox.h"

#include "XyzLoaderUtil.cpp"

static void BM_Load(benchmark::State &state, const unsigned int numFiles) {
    int64_t numLines = state.range(0);
    runWithTestFiles(numFiles, numLines, [&state](const std::string &tmpDir) {
        for (auto _ : state) {
            std::vector<glm::vec3> result = {};
            BoundingBox3 bb;
            loadXyzDir(tmpDir, bb, result);
            benchmark::DoNotOptimize(result);
        }
    });
}

#define BM_LOAD(numFiles)                                                                                              \
    static void BM_Load##numFiles(benchmark::State &state) { BM_Load(state, numFiles); }

BM_LOAD(2)
BM_LOAD(8)
BM_LOAD(64)
BM_LOAD(512)
BENCHMARK(BM_Load2)->Range(2, 16384);
BENCHMARK(BM_Load8)->Range(2, 16384);
BENCHMARK(BM_Load64)->Range(2, 16384);
BENCHMARK(BM_Load512)->Range(2, 16384);

BENCHMARK_MAIN();
