#include "RubiksCubeScene.h"

#include <glm/ext.hpp>

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

const unsigned int NUMBER_OF_SMALL_CUBES = 27;

float *addCubeVertices(float *vertPtr, glm::vec3 min, glm::vec3 max) {
    // front face
    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // back face
    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // left face
    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // right face
    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // top face
    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    // bottom face
    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = max.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = max.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    *vertPtr++ = min.x; // x NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.y; // y NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = min.z; // z NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 1; // r NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // g NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    *vertPtr++ = 0; // b NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    return vertPtr;
}

unsigned int *addCubeIndices(unsigned int *indPtr, unsigned int cubeNumber) {
    const unsigned int numVerticesPerFace = 4;
    const unsigned int numFaces = 6;
    const unsigned int startIndex = cubeNumber * numFaces * numVerticesPerFace;
    for (unsigned int i = 0; i < numFaces; i++) {
        *indPtr++ = startIndex + i * numVerticesPerFace; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *indPtr++ = startIndex + i * numVerticesPerFace + 1; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *indPtr++ = startIndex + i * numVerticesPerFace + 2; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *indPtr++ = startIndex + i * numVerticesPerFace; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *indPtr++ = startIndex + i * numVerticesPerFace + 2; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *indPtr++ = startIndex + i * numVerticesPerFace + 3; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return indPtr;
}

void RubiksCubeScene::setup() {
    shader = std::make_shared<Shader>("scenes/rubiks_cube/RubiksCubeVert.glsl",
                                      "scenes/rubiks_cube/RubiksCubeFrag.glsl");
    shader->bind();

    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    const unsigned int sideCount = 3;
    const unsigned int cubeCount = sideCount * sideCount * sideCount;
    const int numFaces = 6;
    const int numVerticesPerFace = 4;
    const int verticesPerCube = numFaces * numVerticesPerFace;
    const int floatsPerVertex = 6;
    const unsigned int verticesSize = cubeCount * verticesPerCube * floatsPerVertex * sizeof(float);
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc,hicpp-no-malloc)
    vertices = static_cast<float *>(malloc(verticesSize));

    const int numIndicesPerFace = 6;
    const unsigned int indicesCount = numFaces * numIndicesPerFace * cubeCount;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc,hicpp-no-malloc)
    indices = static_cast<unsigned int *>(malloc(indicesCount * sizeof(unsigned int)));

    float *vertPtr = vertices;
    unsigned int *indPtr = indices;
    for (unsigned int x = 0; x < sideCount; x++) {
        for (unsigned int y = 0; y < sideCount; y++) {
            for (unsigned int z = 0; z < sideCount; z++) {
                auto xF = static_cast<float>(x);
                auto yF = static_cast<float>(y);
                auto zF = static_cast<float>(z);

                const float minOffset = 1.5F;
                const float maxOffset = 0.5F;
                const glm::vec3 min = glm::vec3(xF - minOffset, yF - minOffset, zF - minOffset);
                const glm::vec3 max = glm::vec3(xF - maxOffset, yF - maxOffset, zF - maxOffset);
                vertPtr = addCubeVertices(vertPtr, min, max);

                unsigned int cubeNumber = x + (y * sideCount) + (z * sideCount * sideCount);
                indPtr = addCubeIndices(indPtr, cubeNumber);
            }
        }
    }

    BufferLayout bufferLayout = {
            {ShaderDataType::Float3, "position"},
            {ShaderDataType::Float3, "color"}
    };
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, verticesSize, bufferLayout);
    vertexArray->addVertexBuffer(positionBuffer);

    indexBuffer = std::make_shared<IndexBuffer>(indices, indicesCount);

    rubiksCube.reset(new RubiksCube(
            {
                    R_F, R_L, R_RI, R_LI, R_R, R_F, R_F, R_BOI, R_TI, R_BAI, R_L, R_FI, R_FI,
                    R_L, R_L, R_BO, R_BOI, R_BAI, R_TI, R_BOI
            }
    ));
}

void RubiksCubeScene::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void RubiksCubeScene::destroy() {
    free(vertices); // NOLINT(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,hicpp-no-malloc)
    free(indices); // NOLINT(cppcoreguidelines-no-malloc,cppcoreguidelines-owning-memory,hicpp-no-malloc)
}

std::string to_string(unsigned int input) {
    const int DECIMAL_BORDER = 10;
    if (input < DECIMAL_BORDER) {
        return " " + std::to_string(input);
    }
    return std::to_string(input);
}

void RubiksCubeScene::tick() {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    static auto translation = glm::vec3(3.0F, 0.0F, -12.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    static auto modelRotation = glm::vec3(-0.56F, -0.68F, 0.0F);
    static auto cameraRotation = glm::vec3(0.0F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    static auto rotationSpeed = 2.01F;

    ImGui::Begin("Settings");
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    ImGui::DragFloat("Animation Speed", &rotationSpeed, 0.001F, 0.001F, 1.0F);

    ImGui::Checkbox("Loop Commands", &rubiksCube->loop);
    if (ImGui::Button("Shuffle")) {
        rubiksCube->shuffle();
    }
    if (ImGui::Button("Solve")) {
        rubiksCube->startSolving();
    }
    ImGui::Text("Executed Rotation Commands: %d", rubiksCube->executedRotationCommands);

    unsigned int averageLength = rubiksCube->getAverageRotationListLength();
    float averageLengthPerRotationCommand =
            static_cast<float>(averageLength) / static_cast<float>(rubiksCube->executedRotationCommands);
    ImGui::Text("Average Rotation List Length: %d (%f)", averageLength, averageLengthPerRotationCommand);

    unsigned int maximumLength = rubiksCube->getMaximumRotationListLength();
    float maximumLengthPerRotationCommand =
            static_cast<float>(maximumLength) / static_cast<float>(rubiksCube->executedRotationCommands);
    ImGui::Text("Maximum Rotation List Length: %d (%f)", maximumLength, maximumLengthPerRotationCommand);

    ImGui::Text("Total Rotation List Entries Count: %d", rubiksCube->getTotalRotationListEntriesCount());
    ImGui::Text("Squashed Rotations: %d", rubiksCube->squashedRotations);
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    indexBuffer->bind();

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = createViewMatrix(translation, cameraRotation);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    rubiksCube->solve();
    rubiksCube->rotate(rotationSpeed);

    for (unsigned int i = 0; i < NUMBER_OF_SMALL_CUBES; i++) {
        shader->setUniform("cubeMatrix", rubiksCube->getRotationMatrix(i));
        const unsigned int vertexCountPerSmallCube = 36;
        unsigned int count = i * vertexCountPerSmallCube * sizeof(unsigned int);
        GL_Call(glDrawElements(GL_TRIANGLES, vertexCountPerSmallCube, GL_UNSIGNED_INT,
                               reinterpret_cast<void *>(count))); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    indexBuffer->unbind();
    vertexArray->unbind();

    shader->unbind();
}
