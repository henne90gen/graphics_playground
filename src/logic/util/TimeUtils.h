#pragma once

#include <chrono>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>

#define PROFILING 1

struct DataPoint {
    double lastValue = 0.0;
    double average = 0.0;
    double standardDeviation = 0.0;
    double _sum = 0.0;
    double _sdSum = 0.0;
    unsigned int timerCount = 0;
};

/**
 * NOTE: All times are in milliseconds.
 */
class PerformanceCounter {
  public:
    PerformanceCounter();
    ~PerformanceCounter();

    void recordValue(const std::string &name, long long start, long long end);

    void reset();

    std::unordered_map<std::string, DataPoint> dataPoints = {};

  private:
#if PROFILING
    std::ofstream fileOutput = {};
    bool hasWrittenValuesToFile = false;
    std::chrono::time_point<std::chrono::high_resolution_clock> programStart;
#endif
};

class Timer {
  public:
    explicit Timer(PerformanceCounter *performanceTracker, std::string name);

    ~Timer();

  private:
    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    PerformanceCounter *performanceTracker;
};

#ifdef __FUNCSIG__
#define FUNCTION_NAME __FUNCSIG__
#else
#define FUNCTION_NAME __PRETTY_FUNCTION__
#endif

#define COMBINE(X, Y) X##Y

#define TIME_SCOPE() auto COMBINE(timer, __LINE__) = Timer(nullptr, FUNCTION_NAME)
#define TIME_SCOPE_NAME(name) auto COMBINE(timer, __LINE__) = Timer(nullptr, name)

#define RECORD_SCOPE() auto COMBINE(timer, __LINE__) = Timer(getPerformanceCounter(), FUNCTION_NAME)
#define RECORD_SCOPE_NAME(name) auto COMBINE(timer, __LINE__) = Timer(getPerformanceCounter(), name)
