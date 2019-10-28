#include "Scene.h"

void Scene::renderBackMenu() {
    ImGui::Begin(name.c_str());
    ImGui::SetWindowPos(ImVec2(0, 0));
    const unsigned int windowWidth = 330;
    const unsigned int windowHeight = 130;
    ImGui::SetWindowSize(ImVec2(windowWidth, windowHeight));

    if (ImGui::Button("Take Screenshot")) {
        data.recorder.takeScreenshot();
    }

    static const std::array<const char *, 2> items = {"GIF", "MP4"};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::Combo("Recording Type", reinterpret_cast<int *>(&data.recorder.recordingType), items.data(), items.size());

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
