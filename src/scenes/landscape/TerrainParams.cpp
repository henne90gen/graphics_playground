#include "TerrainParams.h"

#include <array>
#include <imgui.h>
#include <random>

TerrainParams::TerrainParams() {
    noiseLayers = {
          NoiseLayer(450.0F, 20.0F), //
          NoiseLayer(300.0F, 15.0F), //
          NoiseLayer(200.0F, 10.0F), //
          NoiseLayer(150.0F, 7.5F),  //
          NoiseLayer(100.0F, 5.0F),  //
          NoiseLayer(80.0F, 4.0F),   //
          NoiseLayer(30.0F, 2.0F),   //
          NoiseLayer(7.5F, 0.75F),   //
    };
}

void TerrainParams::showGui() {
    ImGui::Checkbox("Use Finite Differences", &useFiniteDifferences);
    ImGui::DragFloat("Finite Difference", &finiteDifference, 0.001F);
    ImGui::DragFloat("Power", &power, 0.001F);
    ImGui::DragFloat("Bowl Strength", &bowlStrength, 0.1F);
    ImGui::SliderFloat("Platform Height", &platformHeight, 0.0F, 1.0F);
    ImGui::DragInt("Seed", &seed);
    if (ImGui::Button("New Seed")) {
        std::random_device rand_dev;
        std::mt19937 generator(rand_dev());
        std::uniform_int_distribution<int> distr(0, 1000000);
        seed = distr(generator);
    }
}

void TerrainParams::showLayersGui() {
    ImGui::Begin("Layers");
    int layerToRemove = -1;
    for (int i = 0; i < static_cast<int>(noiseLayers.size()); i++) {
        ImGui::Separator();

        const auto btnLabel = "Remove Layer " + std::to_string(i);
        if (ImGui::Button(btnLabel.c_str())) {
            layerToRemove = i;
        }

        ImGui::SameLine();
        const auto enabledLabel = "Enabled " + std::to_string(i);
        ImGui::Checkbox(enabledLabel.c_str(), &noiseLayers[i].enabled);

        const auto frequencyLabel = "Frequency " + std::to_string(i);
        ImGui::SliderFloat(frequencyLabel.c_str(), &noiseLayers[i].frequency, 0.0F, 500.0F);

        const auto amplitudeLabel = "Amplitude " + std::to_string(i);
        ImGui::SliderFloat(amplitudeLabel.c_str(), &noiseLayers[i].amplitude, 0.0F, 100.0F);
    }
    if (layerToRemove >= 0) {
        noiseLayers.erase(noiseLayers.begin() + layerToRemove);
    }

    ImGui::Separator();
    static int layerType = 0;
    static const std::array<const char *, 2> items = {"Noise", "Ridge"};
    ImGui::Combo("", &layerType, items.data(), items.size());
    ImGui::SameLine();
    if (ImGui::Button("Add Layer")) {
        switch (layerType) {
        case 0:
            noiseLayers.emplace_back();
            break;
        default:
            std::cout << "Invalid layer type" << std::endl;
            break;
        }
    }
    ImGui::End();
}

void TerrainParams::setShaderUniforms(const std::shared_ptr<Shader> &shader) const {
    for (int i = 0; i < static_cast<int64_t>(noiseLayers.size()); i++) {
        shader->setUniform("noiseLayers[" + std::to_string(i) + "].amplitude", noiseLayers[i].amplitude);
        shader->setUniform("noiseLayers[" + std::to_string(i) + "].frequency", noiseLayers[i].frequency);
        shader->setUniform("noiseLayers[" + std::to_string(i) + "].enabled", noiseLayers[i].enabled);
    }
    shader->setUniform("numNoiseLayers", static_cast<int>(noiseLayers.size()));
    shader->setUniform("finiteDifference", finiteDifference);
    shader->setUniform("useFiniteDifferences", useFiniteDifferences);
    shader->setUniform("power", power);
    shader->setUniform("bowlStrength", bowlStrength);
    shader->setUniform("platformHeight", platformHeight);
    shader->setUniform("seed", seed);
}
