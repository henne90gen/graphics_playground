#include "Landscape.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "util/PerlinNoise.h"
#include "util/SimplexNoise.h"

#define WIDTH 10
#define HEIGHT 10

void Landscape::setup() {
    shader = new Shader("../../../src/app/scenes/landscape/Landscape.vertex",
                        "../../../src/app/scenes/landscape/Landscape.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    generatePoints();
}

void Landscape::generatePoints() {
    vertexArray->bind();

    auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    auto height = static_cast<unsigned int>(HEIGHT * pointDensity);

    unsigned int verticesSize = width * height * 2 * sizeof(float);
    auto vertices = static_cast<float *>(malloc(verticesSize));
    float *vertPtr = vertices;
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            *vertPtr++ = static_cast<float>(x) / pointDensity;
            *vertPtr++ = static_cast<float>(y) / pointDensity;
        }
    }
    auto *positionBuffer = new VertexBuffer(vertices, verticesSize);
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(*positionBuffer, positionLayout);

    heightMapSize = width * height * sizeof(float);
    heightMap = static_cast<float *>(malloc(heightMapSize));
    heightBuffer = new VertexBuffer();
    VertexBufferLayout heightLayout;
    heightLayout.add<float>(shader, "height", 1);
    vertexArray->addBuffer(*heightBuffer, heightLayout);

    unsigned int indicesCount = width * height * 6;
    auto indices = static_cast<unsigned int *>(malloc(indicesCount * sizeof(unsigned int)));
    unsigned int *indPtr = indices;
    for (unsigned int y = 0; y < height - 1; y++) {
        for (unsigned int x = 0; x < width - 1; x++) {
            *indPtr++ = (y + 1) * width + x;
            *indPtr++ = y * width + (x + 1);
            *indPtr++ = y * width + x;
            *indPtr++ = (y + 1) * width + x;
            *indPtr++ = (y + 1) * width + (x + 1);
            *indPtr++ = y * width + (x + 1);
        }
    }
    indexBuffer = new IndexBuffer(indices, indicesCount);

    free(vertices);
    free(indices);
}

void Landscape::destroy() {
    free(heightMap);
}

void Landscape::tick() {
    static auto modelScale = glm::vec3(7.0F, 7.0F, 2.0F);
    static auto translation = glm::vec3(-30.0F, -20.0F, -50.0F);
    static auto modelRotation = glm::vec3(-1.0F, 0.0F, 0.0F);
    static auto cameraRotation = glm::vec3(0.0F);
    static auto scale = glm::vec3(5.0F, 5.0F, 1.0F);
    scale.x = pointDensity;
    scale.y = pointDensity;
    scale.z += 0.01F;
    static auto movement = glm::vec2(0.0F);
    static int noiseMode = 0;
    int lastPointDensity = pointDensity;


    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Scale", reinterpret_cast<float *>(&modelScale), 0.01F);
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.01F);
    ImGui::DragFloat3("Scale", reinterpret_cast<float *>(&scale), 0.01F);
    ImGui::DragFloat2("Movement", reinterpret_cast<float *>(&movement), 0.01F);

    ImGui::BeginGroup();
    ImGui::RadioButton("Perlin", &noiseMode, 0);
    ImGui::RadioButton("Simplex", &noiseMode, 1);
    ImGui::EndGroup();

    ImGui::SliderInt("Point Density", &pointDensity, 1, 100);
    ImGui::Text("Point count: %d", pointDensity * pointDensity * WIDTH * HEIGHT);
    ImGui::End();

    shader->bind();
    vertexArray->bind();

    if (lastPointDensity != pointDensity) {
        generatePoints();
    }

    PerlinNoise perlin;
    auto simplex = SimplexNoise(1);
    float maxHeight = 0;
    auto width = static_cast<unsigned int>(WIDTH * pointDensity);
    auto height = static_cast<unsigned int>(HEIGHT * pointDensity);
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            float realX = static_cast<float>(x) / scale.x + movement.x;
            float realY = static_cast<float>(y) / scale.y + movement.y;

            float generatedHeight;
            if (noiseMode == 0) {
                generatedHeight = static_cast<float>(perlin.noise(realX, realY, scale.z));
            } else {
                generatedHeight = (static_cast<float>(simplex.noise3(realX, realY, scale.z)) + 1.0F) / 2.0F;
            }

            heightMap[y * width + x] = generatedHeight * 10.0F;
            if (heightMap[y * width + x] > maxHeight) {
                maxHeight = heightMap[y * width + x];
            }
        }
    }
    heightBuffer->update(heightMap, heightMapSize);

    shader->setUniform("maxHeight", maxHeight);

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.F), getAspectRatio(), 0.1F, 1000.F);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();

    shader->unbind();
}
