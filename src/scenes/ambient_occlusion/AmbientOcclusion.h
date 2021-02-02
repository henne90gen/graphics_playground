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
    std::shared_ptr<Shader> textureShader;
    std::shared_ptr<Shader> geometryShader;
    std::shared_ptr<Shader> ssaoShader;
    std::shared_ptr<Shader> ssaoBlurShader;
    std::shared_ptr<Shader> lightingShader;

    std::shared_ptr<VertexArray> cube;
    std::shared_ptr<VertexArray> quadVA;

    GLuint gBuffer = 0;
    GLuint gPosition = 0;
    GLuint gNormal = 0;
    GLuint gAlbedo = 0;
    GLuint depthBuffer = 0;

    GLuint ssaoFbo = 0;
    GLuint ssaoColorBuffer = 0;

    GLuint ssaoBlurFbo = 0;
    GLuint ssaoColorBlurBuffer = 0;

    std::vector<glm::vec3> ssaoKernel = {};
    unsigned int ssaoNoiseTexture = 0;

    void renderSceneToGBuffer(const glm::vec3 &position1, const glm::vec3 &position2, const glm::vec3 &lightPosition);
    void renderSSAO();
    void renderSSAOBlur();
    void renderGBufferToQuad(const glm::vec3 &lightPosition, const glm::vec3 &lightColor, bool useAmbientOcclusion,
                             bool usePointLight);
    void renderTexture(unsigned int textureId);

    void initGBuffer();
    void initSSAOBuffer();
    void initSSAOBlurBuffer();
    void initKernelAndNoiseTexture();
};
