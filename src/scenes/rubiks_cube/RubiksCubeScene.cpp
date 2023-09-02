#include "RubiksCubeScene.h"

#include <glm/ext.hpp>

#include "Main.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

const unsigned int NUMBER_OF_SMALL_CUBES = 27;

DEFINE_SCENE_MAIN(RubiksCubeScene)
DEFINE_DEFAULT_SHADERS(rubiks_cube_RubiksCube)

float *addCubeVertices(float *vertPtr, glm::vec3 min, glm::vec3 max) {
    // front face
    auto r = 0.7176470588235294F;
    auto g = 0.0705882352941176F;
    auto b = 0.203921568627451F;
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    // back face
    r = 1.0F;
    g = 0.3450980392156863F;
    b = 0.0F;
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    // left face
    r = 0.0F;
    g = 0.2745098039215686F;
    b = 0.6784313725490196F;
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    // right face
    r = 0.0F;
    g = 0.607843137254902F;
    b = 0.2823529411764706F;
    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    // top face
    r = 1.0F;
    g = 0.8352941176470588F;
    b = 0.0F;
    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = max.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    // bottom face
    r = 1.0F;
    g = 1.0F;
    b = 1.0F;
    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = max.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 1.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = max.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 1.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    *vertPtr++ = min.x; // x
    *vertPtr++ = min.y; // y
    *vertPtr++ = min.z; // z
    *vertPtr++ = 0.0F;  // u
    *vertPtr++ = 0.0F;  // v
    *vertPtr++ = r;     // r
    *vertPtr++ = g;     // g
    *vertPtr++ = b;     // b

    return vertPtr;
}

unsigned int *addCubeIndices(unsigned int *indPtr, unsigned int cubeNumber) {
    const unsigned int numVerticesPerFace = 4;
    const unsigned int numFaces = 6;
    const unsigned int startIndex = cubeNumber * numFaces * numVerticesPerFace;
    for (unsigned int i = 0; i < numFaces; i++) {
        *indPtr++ = startIndex + i * numVerticesPerFace;
        *indPtr++ = startIndex + i * numVerticesPerFace + 1;
        *indPtr++ = startIndex + i * numVerticesPerFace + 2;
        *indPtr++ = startIndex + i * numVerticesPerFace;
        *indPtr++ = startIndex + i * numVerticesPerFace + 2;
        *indPtr++ = startIndex + i * numVerticesPerFace + 3;
    }
    return indPtr;
}

void RubiksCubeScene::setup() {
    static auto modelRotation = glm::vec3(0.56F, -0.68F, 0.0F);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));

    shader = CREATE_DEFAULT_SHADER(rubiks_cube_RubiksCube);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    const unsigned int sideCount = 3;
    const unsigned int cubeCount = sideCount * sideCount * sideCount;
    const int numFaces = 6;
    const int numVerticesPerFace = 4;
    const int verticesPerCube = numFaces * numVerticesPerFace;
    const int floatsPerVertex = 8;
    const unsigned int verticesSize = cubeCount * verticesPerCube * floatsPerVertex * sizeof(float);
    vertices = static_cast<float *>(malloc(verticesSize));

    const int numIndicesPerFace = 6;
    const unsigned int indicesCount = numFaces * numIndicesPerFace * cubeCount;
    indices = static_cast<unsigned int *>(malloc(indicesCount * sizeof(unsigned int)));

    float *vertPtr = vertices;
    unsigned int *indPtr = indices;
    for (unsigned int x = 0; x < sideCount; x++) {
        for (unsigned int y = 0; y < sideCount; y++) {
            for (unsigned int z = 0; z < sideCount; z++) {
                const auto xF = static_cast<float>(x);
                const auto yF = static_cast<float>(y);
                const auto zF = static_cast<float>(z);

                const float minOffset = 1.5F;
                const float maxOffset = 0.5F;
                const glm::vec3 min = glm::vec3(xF - minOffset, yF - minOffset, zF - minOffset);
                const glm::vec3 max = glm::vec3(xF - maxOffset, yF - maxOffset, zF - maxOffset);
                vertPtr = addCubeVertices(vertPtr, min, max);

                const unsigned int cubeNumber = x + (y * sideCount) + (z * sideCount * sideCount);
                indPtr = addCubeIndices(indPtr, cubeNumber);
            }
        }
    }

    const BufferLayout bufferLayout = {
          {ShaderDataType::Float3, "position"}, {ShaderDataType::Float2, "uv"}, {ShaderDataType::Float3, "color"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, verticesSize, bufferLayout);
    vertexArray->addVertexBuffer(positionBuffer);

    indexBuffer = std::make_shared<IndexBuffer>(indices, indicesCount);

    rubiksCube.reset(new rubiks::AnimationRubiksCube());
}

