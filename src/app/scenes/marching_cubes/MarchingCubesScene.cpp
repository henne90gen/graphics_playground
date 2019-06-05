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

    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    cubeVertexArray = new VertexArray();
    cubeVertexArray->bind();

    auto positionBuffer = VertexBuffer(cubeCorners.data(), cubeCorners.size() * 3 * sizeof(float));
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "position", 3);
    cubeVertexArray->addBuffer(positionBuffer, bufferLayout);

    std::vector<unsigned int> indices = {
            // bottom
            4, 5, 1, 0, 4, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

            // top
            7, 6, 2, 3, 7, // NOLINT(cppcoreguidelines-avoid-magic-numbers)

            // edges
            6, 5, 1, 2, 3, 0 // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    };
    cubeIndexBuffer = new IndexBuffer(indices);
    cubeIndexBuffer->bind();

    marchingCubes = new MarchingCubes();

    surfaceVertexArray = new VertexArray();
    surfaceVertexBuffer = new VertexBuffer();
    surfaceVertexBuffer->bind();
    bufferLayout = VertexBufferLayout();
    bufferLayout.add<float>(shader, "position", 3);
    surfaceVertexArray->addBuffer(*surfaceVertexBuffer, bufferLayout);
    surfaceIndexBuffer = new IndexBuffer();
    surfaceIndexBuffer->bind();
}

void MarchingCubesScene::destroy() {
    delete marchingCubes;
}

void MarchingCubesScene::tick() {
    static auto translation = glm::vec3(0.0F, -2.5F, -8.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3();
    static auto cameraRotation = glm::vec3(0.25F, 0.0F, 0.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static float scale = 0.1F; // NOLINT(cppcoreguidelines-avoid-magic-numbers)

    const float rotationSpeed = 0.03F;
    modelRotation.y += rotationSpeed;

    showSettings(translation, cameraRotation, modelRotation, scale);

    marchingCubes->step();

    glm::vec3 dimensions = glm::vec3(
            static_cast<float>(marchingCubes->width),
            static_cast<float>(marchingCubes->height),
            static_cast<float>(marchingCubes->depth)
    );
    glm::vec3 modelCenter = dimensions * -0.5F;

    shader->bind();
    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::translate(modelMatrix, modelCenter);
    glm::mat4 viewMatrix = createViewMatrix(translation, cameraRotation);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("dimensions", dimensions);

    drawSurface();
    drawCube();
    shader->unbind();
}

glm::mat4 MarchingCubesScene::createViewMatrix(const glm::vec3 &translation, const glm::vec3 &cameraRotation) const {
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::scale(viewMatrix, glm::vec3(1.0F));
    viewMatrix = glm::translate(viewMatrix, glm::vec3());
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    return viewMatrix;
}

void MarchingCubesScene::showSettings(glm::vec3 &translation, glm::vec3 &cameraRotation, glm::vec3 &modelRotation,
                                      float &scale) const {
    ImGui::Begin("Settings");

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    ImGui::DragFloat("Scale", &scale, 0.001F);

    ImGui::Checkbox("Animate", &marchingCubes->animate);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    ImGui::DragInt("Animation Speed", &marchingCubes->animationSpeed, 1.0F, 1, 100);

    std::array<int, 3> dimensions = {static_cast<int>(marchingCubes->width), static_cast<int>(marchingCubes->height),
                                     static_cast<int>(marchingCubes->depth)};
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    ImGui::DragInt3("Dimensions", dimensions.data(), 1.0F, 1, 100);
    marchingCubes->width = dimensions[0];
    marchingCubes->height = dimensions[1];
    marchingCubes->depth = dimensions[2];

    ImGui::Checkbox("Interpolate", &marchingCubes->interpolate);
    ImGui::SliderFloat("Surface Level", &marchingCubes->surfaceLevel, 0.0F, 1.0F);
    ImGui::SliderFloat("Frequency", &marchingCubes->frequency, 0.0F, 1.0F);

    ImGui::NoiseTypeSelector(&marchingCubes->noiseType);

    if (ImGui::Button("Start")) {
        marchingCubes->start();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        marchingCubes->reset();
    }

    ImGui::Text("Number of vertices: %zu", marchingCubes->getVertices().size());
    ImGui::Text("Number of indices: %zu", marchingCubes->getIndices().size());

    ImGui::End();
}

void MarchingCubesScene::drawCube() {
    shader->setUniform("offset", marchingCubes->getCubeTranslation());
    cubeVertexArray->bind();
    GL_Call(glDrawElements(GL_LINE_LOOP, cubeIndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
    cubeVertexArray->unbind();
}

void MarchingCubesScene::drawSurface() {
    surfaceVertexBuffer->update(marchingCubes->getVertices());
    surfaceIndexBuffer->update(marchingCubes->getIndices());

    shader->setUniform("offset", glm::vec3());
    surfaceVertexArray->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, surfaceIndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));
    surfaceVertexArray->unbind();
}
