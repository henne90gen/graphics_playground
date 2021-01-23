#include "AtmosphericScattering.h"

#include <Main.h>
#include <util/RenderUtils.h>

DEFINE_SCENE_MAIN(AtmosphericScattering)

DEFINE_DEFAULT_SHADERS(atmospheric_scattering_AtmosphericScattering)
DEFINE_DEFAULT_SHADERS(atmospheric_scattering_Cube)

DEFINE_SHADER(atmospheric_scattering_NoiseLib)
DEFINE_SHADER(atmospheric_scattering_ScatterLib)

void AtmosphericScattering::setup() {
    getCamera().setFocalPoint({0.0F, 0.0F, 0.0F});

    cubeShader = CREATE_DEFAULT_SHADER(atmospheric_scattering_Cube);
    cubeShader->attachShaderLib(SHADER_CODE(atmospheric_scattering_ScatterLib));
    cubeVA = createCubeVA(cubeShader);

    terrainShader = CREATE_DEFAULT_SHADER(atmospheric_scattering_AtmosphericScattering);
    terrainShader->attachShaderLib(SHADER_CODE(atmospheric_scattering_NoiseLib));
    terrainShader->attachShaderLib(SHADER_CODE(atmospheric_scattering_ScatterLib));

    terrainVA = std::make_shared<VertexArray>(terrainShader);
    terrainVA->bind();
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
    terrainVA->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {};
    for (int i = 0; i < 100 * 100; i++) {
        if (i % 100 == 99 || i / 100 == 99) {
            continue;
        }
        indices.emplace_back(i + 100, i + 1, i);
        indices.emplace_back(i + 100, i + 101, i + 1);
    }
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    terrainVA->setIndexBuffer(indexBuffer);
}

void AtmosphericScattering::destroy() {}

void AtmosphericScattering::tick() {
    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Position", reinterpret_cast<float *>(&modelPosition), 0.01F);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    ImGui::Separator();
    ImGui::DragFloat3("Cube Position", reinterpret_cast<float *>(&cubePosition), 0.01F);
    ImGui::DragFloat3("Cube Rotation", reinterpret_cast<float *>(&cubeRotation), 0.01F);
    ImGui::DragFloat3("Cube Scale", reinterpret_cast<float *>(&cubeScale), 0.01F);
    ImGui::Separator();
    ImGui::DragFloat3("Light Direction", reinterpret_cast<float *>(&lightDirection), 0.1F);
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor));
    ImGui::DragFloat("Light Power", &lightPower, 0.1F);
    ImGui::Separator();
    ImGui::Checkbox("Show Fex", &showFex);
    ImGui::Checkbox("Use Fex", &useFex);
    ImGui::Checkbox("Show Lin", &showLin);
    ImGui::Checkbox("Use Lin", &useLin);
    ImGui::End();

    renderTerrain();
    renderSkyCube();
}

void AtmosphericScattering::renderTerrain() {
    auto modelMatrix = createModelMatrix(modelPosition, modelRotation, modelScale);
    setUniforms(terrainShader, modelMatrix);

    terrainVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, terrainVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void AtmosphericScattering::renderSkyCube() {
    auto modelMatrix = createModelMatrix(cubePosition, cubeRotation, cubeScale);
    setUniforms(cubeShader, modelMatrix);

    cubeVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, cubeVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void AtmosphericScattering::setUniforms(const std::shared_ptr<Shader> &shader, const glm::mat4 &modelMatrix) {
    shader->bind();

    auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
    shader->setUniform("viewMatrix", getCamera().getViewMatrix());
    shader->setUniform("projectionMatrix", getCamera().getProjectionMatrix());
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("normalMatrix", normalMatrix);

    shader->setUniform("showFex", showFex);
    shader->setUniform("useFex", useFex);
    shader->setUniform("showLin", showLin);
    shader->setUniform("useLin", useLin);

    glm::vec3 cameraPos = getCamera().getPosition();
    shader->setUniform("cameraPosition", cameraPos);
    shader->setUniform("lightDirection", lightDirection);
    shader->setUniform("lightColor", lightColor);
    shader->setUniform("lightPower", lightPower);
}
