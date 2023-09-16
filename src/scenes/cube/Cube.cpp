#include "Cube.h"

#include <array>

#include "Main.h"
#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(Cube)
DEFINE_DEFAULT_SHADERS(cube_Cube)

void Cube::setup() {
    getCamera().setDistance(5);
    shader = CREATE_DEFAULT_SHADER(cube_Cube);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<float> vertices = {
          // back
          -1.0F, -1.0F, -1.0F, 1, 0, 0, // 0
          1.0F, -1.0F, -1.0F, 0, 1, 0,  // 1
          1.0F, 1.0F, -1.0F, 0, 0, 1,   // 2
          -1.0F, 1.0F, -1.0F, 1, 1, 0,  // 3

          // front
          -1.0F, -1.0F, 1.0F, 1, 0, 1, // 4
          1.0F, -1.0F, 1.0F, 0, 1, 1,  // 5
          1.0F, 1.0F, 1.0F, 1, 1, 1,   // 6
          -1.0F, 1.0F, 1.0F, 0, 0, 0   // 7
    };
    BufferLayout bufferLayout = {{ShaderDataType::Float3, "position"}, {ShaderDataType::Float3, "color"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    vertexArray->addVertexBuffer(positionBuffer);

    std::vector<unsigned int> indices = {
          // front
          0, 1, 2, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          0, 2, 3, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // back
          4, 5, 6, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          4, 6, 7, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // right
          5, 1, 2, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          5, 2, 6, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // left
          0, 4, 7, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          0, 7, 3, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // top
          7, 6, 2, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          7, 2, 3, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

          // bottom
          4, 5, 1, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
          4, 1, 0, // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    };
    indexBuffer = std::make_shared<IndexBuffer>(indices);
}

void Cube::destroy() {}

void Cube::tick() {
    static auto translation = glm::vec3(0.0F, 0.0F, 0);
    static auto modelRotation = glm::vec3(0.5F, 0.8F, 0.0F);
    static float scale = 0.5F;

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&translation), 0.05F);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    ImGui::DragFloat("Model Scale", &scale, 0.001F);
    ImGui::End();

    shader->bind();
    vertexArray->bind();

    glm::mat4 modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, translation);
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale));

    glm::mat4 viewMatrix = getCamera().getViewMatrix();
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);

    indexBuffer->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    vertexArray->unbind();

    shader->unbind();
}
