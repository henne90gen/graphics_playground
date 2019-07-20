#include "Scene.h"

void Scene::renderBackMenu() {
    ImGui::Begin(name.c_str());
    ImGui::SetWindowPos(ImVec2(0, 0));
    const unsigned int windowWidth = 200;
    const unsigned int windowHeight = 100;
    ImGui::SetWindowSize(ImVec2(windowWidth, windowHeight));

    if (ImGui::Button("Take Screenshot")) {
        data.takeScreenshot();
    }
    if (ImGui::Button("Back")) {
        data.backToMainMenu();
    }
    ImGui::End();
}
