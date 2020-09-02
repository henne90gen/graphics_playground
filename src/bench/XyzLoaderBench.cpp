#include <benchmark/benchmark.h>
#include <filesystem>
#include <fstream>

#include "gis/XyzLoader.h"
#include "util/BoundingBox.h"

static void runWithTestFiles(unsigned int numFiles, unsigned int numLines,
                             const std::function<void(const std::string &)> &func) {
    const std::string tmpDir = "dtm-" + std::to_string(numFiles) + "-" + std::to_string(numLines);
    if (!std::filesystem::exists(tmpDir)) {
        std::filesystem::create_directory(tmpDir);
    }
    for (unsigned int i = 0; i < numFiles; i++) {
        std::ofstream file;
        file.open(tmpDir + "/f" + std::to_string(i) + ".xyz");
        file << i * 1.0F << i * 1000.0F << i * 1000000.0F << "\n";
        file.close();
    }

    func(tmpDir);

    std::filesystem::remove_all(tmpDir);
}

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

static void BM_CountLines(benchmark::State &state, const unsigned int numFiles) {
    int64_t numLines = state.range(0);
    runWithTestFiles(numFiles, numLines, [&state](const std::string &tmpDir) {
        for (auto _ : state) {
            int result = countLinesInDir(tmpDir);
            benchmark::DoNotOptimize(result);
        }
    });
}

#define BM_COUNT_LINES(numFiles)                                                                                       \
    static void BM_CountLines##numFiles(benchmark::State &state) { BM_CountLines(state, numFiles); }

BM_COUNT_LINES(2)
BM_COUNT_LINES(8)
BM_COUNT_LINES(64)
BM_COUNT_LINES(512)
BENCHMARK(BM_CountLines2)->Range(2, 16384);
BENCHMARK(BM_CountLines8)->Range(2, 16384);
BENCHMARK(BM_CountLines64)->Range(2, 16384);
BENCHMARK(BM_CountLines512)->Range(2, 16384);

BENCHMARK_MAIN();
