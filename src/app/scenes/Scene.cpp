#include "Scene.h"

void Scene::renderBackMenu() {
    ImGui::Begin(name);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(200, 60));

    if (ImGui::Button("Back")) {
        backToMainMenu();
    }
    ImGui::End();
}
