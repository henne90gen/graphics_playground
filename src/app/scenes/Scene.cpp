#include "Scene.h"

void Scene::renderBackMenu() {
    ImGui::Begin(name.c_str());
    ImGui::SetWindowPos(ImVec2(0, 0));
    const unsigned int windowWidth = 150;
    const unsigned int windowHeight = 130;
    ImGui::SetWindowSize(ImVec2(windowWidth, windowHeight));

    if (ImGui::Button("Take Screenshot")) {
        data.recorder.takeScreenshot();
    }

    ImGui::RadioButton("GIF", reinterpret_cast<int *>(&data.recorder.recordingType),
                       ScreenRecorder::RecordingType::GIF);
    ImGui::SameLine();
    ImGui::RadioButton("MP4", reinterpret_cast<int *>(&data.recorder.recordingType),
                       ScreenRecorder::RecordingType::MP4);

    if (data.recorder.isRecording()) {
        if (ImGui::Button("Stop Recording")) {
            data.recorder.stopRecording();
        }
    } else {
        if (ImGui::Button("Start Recording")) {
            data.recorder.startRecording();
        }
    }

    if (ImGui::Button("Back")) {
        data.backToMainMenu();
    }
    ImGui::End();
}

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

void Scene::setup(unsigned int windowWidth, unsigned int windowHeight) {
    RECORD_SCOPE();

    setup();

    setDimensions(windowWidth, windowHeight);
}

void Scene::tick(unsigned int windowWidth, unsigned int windowHeight) {
    RECORD_SCOPE();

    setDimensions(windowWidth, windowHeight);

    tick();

    renderBackMenu();
    renderMetrics();
}
