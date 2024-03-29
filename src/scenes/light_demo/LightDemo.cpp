#include "LightDemo.h"

#include "Main.h"
#include "Model.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(LightDemo)
DEFINE_DEFAULT_SHADERS(light_demo_LightDemo)

void LightDemo::setup() {
    shader = CREATE_DEFAULT_SHADER(light_demo_LightDemo);
    shader->bind();
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    auto error = Model::loadFromFile("light_demo_resources/models/monkey.obj", shader, model);
    if (error != 0) {
        std::cout << "Failed to load model" << std::endl;
    }
}

void LightDemo::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void LightDemo::destroy() {}

void LightDemo::tick() {
    static auto modelTranslation = glm::vec3();
    static auto modelRotation = glm::vec3();
    static float scale = 1.0F;
    static auto ambientColor = glm::vec3(0.05F, 0.05F, 0.05F);
    static auto specularColor = glm::vec3(0.3F, 0.3F, 0.3F);
    static auto lightPosition = glm::vec3(0.0F, -0.7F, 1.6F);
    static auto lightColor = glm::vec3(1.0F);
    static bool rotate = true;
    static bool useAmbient = true;
    static bool useDiffuse = true;
    static bool useSpecular = true;

    // TODO(henne): make 'shininess' configurable (how much specular reflection the surface has)

    static const float rotationSpeed = 0.01F;
    if (rotate) {
        modelRotation.y += rotationSpeed;
    }

    showSettings(modelTranslation, modelRotation, scale, ambientColor, specularColor, lightPosition, lightColor, rotate,
                 useAmbient, useDiffuse, useSpecular);

    shader->bind();
    shader->setUniform("u_AmbientColor", ambientColor);
    shader->setUniform("u_SpecularColor", specularColor);
    shader->setUniform("u_Light.position", lightPosition);
    shader->setUniform("u_Light.color", lightColor);
    shader->setUniform("u_UseAmbient", useAmbient);
    shader->setUniform("u_UseDiffuse", useDiffuse);
    shader->setUniform("u_UseSpecular", useSpecular);

    glm::mat4 viewMatrix = getCamera().getViewMatrix();
    drawModel(scale, modelTranslation, modelRotation, viewMatrix);
}

void LightDemo::showSettings(glm::vec3 &modelTranslation, glm::vec3 &modelRotation, float &scale,
                             glm::vec3 &ambientColor, glm::vec3 &specularColor, glm::vec3 &lightPosition,
                             glm::vec3 &lightColor, bool &rotate, bool &useAmbient, bool &useDiffuse,
                             bool &useSpecular) {
    const float dragSpeed = 0.01F;
    ImGui::Begin("Settings");
    ImGui::DragFloat3("Model Translation", reinterpret_cast<float *>(&modelTranslation), dragSpeed);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), dragSpeed);
    ImGui::DragFloat("Model Scale", &scale, dragSpeed);
    ImGui::ColorEdit3("Ambient Color", reinterpret_cast<float *>(&ambientColor));
    ImGui::ColorEdit3("Specular Color", reinterpret_cast<float *>(&specularColor));
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lightPosition), dragSpeed);
    ImGui::ColorEdit3("Light Color", reinterpret_cast<float *>(&lightColor));
    ImGui::Checkbox("Rotate", &rotate);
    ImGui::Checkbox("Ambient", &useAmbient);
    ImGui::Checkbox("Diffuse", &useDiffuse);
    ImGui::Checkbox("Specular", &useSpecular);
    ImGui::End();
}

void LightDemo::drawModel(float scale, const glm::vec3 &modelTranslation, const glm::vec3 &modelRotation,
                          const glm::mat4 &viewMatrix) const {
    if (!model.isLoaded()) {
        return;
    }

    shader->bind();
    for (const auto &mesh : model.getMeshes()) {
        if (!mesh.visible) {
            continue;
        }

        mesh.vertexArray->bind();

        glm::mat4 modelMatrix = glm::mat4(1.0F);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, modelTranslation);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        shader->setUniform("u_Model", modelMatrix);
        shader->setUniform("u_NormalMatrix", normalMatrix);
        shader->setUniform("u_View", viewMatrix);
        shader->setUniform("u_Projection", projectionMatrix);

        shader->setUniform("u_TextureSampler", 0);
        mesh.texture->bind();

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(glDrawElements(GL_TRIANGLES, mesh.indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh.vertexArray->unbind();
    }
    shader->unbind();
}
