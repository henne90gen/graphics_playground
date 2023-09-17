#pragma once

#include "Scene.h"

#include "Model.h"
#include "gl/IndexBuffer.h"
#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "util/TimeUtils.h"

#include <functional>
#include <memory>

class ModelLoading : public Scene {
  public:
    explicit ModelLoading() : Scene("ModelLoading"){};

    ~ModelLoading() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    Model model;

    void drawModel(const glm::vec3 &modelRotation, float scale, bool drawWireframe);

    void showSettings(bool &rotate, glm::vec3 &modelRotation, float &scale, bool &drawWireframe,
                      unsigned int &currentModel, std::vector<std::string> &paths);
};
