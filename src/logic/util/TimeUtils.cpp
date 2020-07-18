#include "TimeUtils.h"

#include <cmath>
#include <iostream>
#include <utility>

#define NANOSECONDS(timePoint)                                                                                         \
    std::chrono::time_point_cast<std::chrono::nanoseconds>(timePoint).time_since_epoch().count()

Timer::Timer(PerformanceCounter *performanceTracker, std::string name)
    : name(std::move(name)), performanceTracker(performanceTracker) {
    start = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto endNs = NANOSECONDS(end);
    auto startNs = NANOSECONDS(start);
    if (performanceTracker != nullptr) {
        performanceTracker->recordValue(name, startNs, endNs);
    } else {
        long long duration = endNs - startNs;
        const double conversionFactor = 0.001;
        double ms = static_cast<double>(duration) * conversionFactor * conversionFactor;
        std::cout << name << ": " << ms << "ms" << std::endl;
    }
}

PerformanceCounter::PerformanceCounter() {
#if PROFILING
    programStart = std::chrono::high_resolution_clock::now();
    fileOutput = std::ofstream("results.json", std::ofstream::out);
    fileOutput << "[";
#endif
}

PerformanceCounter::~PerformanceCounter() {
#if PROFILING
    fileOutput << "]";
    fileOutput.flush();
    fileOutput.close();
#endif
}

void PerformanceCounter::recordValue(const std::string &name, long long start, long long end) {
    if (dataPoints.find(name) == dataPoints.end()) {
        dataPoints[name] = {};
    }
    double valueMicro = static_cast<double>(end - start) * 0.001;
    double value = valueMicro * 0.001;

    auto &dp = dataPoints[name];
    dp.timerCount++;

    dp.lastValue = value;

    dp._sum += value;
    dp.average = dp._sum / dp.timerCount;

    dp._sdSum += (value - dp.average) * (value - dp.average);
    dp.standardDeviation = sqrt(dp._sdSum / dp.timerCount);

#if PROFILING
    unsigned int processId = 0;
    unsigned int threadId = 0;
    double startMicro = static_cast<double>(start - NANOSECONDS(programStart)) * 0.001;
    if (hasWrittenValuesToFile) {
        fileOutput << ",";
    }
    fileOutput << "{";
    fileOutput << R"("name": ")" << name << "\",";
    fileOutput << R"("cat": "",)";
    fileOutput << R"("ph": "X",)";
    fileOutput << R"("ts": )" << startMicro << ",";
    fileOutput << R"("dur": )" << valueMicro << ",";
    fileOutput << R"("pid": )" << processId << ",";
    fileOutput << R"("tid": )" << threadId << ",";
    fileOutput << R"("args": {})";
    fileOutput << "}";
    fileOutput.flush();

    hasWrittenValuesToFile = true;
#endif
}

void PerformanceCounter::reset() {
    for (auto &dataPoint : dataPoints) {
        dataPoint.second = {};
    }
}
