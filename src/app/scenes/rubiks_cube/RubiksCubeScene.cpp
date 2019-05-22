#include "RubiksCubeScene.h"

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

void RubiksCubeScene::setup() {
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
    vertices = static_cast<float *>(malloc(verticesSize));

    unsigned int indicesCount = 6 * 6 * cubeCount;
    indices = static_cast<unsigned int *>(malloc(indicesCount * sizeof(unsigned int)));

    float *vertPtr = vertices;
    unsigned int *indPtr = indices;
    for (unsigned int x = 0; x < sideCount; x++) {
        for (unsigned int y = 0; y < sideCount; y++) {
            for (unsigned int z = 0; z < sideCount; z++) {
                const glm::vec3 min = glm::vec3(x - 1.5F, y - 1.5F, z - 1.5F);
                const glm::vec3 max = glm::vec3(x - 0.5F, y - 0.5F, z - 0.5F);
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

    rubiksCube = RubiksCube({
                                    R_FI, R_F,
//                                    R_BA, R_BAI,
//                                    R_L, R_LI,
//                                    R_R, R_RI,
//                                    R_T, R_TI,
//                                    R_BO, R_BOI
                            });
}

void RubiksCubeScene::destroy() {
    free(vertices);
    free(indices);
}

std::string to_string(unsigned int input) {
    if (input < 10) {
        return " " + std::to_string(input);
    }
    return std::to_string(input);
}

void RubiksCubeScene::tick() {
    static auto translation = glm::vec3(-2.0F, 0.0F, -12.0F);
    static auto modelRotation = glm::vec3(0.4F, -0.3F, 0.0F);
    static auto cameraRotation = glm::vec3(0.0F);
    static auto rotationSpeed = 2.01F;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
    ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    ImGui::DragFloat("Animation Speed", &rotationSpeed, 0.001F, 0.001F, 1.0F);

    ImGui::Checkbox("Loop Commands", &rubiksCube.loop);
    if (ImGui::Button("Shuffle")) {
        rubiksCube.shuffle();
    }
    ImGui::Text("Executed Rotation Commands: %d", rubiksCube.executedRotationCommands);

    unsigned int averageLength = rubiksCube.getAverageRotationListLength();
    float averageLengthPerRotationCommand = static_cast<float>(averageLength) / static_cast<float>(rubiksCube.executedRotationCommands);
    ImGui::Text("Average Rotation List Length: %d (%f)", averageLength, averageLengthPerRotationCommand);

    unsigned int maximumLength = rubiksCube.getMaximumRotationListLength();
    float maximumLengthPerRotationCommand = static_cast<float>(maximumLength) / static_cast<float>(rubiksCube.executedRotationCommands);
    ImGui::Text("Maximum Rotation List Length: %d (%f)", maximumLength, maximumLengthPerRotationCommand);

    ImGui::Text("Total Rotation List Entries Count: %d", rubiksCube.getTotalRotationListEntriesCount());
    ImGui::Text("Squashed Rotations: %d", rubiksCube.squashedRotations);
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    indexBuffer->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = glm::mat4(1.0F);
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.x, glm::vec3(1, 0, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.y, glm::vec3(0, 1, 0));
    viewMatrix = glm::rotate(viewMatrix, cameraRotation.z, glm::vec3(0, 0, 1));
    viewMatrix = glm::translate(viewMatrix, translation);
    float fieldOfView = 45.0F;
    float zNear = 0.1F;
    float zFar = 100.0F;
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(fieldOfView), getAspectRatio(), zNear, zFar);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    rubiksCube.rotate(rotationSpeed);

    for (unsigned int i = 0; i < 27; i++) {
        shader->setUniform("cubeMatrix", rubiksCube.getRotationMatrix(i));
        GL_Call(glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void *) (i * 36 * sizeof(unsigned int))));
    }

    indexBuffer->unbind();
    vertexArray->unbind();

    shader->unbind();
}
