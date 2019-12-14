#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Model.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"

class BloomEffect : public Scene {
  public:
    BloomEffect(SceneData &data) : Scene(data, "BloomEffect"){};
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
    std::unique_ptr<Model> model = {};

    glm::mat4 projectionMatrix;

    unsigned int hdrFBO;
    unsigned int pingpongFBO[2];
    unsigned int finalFbo;

    unsigned int colorBuffers[2];
    unsigned int pingpongColorbuffers[2];
    unsigned int finalTexture;

    void drawModel(float scale, const glm::vec3 &modelTranslation, const glm::vec3 &modelRotation,
                   const glm::vec3 &cameraRotation, const glm::vec3 &cameraTranslation, bool drawWireframe) const;

    void renderQuad(const std::shared_ptr<Shader> &s);
    void renderAllQuads(bool drawSteps);
};
