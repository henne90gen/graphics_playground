#pragma once

#include "Scene.h"

#include <functional>

#include "opengl/Model.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"

class BloomEffect : public Scene {
  public:
    BloomEffect() : Scene("BloomEffect"){};
    ~BloomEffect() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> shaderBlur;
    std::shared_ptr<Shader> shaderBloom;
    std::shared_ptr<Shader> textureShader;

    std::shared_ptr<VertexArray> quadVA;
    std::shared_ptr<VertexArray> cubeVA;
    std::shared_ptr<Texture> staticRedTexture;
    std::unique_ptr<Model> model = {};

    glm::mat4 projectionMatrix;

    unsigned int hdrFBO;
    unsigned int pingpongFBO[2];
    unsigned int finalFbo;

    unsigned int colorBuffers[2];
    unsigned int pingpongColorbuffers[2];
    unsigned int finalTexture;

    void setupFramebuffers();
    void setupHdrFramebuffer();
    void setupBlurringFramebuffers();
    void setupFinalFramebuffer();

    void renderSceneToFramebuffer(const glm::vec3 &modelPosition, const glm::vec3 &modelRotation,
                                  const glm::vec3 &lightPosition, float threshold);
    unsigned int blurRenderedScene();
    void applyBloomAndRenderAgain(unsigned int blurTexture, bool doBloom, bool doGammaCorrection, float exposure);
    void drawModel(const glm::mat4 &viewMatrix, const glm::vec3 &modelTranslation,
                   const glm::vec3 &modelRotation) const;
    void renderQuad(const std::shared_ptr<Shader> &s);
    void renderStepsOrFinal(bool drawSteps);
    void initLightCubeData();
    void drawLightCube(const glm::mat4 &viewMatrix, const glm::vec3 &lightPosition) const;
};
