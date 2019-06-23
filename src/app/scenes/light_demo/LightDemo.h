#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>

#include "opengl/Shader.h"
#include "opengl/Model.h"
//#include "opengl/VertexArray.h"
//#include "opengl/VertexBuffer.h"
//#include "opengl/IndexBuffer.h"

class LightDemo : public Scene {
public:
    LightDemo(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "LightDemo") {};

    ~LightDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::unique_ptr<Model> model = {};

    bool drawWireframe = false;
    glm::mat4 projectionMatrix;

    void
    drawModel(float scale, const glm::vec3 &modelTranslation, const glm::vec3 &modelRotation,
              const glm::vec3 &cameraRotation, const glm::vec3 &cameraTranslation) const;

    void showSettings(glm::vec3 &cameraTranslation, glm::vec3 &cameraRotation, glm::vec3 &modelTranslation,
                      glm::vec3 &modelRotation, float &scale, glm::vec3 &lightPosition, glm::vec3 &lightColor) const;
};
