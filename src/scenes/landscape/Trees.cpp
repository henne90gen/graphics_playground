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
    treeModel = std::make_shared<Model>();
    treeModel->loadFromFile("landscape_resources/assets/models/low_poly_tree/low_poly_tree.obj", shader);
    for (const auto &mesh : treeModel->getRawModel()->meshes) {
        vertexCount += mesh.vertices.size();
    }
#endif
}

void Trees::showGui() {
    ImGui::DragInt("Tree Count", &treeCount);
#if USE_TREE_MODELS
    ImGui::Text("Mesh count: %d", treeModel->getMeshes().size());
    ImGui::Text("Vertex count: %d", vertexCount);
#endif
}

void Trees::render(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const ShaderToggles &shaderToggles,
                   const TerrainParams &terrainParams) {
#if USE_TREE_MODELS
    if (!treeModel) {
        return;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    shader->bind();
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    terrainParams.setShaderUniforms(shader);

    shader->setUniform("treeCount", treeCount);

    for (const auto &mesh : treeModel->getMeshes()) {
        if (!mesh->visible) {
            continue;
        }

        mesh->vertexArray->bind();

        mesh->texture->bind();

        if (shaderToggles.drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(glDrawElementsInstanced(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr,
                                        treeCount));
        //        GL_Call(glDrawElements(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        if (shaderToggles.drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh->vertexArray->unbind();
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
