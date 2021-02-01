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
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;

    std::shared_ptr<VertexArray> cube;
    std::shared_ptr<VertexArray> quadVA;

    GLuint fbo = 0;
    GLuint textureId = 0;
    GLuint rbo = 0;

    void renderSceneToFramebuffer(const glm::vec3 &position1, const glm::vec3 &position2,
                                  const glm::vec3 &lightPosition);
    void initFramebuffer();
};
