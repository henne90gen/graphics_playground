#pragma once

#include <Scene.h>

#include <Model.h>
#include <camera/Camera.h>
#include <functional>
#include <gl/IndexBuffer.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <memory>

#include "Clouds.h"
#include "Layers.h"
#include "ShaderToggles.h"
#include "Terrain.h"
#include "Trees.h"

struct Light {
    glm::vec3 fragmentToLightDir = glm::vec3(0.0F, 1.0F, 0.0F);
    glm::vec3 color = glm::vec3(1.0F);

    float ambient = 0.1F;
    float diffuse = 1.0F;
    float specular = 0.3F;

    float distance = 100.0F;
    float scale = 10.0F;
};

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

    std::shared_ptr<Shader> flatShader;
    std::shared_ptr<Shader> textureShader;
    std::shared_ptr<Shader> lightingShader;
    std::shared_ptr<Shader> ssaoShader;
    std::shared_ptr<Shader> ssaoBlurShader;

    GLuint gBuffer = 0;
    GLuint gPosition = 0;
    GLuint gNormal = 0;
    GLuint gAlbedo = 0;
    GLuint gDoLighting = 0;
    GLuint depthBuffer = 0;

    GLuint ssaoFbo = 0;
    GLuint ssaoColorBuffer = 0;

    GLuint ssaoBlurFbo = 0;
    GLuint ssaoColorBlurBuffer = 0;

    std::vector<glm::vec3> ssaoKernel = {};
    unsigned int ssaoNoiseTexture = 0;

    Camera playerCamera = {};
    Clouds sky = {};
    Trees trees = {};
    Terrain terrain = {};
    glm::vec3 atmosphere = glm::vec3(0.4F, 0.45F, 1.2F);

    void renderTerrain(const Camera &camera, const Light &light, const ShaderToggles &shaderToggles);
    void renderLight(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix, const Light &light);
    void renderSky(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix);
    void renderSSAO();
    void renderSSAOBlur();
    void renderGBufferToQuad(const Camera &camera, const Light &light, const ShaderToggles &shaderToggles);
    void renderGBufferViewer();

    void renderTextureViewer();
    void renderTexture(const glm::vec3 &texturePosition, float zoom, unsigned int texture);

    void initGBuffer();
    void initSSAOBuffer();
    void initSSAOBlurBuffer();
    void initKernelAndNoiseTexture();
};
