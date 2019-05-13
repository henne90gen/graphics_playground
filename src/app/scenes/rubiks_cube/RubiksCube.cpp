#include "RubiksCube.h"

#include <glm/ext.hpp>

float *addCubeVertices(float *vertPtr, glm::vec3 min, glm::vec3 max) {
    // front face
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    // back face
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    // left face
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 1; // b

    // right face
    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 1; // b

    // top face
    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0; // r
    *vertPtr++ = 1; // g
    *vertPtr++ = 0; // b

    // bottom face
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 0; // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1; // r
    *vertPtr++ = 0; // g
    *vertPtr++ = 0; // b

    return vertPtr;
}

unsigned int *addCubeIndices(unsigned int *indPtr, unsigned int cubeNumber) {
    // 6 indices per face, 6 faces per cube
    unsigned int startIndex = cubeNumber * 24;
    for (unsigned int i = 0; i < 6; i++) {
        *indPtr++ = startIndex + i * 4;
        *indPtr++ = startIndex + i * 4 + 1;
        *indPtr++ = startIndex + i * 4 + 2;
        *indPtr++ = startIndex + i * 4;
        *indPtr++ = startIndex + i * 4 + 2;
        *indPtr++ = startIndex + i * 4 + 3;
    }
    return indPtr;
}

void RubiksCube::setup() {
    shader = new Shader("../../../src/app/scenes/rubiks_cube/RubiksCube.vertex",
                        "../../../src/app/scenes/rubiks_cube/RubiksCube.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    unsigned int sideCount = 3;
    unsigned int cubeCount = sideCount * sideCount * sideCount;
    int verticesPerCube = 4 * 6;
    int floatsPerVertex = 6;
    unsigned int verticesSize = cubeCount * verticesPerCube * floatsPerVertex * sizeof(float);
    vertices = (float *) malloc(verticesSize);

    unsigned int indicesCount = 6 * 6 * cubeCount;
    indices = (unsigned int *) malloc(indicesCount * sizeof(unsigned int));

    float *vertPtr = vertices;
    unsigned int *indPtr = indices;
    for (unsigned int x = 0; x < sideCount; x++) {
        for (unsigned int y = 0; y < sideCount; y++) {
            for (unsigned int z = 0; z < sideCount; z++) {
                const glm::vec3 min = glm::vec3(x - 1.5f, y - 1.5f, z - 1.5f);
                const glm::vec3 max = glm::vec3(x - 0.5f, y - 0.5f, z - 0.5f);
                vertPtr = addCubeVertices(vertPtr, min, max);
                unsigned int cubeNumber = x + (y * sideCount) + (z * sideCount * sideCount);
                indPtr = addCubeIndices(indPtr, cubeNumber);
            }
        }
    }

    auto *positionBuffer = new VertexBuffer(vertices, verticesSize);
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "position", 3);
    bufferLayout.add<float>(shader, "color", 3);
    vertexArray->addBuffer(*positionBuffer, bufferLayout);

    indexBuffer = new IndexBuffer(indices, indicesCount);

    for (auto &cubeRotation : cubeRotations) {
        cubeRotation = {
                std::vector<glm::vec3>(),
                glm::mat4(1.0f)
        };
        cubeRotation.rotations.emplace_back();
    }

    for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(unsigned int); i++) {
        cubePositions[i] = i;
    }

    rotation = {RIGHT, CLOCKWISE, 0};
}

void RubiksCube::destroy() {
    free(vertices);
    free(indices);
}

std::string to_string(unsigned int input) {
    if (input < 10) {
        return " " + std::to_string(input);
    }
    return std::to_string(input);
}

void RubiksCube::tick() {
    static auto translation = glm::vec3(-2.0f, 0.0f, -12.0f);
    static auto modelRotation = glm::vec3(0.4f, -0.3f, 0.0f);
    static auto cameraRotation = glm::vec3(0.0f);

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Position", (float *) &translation, 0.05f);
    ImGui::DragFloat3("Rotation", (float *) &modelRotation, 0.01f);
    ImGui::DragFloat3("Camera Rotation", (float *) &cameraRotation, 0.01f);
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    indexBuffer->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.f), getAspectRatio(), 0.1f, 100.f);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    rotation = {rotation.face, rotation.direction, rotation.currentAngle};
    if (rotate(cubeRotations, cubePositions, rotation, 0.01f)) {
        if (rotation.face == BOTTOM) {
            rotation = {RIGHT, CLOCKWISE, 0};
        } else {
            rotation = {BOTTOM, CLOCKWISE, 0};
        }
    }

    for (unsigned int i = 0; i < 27; i++) {
        shader->setUniform("cubeMatrix", cubeRotations[i].rotationMatrix);
        GL_Call(glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void *) (i * 36 * sizeof(unsigned int))));
    }

    indexBuffer->unbind();
    vertexArray->unbind();

    shader->unbind();
}
