#include "scenes/landscape/Landscape.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "SimplexNoise.h"
#include "PerlinNoise.h"

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
    static glm::vec3 translation = glm::vec3(-50.0f, -10.0f, -50.0f);
    static glm::vec3 modelRotation = glm::vec3(-1.0f, 0.0f, 0.0f);
    static glm::vec3 cameraRotation = glm::vec3(0.0f);
    static glm::vec3 scale = glm::vec3(5.0f, 5.0f, 1.0f);
    scale.z += 0.01f;
    static glm::vec2 movement = glm::vec2(0.0f);
    static int noiseMode = 0;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Scale", (float *) &scale, 0.01f);
    ImGui::DragFloat2("Movement", (float *) &movement, 0.01f);

    ImGui::BeginGroup();
    ImGui::RadioButton("Perlin", &noiseMode, 0);
    ImGui::RadioButton("Simplex", &noiseMode, 1);
    ImGui::EndGroup();

    ImGui::End();

    shader->bind();
    vertexArray->bind();

    PerlinNoise perlin;
    auto simplex = SimplexNoise(1);
    static float heights[WIDTH * HEIGHT] = {};
    float maxHeight = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float realX = (float) x / scale.x + movement.x;
            float realY = (float) y / scale.y + movement.y;

            float height;
            if (noiseMode == 0) {
                height = (float) perlin.noise(realX, realY, scale.z);
            } else {
                height = ((float) simplex.noise3(realX, realY, scale.z) + 1.0f) / 2.0f;
            }

            heights[y * WIDTH + x] = height * 10.0f;
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
