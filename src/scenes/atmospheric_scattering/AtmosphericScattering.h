#pragma once

#include <Scene.h>

#include <functional>

#include <gl/Shader.h>
#include <gl/VertexArray.h>

class AtmosphericScattering : public Scene {
  public:
    explicit AtmosphericScattering() : Scene("AtmosphericScattering"){};
    ~AtmosphericScattering() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> terrainShader;
    std::shared_ptr<Shader> cubeShader;
    std::shared_ptr<VertexArray> terrainVA;
    std::shared_ptr<VertexArray> cubeVA;

    glm::vec3 modelPosition = glm::vec3(-50.0F, -50.0F, -50.0F);
    glm::vec3 modelRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    glm::vec3 modelScale = glm::vec3(1.0F);

    glm::vec3 cubePosition = glm::vec3(0.0F, 25.0F, 0.0F);
    glm::vec3 cubeRotation = glm::vec3(0.0F, 0.0F, 0.0F);
    glm::vec3 cubeScale = glm::vec3(100.0F);

    glm::vec3 lightDirection = glm::vec3(0.0F, -10.0F, 50.0F);
    glm::vec3 lightColor = glm::vec3(1.0F, 1.0F, 1.0F);
    float lightPower = 100.0F;

    bool showFex = false;
    bool useFex = true;
    bool showLin = false;
    bool useLin = true;

    void setUniforms(const std::shared_ptr<Shader> &shader, const glm::mat4 &modelMatrix);
    void renderTerrain();
    void renderSkyCube();
};
