#include "Trees.h"

#include <imgui.h>
#include <util/RenderUtils.h>

#define USE_TREE_MODELS 1

DEFINE_DEFAULT_SHADERS(landscape_Tree)

DEFINE_SHADER(landscape_NoiseLib)

void Trees::init() {
    shader = CREATE_DEFAULT_SHADER(landscape_Tree);
    shader->attachShaderLib(SHADER_CODE(landscape_NoiseLib));
    cubeVA = createCubeVA(shader);

#if USE_TREE_MODELS
    unsigned int error =
          Model::loadFromFile("landscape_resources/assets/models/low_poly_tree/low_poly_tree.obj", shader, treeModel);
    if (error != 0) {
        std::cout << "Failed to load tree model" << std::endl;
        return;
    }

    for (const auto &mesh : treeModel.getRawModel().meshes) {
        vertexCount += mesh.vertices.size();
    }
#endif
}

void Trees::showGui() {
    ImGui::DragInt("Tree Count", &treeCount);
#if USE_TREE_MODELS
    ImGui::Text("Mesh count: %d", treeModel.getMeshes().size());
    ImGui::Text("Vertex count: %d", vertexCount);
#endif
}

void Trees::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                   const TerrainParams &terrainParams) {
#if USE_TREE_MODELS
    if (!treeModel.isLoaded()) {
        return;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(viewMatrix * modelMatrix)));
    shader->bind();
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("normalMatrix", normalMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    terrainParams.setShaderUniforms(shader);

    shader->setUniform("treeCount", treeCount);
    shader->setUniform("textureSampler", 0);

    GL_Call(glActiveTexture(GL_TEXTURE0));
    for (const auto &mesh : treeModel.getMeshes()) {
        if (!mesh.visible) {
            continue;
        }

        mesh.vertexArray->bind();

        mesh.texture->bind();

        if (shaderToggles.drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(glDrawElementsInstanced(GL_TRIANGLES, mesh.indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr,
                                        treeCount));

        if (shaderToggles.drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh.vertexArray->unbind();
    }
#else
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
#endif
}
