#include "Scene.h"

void Scene::renderBackMenu() {
    ImGui::Begin(name.c_str());
    ImGui::SetWindowPos(ImVec2(0, 0));
    const unsigned int windowWidth = 250;
    const unsigned int windowHeight = 100;
    ImGui::SetWindowSize(ImVec2(windowWidth, windowHeight));

    if (ImGui::Button("Take Screenshot")) {
        data.recorder.takeScreenshot();
    }
    if (data.recorder.isRecording()) {
        if (ImGui::Button("Stop Recording")) {
            data.recorder.stopRecording();
        }
    } else {
        if (ImGui::Button("Start Recording")) {
            data.recorder.startRecording();
        }
    }
    ImGui::SameLine();
    ImGui::RadioButton("GIF", (int *) &data.recorder.recordingType, ScreenRecorder::RecordingType::GIF);
    ImGui::SameLine();
    ImGui::RadioButton("PNG", (int *) &data.recorder.recordingType, ScreenRecorder::RecordingType::PNG);

    if (ImGui::Button("Back")) {
        data.backToMainMenu();
    }
    ImGui::End();
}
