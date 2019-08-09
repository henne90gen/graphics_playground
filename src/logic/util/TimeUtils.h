#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

struct DataPoint {
    double average;
    double standardDeviation;
    double timerCount;
    double sum;
    double sdSum;
};

/**
 * NOTE: All times are in milliseconds.
 */
class PerformanceTracker {
public:
    PerformanceTracker() = default;

    ~PerformanceTracker() = default;

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
    explicit Timer(std::shared_ptr<PerformanceTracker> performanceTracker, std::string name);

    ~Timer();

private:

    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::shared_ptr<PerformanceTracker> performanceTracker;
};


#define TIME_SCOPE() \
    auto timer = Timer(nullptr, __func__)

#define TIME_SCOPE_RECORD(PerfCounter) \
    auto timer = Timer(PerfCounter, __func__)

#define TIME_SCOPE_RECORD_NAME(PerfCounter, Name) \
    auto timer = Timer(PerfCounter, Name)
