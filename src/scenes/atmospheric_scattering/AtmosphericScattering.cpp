#include "AtmosphericScattering.h"

#include <Main.h>
#include <util/RenderUtils.h>

constexpr float FIELD_OF_VIEW = 45.0F;
constexpr float Z_NEAR = 0.1F;
constexpr float Z_FAR = 10000.0F;

DEFINE_SCENE_MAIN(AtmosphericScattering)

DEFINE_DEFAULT_SHADERS(atmospheric_scattering_AtmosphericScattering)
DEFINE_SHADER(atmospheric_scattering_NoiseLib)

void AtmosphericScattering::setup() {
    shader = CREATE_DEFAULT_SHADER(atmospheric_scattering_AtmosphericScattering);
    shader->attachShaderLib(SHADER_CODE(atmospheric_scattering_NoiseLib));

    quadVA = std::make_shared<VertexArray>(shader);
    std::vector<glm::vec3> vertices = {};
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            vertices.emplace_back(i, 0, j);
        }
    }
    BufferLayout layout = {
          {ShaderDataType::Float3, "a_Position"},
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    quadVA->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {};
    for (int i = 0; i < 100 * 100; i++) {
        if (i % 100 == 99 || i / 100 == 99) {
            continue;
        }
        indices.emplace_back(i + 100, i + 1, i);
        indices.emplace_back(i + 100, i + 101, i + 1);
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    quadVA->setIndexBuffer(indexBuffer);
}

void AtmosphericScattering::destroy() {}

void AtmosphericScattering::tick() {
    static auto cameraPosition = glm::vec3(-55.0F, -75.0F, -85.0F);
    static auto cameraRotation = glm::vec3(0.4F, -1.01F, 0.0F);
    static auto modelPosition = glm::vec3(-50.0F, -5.0F, -15.0F);
    static auto modelRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    static auto modelScale = glm::vec3(1.0F);

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 0.1F);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&modelPosition), 0.01F);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    ImGui::End();

    shader->bind();
    quadVA->bind();

    auto projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    auto viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    auto modelMatrix = createModelMatrix(modelPosition, modelRotation, modelScale);
    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("viewMatrix", viewMatrix);
    shader->setUniform("projectionMatrix", projectionMatrix);
    shader->setUniform("normalMatrix", normalMatrix);

    glm::mat4 viewModel = inverse(viewMatrix);
    glm::vec3 cameraPos(viewModel[3] / viewModel[3][3]); // Might have to divide by w if you can't assume w == 1
    shader->setUniform("cameraPosition", cameraPos);

    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
