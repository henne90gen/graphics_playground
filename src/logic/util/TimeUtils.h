#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

struct DataPoint {
    double lastValue;
    double average;
    double standardDeviation;
    double timerCount;
    double sum;
    double sdSum;
};

/**
 * NOTE: All times are in milliseconds.
 */
class PerformanceCounter {
public:
  PerformanceCounter() = default;

    ~PerformanceCounter() = default;

    void recordValue(const std::string &name, double value);

    void reset() {
        dataPoints = {};
    }

    void addTimer(const std::string &name) {
        dataPoints[name] = {};
    }

    std::unordered_map<std::string, DataPoint> dataPoints = {};
};

class Timer {
public:
    explicit Timer(std::shared_ptr<PerformanceCounter> performanceTracker, std::string name);

    ~Timer();

private:

    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::shared_ptr<PerformanceCounter> performanceTracker;
};


#define TIME_SCOPE() \
    auto timer = Timer(nullptr, __func__)

#define TIME_SCOPE_NAME(name) \
    auto timer = Timer(nullptr, name)

#define TIME_SCOPE_RECORD(perfCounter) \
    auto timer = Timer(perfCounter, __func__)

#define TIME_SCOPE_RECORD_NAME(perfCounter, name) \
    auto timer = Timer(perfCounter, name)
