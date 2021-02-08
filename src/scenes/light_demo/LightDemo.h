#pragma once

#include "Scene.h"

#include <functional>
#include <memory>

#include "Model.h"
#include "gl/Shader.h"

class LightDemo : public Scene {
  public:
    explicit LightDemo() : Scene("LightDemo"){};

    ~LightDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    Model model = {};

    bool drawWireframe = false;
    glm::mat4 projectionMatrix;

    void drawModel(float scale, const glm::vec3 &modelTranslation, const glm::vec3 &modelRotation,
                   const glm::vec3 &cameraRotation, const glm::vec3 &cameraTranslation) const;

    static void showSettings(glm::vec3 &cameraTranslation, glm::vec3 &cameraRotation, glm::vec3 &modelTranslation,
                             glm::vec3 &modelRotation, float &scale, glm::vec3 &ambientColor, glm::vec3 &specularColor,
                             glm::vec3 &lightPosition, glm::vec3 &lightColor, bool &rotate, bool &useAmbient,
                             bool &useDiffuse, bool &useSpecular);
};
