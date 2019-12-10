#pragma once

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "scenes/Scene.h"

#include <functional>

struct Object {
    std::shared_ptr<VertexArray> va;
    glm::vec3 position;
    glm::vec3 color;
    float scale;
};

class FramebufferDemo : public Scene {
  public:
    explicit FramebufferDemo(SceneData &data) : Scene(data, "FramebufferDemo"){};

    ~FramebufferDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::vector<Object> objects = {};
    glm::mat4 projectionMatrix = glm::mat4(1.0);

    GLuint fbo;

    void renderObjects(const glm::mat4 &viewMatrix);
};
