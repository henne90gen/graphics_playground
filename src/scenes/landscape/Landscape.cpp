#include "scenes/landscape/Landscape.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

void Landscape::setup() {
    shader = new Shader("../src/scenes/landscape/Landscape.vertex", "../src/scenes/landscape/Landscape.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

#define WIDTH 100
#define HEIGHT 100

    static float vertices[WIDTH * HEIGHT * 2] = {};
    float *vertPtr = vertices;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            *vertPtr++ = (float) x / 1.0f;
            *vertPtr++ = (float) y / 1.0f;
        }
    }
    auto *positionBuffer = new VertexBuffer(vertices, sizeof(vertices));
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(*positionBuffer, positionLayout);

    unsigned int indices[WIDTH * HEIGHT * 6] = {};
    unsigned int *indPtr = indices;
    for (int y = 0; y < HEIGHT - 1; y++) {
        for (int x = 0; x < WIDTH - 1; x++) {
            *indPtr++ = (unsigned int) ((y + 1) * WIDTH + x);
            *indPtr++ = (unsigned int) (y * WIDTH + (x + 1));
            *indPtr++ = (unsigned int) (y * WIDTH + x);
            *indPtr++ = (unsigned int) ((y + 1) * WIDTH + x);
            *indPtr++ = (unsigned int) ((y + 1) * WIDTH + (x + 1));
            *indPtr++ = (unsigned int) (y * WIDTH + (x + 1));
        }
    }
    indexBuffer = new IndexBuffer(indices, sizeof(indices) / sizeof(float));
}

void Landscape::destroy() {}

void Landscape::tick() {
    static glm::vec3 translation = glm::vec3(-4.5f, -1.5f, -50.0f);
    static glm::vec3 modelRotation = glm::vec3(-1.0f, 0.0f, 0.0f);
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

    static float heights[WIDTH * HEIGHT] = {};
    float maxHeight = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float realX = (float) x / (float) WIDTH - 0.5f;
            float realY = (float) y / (float) HEIGHT - 0.5f;
            heights[y * WIDTH + x] = -1 * realX * realX + -1 * realY * realY + 1.0f;
            heights[y * WIDTH + x] = heights[y * WIDTH + x]  * scale;
            if (heights[y * WIDTH + x] > maxHeight) {
                maxHeight = heights[y * WIDTH + x];
            }
        }
    }
    auto *heightBuffer = new VertexBuffer(heights, sizeof(heights));
    VertexBufferLayout heightLayout;
    heightLayout.add<float>(shader, "height", 1);
    vertexArray->addBuffer(*heightBuffer, heightLayout);

    shader->setUniform("maxHeight", maxHeight);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.f), getAspectRatio(), 0.1f, 1000.f);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, 0));

    vertexArray->unbind();

    shader->unbind();
}
