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
            -0.5F, -0.5F, -0.5F, // 0
            0.5F, -0.5F, -0.5F,  // 1
            0.5F, 0.5F, -0.5F,   // 2
            -0.5F, 0.5F, -0.5F,  // 3

            // front
            -0.5F, -0.5F, 0.5F, // 4
            0.5F, -0.5F, 0.5F,  // 5
            0.5F, 0.5F, 0.5F,   // 6
            -0.5F, 0.5F, 0.5F   // 7
    };
    auto *positionBuffer = new VertexBuffer(vertices);
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "position", 3);
    vertexArray->addBuffer(*positionBuffer, bufferLayout);

    std::vector<unsigned int> indices = {
            // bottom
            4,5,1,0,4,

            // top
            7, 6, 2, 3, 7,

            // edges
            6, 5, 1, 2, 3, 0
    };
    indexBuffer = new IndexBuffer(indices);

    marchingCubes = new MarchingCubes();
}

void MarchingCubesScene::destroy() {
    delete marchingCubes;
}

void MarchingCubesScene::tick() {
    static auto translation = glm::vec3(-2.75F, -2.1F, -5.5F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3(0.21F, -0.35F, 0.0F);
    static float scale = 0.1F;

    ImGui::Begin("Settings");

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
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

    drawCube(translation, cameraRotation, scale);

}

void MarchingCubesScene::drawCube(const glm::vec3 &translation, const glm::vec3 &cameraRotation, float scale) {
    shader->bind();
    vertexArray->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::translate(modelMatrix, marchingCubes->getCubeTranslation());
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
    GL_Call(glDrawElements(GL_LINE_LOOP, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();
    shader->unbind();
}
