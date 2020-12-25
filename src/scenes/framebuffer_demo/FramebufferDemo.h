#pragma once

#include "Scene.h"
#include "gl/Shader.h"
#include "gl/VertexArray.h"

#include <functional>

struct Object {
    std::shared_ptr<VertexArray> va;
    glm::vec3 position;
    glm::vec3 color;
    float scale;
};

class FramebufferDemo : public Scene {
  public:
    explicit FramebufferDemo() : Scene("FramebufferDemo"){};

    ~FramebufferDemo() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::vector<Object> objects = {};
    glm::mat4 projectionMatrix = glm::mat4(1.0);

    unsigned int fbo = 0;
    unsigned int mirrorTextureId = 0;
    std::shared_ptr<VertexArray> mirrorVA;

    void renderObjects(const glm::mat4 &viewMatrix);
    void renderMirror(const glm::mat4 &viewMatrix, const glm::vec3 &mirrorPosition, const glm::vec3 &mirrorRotation);
    void initFramebuffer();
};
