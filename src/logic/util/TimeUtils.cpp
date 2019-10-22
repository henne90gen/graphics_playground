#include <utility>

#include "TimeUtils.h"

#include <utility>
#include <iostream>
#include <cmath>

Timer::Timer(std::shared_ptr<PerformanceCounter> performanceTracker, std::string name) :
        name(std::move(name)),
        performanceTracker(std::move(performanceTracker)) {
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

    dp.sum += value;
    dp.average = dp.sum / dp.timerCount;

    dp.sdSum += (value - dp.average) * (value - dp.average);
    dp.standardDeviation = sqrt(dp.sdSum / dp.timerCount);
}
