#pragma once

#include <Scene.h>

#include <functional>

#include <gl/Shader.h>
#include <gl/VertexArray.h>

class AmbientOcclusion : public Scene {
  public:
    explicit AmbientOcclusion() : Scene("AmbientOcclusion"){};
    ~AmbientOcclusion() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;

    std::shared_ptr<VertexArray> cube1;
    std::shared_ptr<VertexArray> cube2;
    std::shared_ptr<VertexArray> quadVA;

    GLuint fbo = 0;
    GLuint textureId = 0;

    void initFramebuffer();
};
