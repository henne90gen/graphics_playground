#pragma once

#include <Scene.h>

#include <Model.h>
#include <functional>
#include <gl/IndexBuffer.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <memory>
#include <util/Camera.h>

#include "Layers.h"
#include "ShaderToggles.h"
#include "Sky.h"
#include "Terrain.h"
#include "Trees.h"

class Landscape : public Scene {
  public:
    explicit Landscape() : Scene("Landscape"){};

    ~Landscape() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

    void onAspectRatioChange() override;

  private:
    std::shared_ptr<VertexArray> quadVA;
    std::shared_ptr<VertexArray> cubeVA;

    std::shared_ptr<Shader> textureShader;
    std::shared_ptr<Shader> flatShader;
    std::shared_ptr<Shader> lightingShader;

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

    Camera playerCamera = {};
    Sky sky = {};
    Trees trees = {};
    Terrain terrain = {};

    void renderTerrain(bool renderQuad);
    void renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const glm::vec3 &lightPosition,
                     const glm::vec3 &lightColor);

    void renderTextureViewer();
    void renderTexture(const glm::vec3 &texturePosition, float zoom, const std::shared_ptr<Texture> &texture);

    void renderGBufferToQuad(const glm::vec3 &lightPosition, const glm::vec3 &lightColor, const glm::vec3 &sunDirection,
                             bool useAmbientOcclusion);
    void renderGBufferViewer();

    void initGBuffer();
    void initSSAOBuffer();
    void initSSAOBlurBuffer();
    void initKernelAndNoiseTexture();
};