void RubiksCubeScene::destroy() {
    free(vertices);
    free(indices);
}

std::string to_string(unsigned int input) {
    const int DECIMAL_BORDER = 10;
    if (input < DECIMAL_BORDER) {
        return " " + std::to_string(input);
    }
    return std::to_string(input);
}

void showRotationButtons(std::shared_ptr<rubiks::AnimationRubiksCube> cube) {
    ImGui::NewLine();
    if (!ImGui::BeginTable("Rotation Buttons", 2)) {
        return;
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (ImGui::Button("Front")) {
        cube->addRotationCommand(R_F);
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Front Inverse")) {
        cube->addRotationCommand(R_FI);
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (ImGui::Button("Back")) {
        cube->addRotationCommand(R_B);
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Back Inverse")) {
        cube->addRotationCommand(R_BI);
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (ImGui::Button("Left")) {
        cube->addRotationCommand(R_L);
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Left Inverse")) {
        cube->addRotationCommand(R_LI);
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (ImGui::Button("Right")) {
        cube->addRotationCommand(R_R);
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Right Inverse")) {
        cube->addRotationCommand(R_RI);
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (ImGui::Button("Up")) {
        cube->addRotationCommand(R_U);
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Up Inverse")) {
        cube->addRotationCommand(R_UI);
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (ImGui::Button("Down")) {
        cube->addRotationCommand(R_D);
    }
    ImGui::TableNextColumn();
    if (ImGui::Button("Down Inverse")) {
        cube->addRotationCommand(R_DI);
    }

    ImGui::EndTable();
}

void RubiksCubeScene::tick() {
    static auto rotationSpeed = 0.1F;

    ImGui::Begin("Settings");
    ImGui::DragFloat("Animation Speed", &rotationSpeed, 0.001F, 0.001F, 1.0F);

    if (ImGui::Button("Shuffle")) {
        rubiksCube->shuffle();
    }
    if (ImGui::Button("Solve")) {
        rubiksCube->solve();
    }
    if (ImGui::Button("Pause")) {
        rubiksCube->togglePause();
    }

    showRotationButtons(rubiksCube);

    ImGui::End();

    shader->bind();
    vertexArray->bind();
    indexBuffer->bind();

    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", getCamera().getViewMatrix());
    shader->setUniform("projectionMatrix", getCamera().getProjectionMatrix());

    rubiksCube->rotate(rotationSpeed);

    for (unsigned int i = 0; i < rubiks::CUBELET_COUNT; i++) {
        shader->setUniform("cubeMatrix", rubiksCube->getCubeletRotationMatrix(i));
        const unsigned int vertexCountPerSmallCube = 36;
        const uint64_t count = i * vertexCountPerSmallCube * sizeof(unsigned int);
        GL_Call(
              glDrawElements(GL_TRIANGLES, vertexCountPerSmallCube, GL_UNSIGNED_INT, reinterpret_cast<void *>(count)));
    }

    indexBuffer->unbind();
    vertexArray->unbind();

    shader->unbind();
}
