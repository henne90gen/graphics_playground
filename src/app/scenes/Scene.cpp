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
