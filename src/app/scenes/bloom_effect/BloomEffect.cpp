#include "BloomEffect.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

void BloomEffect::setup() {
    shader = std::make_shared<Shader>("scenes/bloom_effect/BloomEffectVert.glsl",
                                      "scenes/bloom_effect/BloomEffectFrag.glsl");
    shader->bind();
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    model = std::make_unique<Model>();
    model->loadFromFile("scenes/light_demo/models/monkey.obj", shader);
}

void BloomEffect::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void BloomEffect::destroy() {}

void BloomEffect::tick() {
    static auto cameraTranslation = glm::vec3(0.5F, 0.0F, -5.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto cameraRotation = glm::vec3();
    static auto modelTranslation = glm::vec3();
    static auto modelRotation = glm::vec3();
    static float scale = 1.0F;
    static bool drawWireframe = false;
    static auto ambientColor = glm::vec3(0.05F, 0.05F, 0.05F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto specularColor = glm::vec3(0.3F, 0.3F, 0.3F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto lightPosition = glm::vec3(0.0F, -0.7F, 1.6F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto lightColor = glm::vec3(1.0F);

    shader->bind();
    shader->setUniform("u_AmbientColor", ambientColor);
    shader->setUniform("u_SpecularColor", specularColor);
    shader->setUniform("u_Light.position", lightPosition);
    shader->setUniform("u_Light.color", lightColor);
    drawModel(scale, modelTranslation, modelRotation, cameraRotation, cameraTranslation, drawWireframe);
}

void BloomEffect::drawModel(float scale, const glm::vec3 &modelTranslation, const glm::vec3 &modelRotation,
                          const glm::vec3 &cameraRotation, const glm::vec3 &cameraTranslation, const bool drawWireframe) const {
    shader->bind();
    for (auto &mesh : model->getMeshes()) {
        if (!mesh->visible) {
            continue;
        }

        mesh->vertexArray->bind();

        glm::mat4 modelMatrix = glm::mat4(1.0F);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, modelTranslation);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        glm::mat4 viewMatrix = createViewMatrix(cameraTranslation, cameraRotation);
        shader->setUniform("u_Model", modelMatrix);
        shader->setUniform("u_NormalMatrix", normalMatrix);
        shader->setUniform("u_View", viewMatrix);
        shader->setUniform("u_Projection", projectionMatrix);

        shader->setUniform("u_TextureSampler", 0);
        mesh->texture->bind();

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(glDrawElements(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh->vertexArray->unbind();
    }
    shader->unbind();
}
