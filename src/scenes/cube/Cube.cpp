#include "scenes/cube/Cube.h"

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "ImGuiUtils.h"
#include "opengl/Utils.h"

void Cube::setup() {
    shader = new Shader("../src/scenes/cube/Cube.vertex", "../src/scenes/cube/Cube.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    static float vertices[] = {
            // back
            -1.0f, -1.0f, -1.0f, 1, 0, 0, // 0
            1.0f, -1.0f, -1.0f, 0, 1, 0,  // 1
            1.0f, 1.0f, -1.0f, 0, 0, 1,   // 2
            -1.0f, 1.0f, -1.0f, 1, 1, 0,  // 3

            // front
            -1.0f, -1.0f, 1.0f, 1, 0, 1, // 4
            1.0f, -1.0f, 1.0f, 0, 1, 1,  // 5
            1.0f, 1.0f, 1.0f, 1, 1, 1,   // 6
            -1.0f, 1.0f, 1.0f, 0, 0, 0   // 7
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
    static glm::vec3 translation = glm::vec3(0.0f, 0.0f, -4.5f);
    static glm::vec3 modelRotation = glm::vec3(0.0f);
    static glm::vec3 cameraRotation = glm::vec3(0.0f);
    static float scale = 1.0f;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Position", (float *) &translation, 0.05f);
    ImGui::DragFloat3("Rotation", (float *) &modelRotation, 0.01f);
    ImGui::DragFloat3("Camera Rotation", (float *) &cameraRotation, 0.01f);
    ImGui::DragFloat("Scale", &scale, 0.1f);
    ImGui::End();

    shader->bind();
    vertexArray->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.f), getAspectRatio(), 0.1f, 10.f);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, 0));

    vertexArray->unbind();

    shader->unbind();
}
