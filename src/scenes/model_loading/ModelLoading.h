#pragma once

#include "Scene.h"

#include "Model.h"
#include "ModelLoader.h"
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
    std::shared_ptr<Model> glModel;

    void drawModel(const glm::vec3 &translation, const glm::vec3 &modelRotation, float scale, bool drawWireframe);

    static void showSettings(bool &rotate, bool &rotateWithMouse, float &mouseRotationSpeed, glm::vec3 &translation,
                             glm::vec3 &modelRotation, float &scale, bool &drawWireframe, unsigned int &currentModel,
                             std::vector<std::string> &paths, std::shared_ptr<Model> &renderModel);
};
