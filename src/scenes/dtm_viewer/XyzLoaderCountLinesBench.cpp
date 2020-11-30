#include <benchmark/benchmark.h>

#include "XyzLoader.h"

#include "../../bench/XyzLoaderUtil.cpp"

static void BM_CountLines(benchmark::State &state, const unsigned int numFiles) {
    int64_t numLines = state.range(0);
    runWithTestFiles(numFiles, numLines, [&state](const std::string &tmpDir) {
        for (auto _ : state) {
            int result = countLinesInDir(tmpDir);
            benchmark::DoNotOptimize(result);
            benchmark::ClobberMemory();
        }
    });
}

#define BM_COUNT_LINES(numFiles)                                                                                       \
    static void BM_CountLines##numFiles(benchmark::State &state) { BM_CountLines(state, numFiles); }                   \
    BENCHMARK(BM_CountLines##numFiles)->Range(2, 16384);

BM_COUNT_LINES(2)
BM_COUNT_LINES(8)
BM_COUNT_LINES(64)
BM_COUNT_LINES(512)
