#include "Trees.h"

#include <imgui.h>
#include <util/RenderUtils.h>

DEFINE_DEFAULT_SHADERS(landscape_Tree)

DEFINE_SHADER(landscape_NoiseLib)

void Trees::init() {
    shader = CREATE_DEFAULT_SHADER(landscape_Tree);
    shader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));
    cubeVA = createCubeVA(shader);
}

void Trees::showGui() { ImGui::DragInt("Tree Count", &treeCount); }

void Trees::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                   const TerrainParams &terrainParams) {
#if 1
    cubeVA->bind();
    shader->bind();
    cubeVA->setShader(shader);
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    terrainParams.setShaderUniforms(shader);

    shader->setUniform("treeCount", treeCount);

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElementsInstanced(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr,
                                    treeCount));

    if (shaderToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

#else
    if (!treeModel) {
        return;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    static auto scale = 0.1F;
    ImGui::Begin("Settings");
    ImGui::DragFloat("Scale", &scale, 0.001);
    ImGui::End();
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    //    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    //    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    //    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    treeShader->bind();
    treeShader->setUniform("modelMatrix", modelMatrix);
    treeShader->setUniform("viewMatrix", viewMatrix);
    treeShader->setUniform("projectionMatrix", projectionMatrix);

    for (int i = 0; i < static_cast<int64_t>(noiseLayers.size()); i++) {
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].amplitude", noiseLayers[i].amplitude);
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].frequency", noiseLayers[i].frequency);
        treeShader->setUniform("noiseLayers[" + std::to_string(i) + "].enabled", noiseLayers[i].enabled);
    }
    treeShader->setUniform("numNoiseLayers", static_cast<int>(noiseLayers.size()));
    treeShader->setUniform("finiteDifference", finiteDifference);
    treeShader->setUniform("useFiniteDifferences", shaderToggles.useFiniteDifferences);
    treeShader->setUniform("power", power);
    treeShader->setUniform("bowlStrength", bowlStrength);
    treeShader->setUniform("platformHeight", platformHeight);

    treeShader->setUniform("treeCount", treeCount);

    //    treeShader->setUniform("u_TextureSampler", 0);

    for (const auto &mesh : treeModel->getMeshes()) {
        if (!mesh->visible) {
            continue;
        }

        mesh->terrainVA->bind();

        mesh->texture->bind();

        //        if (drawWireframe) {
        //            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        //        }

        GL_Call(glDrawElementsInstanced(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr,
                                        treeCount));
        //        GL_Call(glDrawElements(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        //        if (drawWireframe) {
        //            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        //        }

        mesh->terrainVA->unbind();
    }
#endif
}
