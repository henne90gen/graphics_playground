#include "Scene.h"

void Scene::renderMetrics() {
    ImGui::Begin("Metrics");

    for (auto &dataPoint : performanceCounter.dataPoints) {
        ImGui::Text("%.3fms - Last Time - %s", dataPoint.second.lastValue, dataPoint.first.c_str());
        ImGui::Text("%.3fms - Average Time - %s", dataPoint.second.average, dataPoint.first.c_str());
        ImGui::Text("%.3fms - Standard Deviation - %s", dataPoint.second.standardDeviation, dataPoint.first.c_str());
    }

    if (ImGui::Button("Reset Performance Counter")) {
        performanceCounter.reset();
    }

    ImGui::End();
}

void Scene::setup(unsigned int windowWidth, unsigned int windowHeight, SceneData sceneData) {
    RECORD_SCOPE();

    this->data = sceneData;
    setDimensions(windowWidth, windowHeight);
    setup();
}

void Scene::tick(unsigned int windowWidth, unsigned int windowHeight) {
    RECORD_SCOPE();

    setDimensions(windowWidth, windowHeight);

    auto currentTime = std::chrono::high_resolution_clock::now();
    int64_t currentTimeNs =
          std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTime).time_since_epoch().count();
    timeDelta = static_cast<double>(currentTimeNs - lastTimeNs) / 1000000000.0;
    lastTimeNs = currentTimeNs;

    tick();

    renderMetrics();
}
