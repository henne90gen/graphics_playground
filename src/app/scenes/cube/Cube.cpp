#include "Cube.h"

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void Cube::setup() {
    shader = new Shader("../../../src/app/scenes/cube/Cube.vertex",
                        "../../../src/app/scenes/cube/Cube.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    static float vertices[] = {
            // back
            -1.0F, -1.0F, -1.0F, 1, 0, 0, // 0
            1.0F, -1.0F, -1.0F, 0, 1, 0,  // 1
            1.0F, 1.0F, -1.0F, 0, 0, 1,   // 2
            -1.0F, 1.0F, -1.0F, 1, 1, 0,  // 3

            // front
            -1.0F, -1.0F, 1.0F, 1, 0, 1, // 4
            1.0F, -1.0F, 1.0F, 0, 1, 1,  // 5
            1.0F, 1.0F, 1.0F, 1, 1, 1,   // 6
            -1.0F, 1.0F, 1.0F, 0, 0, 0   // 7
    };
    auto *positionBuffer = new VertexBuffer(vertices, sizeof(vertices));
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "position", 3);
    bufferLayout.add<float>(shader, "color", 3);
    vertexArray->addBuffer(*positionBuffer, bufferLayout);

    unsigned int indices[] = {
            // front
            0, 1, 2, //
            0, 2, 3, //

            // back
            4, 5, 6, //
            4, 6, 7, //

            // right
            5, 1, 2, //
            5, 2, 6, //

            // left
            0, 4, 7, //
            0, 7, 3, //

            // top
            7, 6, 2, //
            7, 2, 3, //

            // bottom
            4, 5, 1, //
            4, 1, 0, //
    };
    indexBuffer = new IndexBuffer(indices, sizeof(indices) / sizeof(float));
}

void Cube::destroy() {}

void Cube::tick() {
    static auto translation = glm::vec3(0.0F, 0.0F, -4.5F);
    static auto modelRotation = glm::vec3(0.0F);
    static auto cameraRotation = glm::vec3(0.0F);
    static float scale = 1.0F;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
    ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    ImGui::DragFloat("Scale", &scale, 0.001F);
    ImGui::End();

    shader->bind();
    vertexArray->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.F), getAspectRatio(), 0.1F, 10.F);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();

    shader->unbind();
}
