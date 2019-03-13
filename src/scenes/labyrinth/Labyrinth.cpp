#include "scenes/labyrinth/Labyrinth.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "SimplexNoise.h"
#include "PerlinNoise.h"

void Labyrinth::setup() {
    shader = new Shader("../src/scenes/labyrinth/Labyrinth.vertex", "../src/scenes/labyrinth/Labyrinth.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

#define WIDTH 100
#define HEIGHT 100

    static float vertices[WIDTH * HEIGHT * 3] = {};
    float *vertPtr = vertices;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            *vertPtr++ = (float) x;
            *vertPtr++ = (float) y;
            *vertPtr++ = (float) 0.0;
        }
    }
    auto *positionBuffer = new VertexBuffer(vertices, sizeof(vertices));
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 3);
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

void Labyrinth::destroy() {}

void Labyrinth::tick() {
    static glm::vec3 translation = glm::vec3(-50.0f, -20.0f, -50.0f);
    static glm::vec3 modelRotation = glm::vec3(-1.0f, 0.0f, 0.0f);
    static glm::vec3 cameraRotation = glm::vec3(0.0f);

    ImGui::Begin("Settings");
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
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.f), getAspectRatio(), 0.1f, 1000.f);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, 0));

    vertexArray->unbind();

    shader->unbind();
}
