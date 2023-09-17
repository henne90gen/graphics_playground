#pragma once

#include "Scene.h"

#include <functional>

#include <gl/Shader.h>
#include <gl/VertexArray.h>

struct ModelSpace {
    glm::vec3 position = {0.0F, 0.0F, 0.0F};
    glm::vec3 rotation = {0.0F, 0.0F, 0.0F};
    glm::vec3 scale = {1.0F, 1.0F, 1.0F};
};

struct TessellationLevels {
    glm::vec3 outer = glm::vec3(5.0F);
    float inner = 5.0F;
};

class Tessellation : public Scene {
  public:
    explicit Tessellation() : Scene("Tessellation"){};
    ~Tessellation() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> simpleShader;
    std::shared_ptr<VertexArray> va;

    void renderTessellatedQuad(bool drawWireframe, const ModelSpace &model, const TessellationLevels &levels);
};
