#include "MarchingCubesScene.h"

#include <array>

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

void MarchingCubesScene::setup() {
    shader = new Shader("../../../src/app/scenes/marching_cubes/MarchingCubes.vertex",
                        "../../../src/app/scenes/marching_cubes/MarchingCubes.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    std::vector<float> vertices = {
            // back
            -1.0F, -1.0F, -1.0F, // 0
            1.0F, -1.0F, -1.0F,  // 1
            1.0F, 1.0F, -1.0F,   // 2
            -1.0F, 1.0F, -1.0F,  // 3

            // front
            -1.0F, -1.0F, 1.0F, // 4
            1.0F, -1.0F, 1.0F,  // 5
            1.0F, 1.0F, 1.0F,   // 6
            -1.0F, 1.0F, 1.0F   // 7
    };
    auto *positionBuffer = new VertexBuffer(vertices);
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "position", 3);
    vertexArray->addBuffer(*positionBuffer, bufferLayout);

    std::vector<unsigned int> indices = {
            // front
            0, 1, 2, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            0, 2, 3, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

            // back
            4, 5, 6, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            4, 6, 7, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

            // right
            5, 1, 2, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            5, 2, 6, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

            // left
            0, 4, 7, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            0, 7, 3, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

            // top
            7, 6, 2, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            7, 2, 3, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

            // bottom
            4, 5, 1, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
            4, 1, 0, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    };
    indexBuffer = new IndexBuffer(indices);

    marchingCubes = new MarchingCubes();
}

void MarchingCubesScene::destroy() {
    delete marchingCubes;
}

void MarchingCubesScene::tick() {
    static auto translation = glm::vec3(0.0F, 0.0F, -4.5F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3(0.0F);
    static auto cameraRotation = glm::vec3(0.0F);
    static float scale = 0.1F;

    ImGui::Begin("Settings");

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numberss)
    ImGui::DragFloat("Scale", &scale, 0.001F);

    ImGui::DragInt("Animation Speed", &marchingCubes->animationSpeed);
    if (ImGui::Button("Start")) {
        marchingCubes->start();
    }
    if (ImGui::Button("Reset")) {
        marchingCubes->reset();
    }

    ImGui::End();

    marchingCubes->step();

    shader->bind();
    vertexArray->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::translate(modelMatrix, marchingCubes->getCubeTranslation());
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::scale(viewMatrix, glm::vec3(1.0F));
    viewMatrix = glm::translate(viewMatrix, glm::vec3());
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
    GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    vertexArray->unbind();
    shader->unbind();
}
