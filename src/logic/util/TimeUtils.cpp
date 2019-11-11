#include <utility>

#include "TimeUtils.h"

#include <cmath>
#include <iostream>
#include <utility>

Timer::Timer(std::shared_ptr<PerformanceCounter> performanceTracker, std::string name)
    : name(std::move(name)), performanceTracker(std::move(performanceTracker)) {
    start = std::chrono::high_resolution_clock::now();
    if (performanceTracker) {
        performanceTracker->addTimer(name);
    }
}

Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto endNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(end).time_since_epoch().count();
    auto startNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(start).time_since_epoch().count();
    long long duration = endNs - startNs;
    double ms = duration * 0.001 * 0.001;
    if (performanceTracker) {
        performanceTracker->recordValue(name, ms);
    } else {
        std::cout << name << ": " << ms << "ms" << std::endl;
    }
}

void PerformanceCounter::recordValue(const std::string &name, double value) {
    auto &dp = dataPoints[name];
    dp.timerCount++;

    dp.lastValue = value;

    dp._sum += value;
    dp.average = dp._sum / dp.timerCount;

    dp._sdSum += (value - dp.average) * (value - dp.average);
    dp.standardDeviation = sqrt(dp._sdSum / dp.timerCount);
}

void PerformanceCounter::reset() {
    for (auto &dataPoint : dataPoints) {
        dataPoint.second = {};
    }
}
