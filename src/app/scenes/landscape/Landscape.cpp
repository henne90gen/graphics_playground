#include "Landscape.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "util/SimplexNoise.h"
#include "util/PerlinNoise.h"

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
    auto vertices = (float *) malloc(verticesSize);
    float *vertPtr = vertices;
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            *vertPtr++ = (float) x / pointDensity;
            *vertPtr++ = (float) y / pointDensity;
        }
    }
    auto *positionBuffer = new VertexBuffer(vertices, verticesSize);
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(*positionBuffer, positionLayout);

    heightMapSize = width * height * sizeof(float);
    heightMap = (float *) malloc(heightMapSize);
    heightBuffer = new VertexBuffer();
    VertexBufferLayout heightLayout;
    heightLayout.add<float>(shader, "height", 1);
    vertexArray->addBuffer(*heightBuffer, heightLayout);

    unsigned int indicesCount = width * height * 6;
    auto indices = (unsigned int *) malloc(indicesCount * sizeof(unsigned int));
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
    static glm::vec3 modelScale = glm::vec3(7.0f, 7.0f, 2.0f);
    static glm::vec3 translation = glm::vec3(-30.0f, -20.0f, -50.0f);
    static glm::vec3 modelRotation = glm::vec3(-1.0f, 0.0f, 0.0f);
    static glm::vec3 cameraRotation = glm::vec3(0.0f);
    static glm::vec3 scale = glm::vec3(5.0f, 5.0f, 1.0f);
    scale.x = pointDensity;
    scale.y = pointDensity;
    scale.z += 0.01f;
    static glm::vec2 movement = glm::vec2(0.0f);
    static int noiseMode = 0;
    int lastPointDensity = pointDensity;


    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Scale", (float *) &modelScale, 0.01f);
    ImGui::DragFloat3("Position", (float *) &translation, 0.01f);
    ImGui::DragFloat3("Scale", (float *) &scale, 0.01f);
    ImGui::DragFloat2("Movement", (float *) &movement, 0.01f);

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
            float realX = (float) x / scale.x + movement.x;
            float realY = (float) y / scale.y + movement.y;

            float generatedHeight;
            if (noiseMode == 0) {
                generatedHeight = (float) perlin.noise(realX, realY, scale.z);
            } else {
                generatedHeight = ((float) simplex.noise3(realX, realY, scale.z) + 1.0f) / 2.0f;
            }

            heightMap[y * width + x] = generatedHeight * 10.0f;
            if (heightMap[y * width + x] > maxHeight) {
                maxHeight = heightMap[y * width + x];
            }
        }
    }
    heightBuffer->update(heightMap, heightMapSize);

    shader->setUniform("maxHeight", maxHeight);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, modelScale);
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
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();

    shader->unbind();
}
