#pragma once

#include "scenes/Scene.h"

#include "model_loading/ModelLoader.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Model.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "util/TimeUtils.h"

#include <functional>
#include <memory>

class ModelLoading : public Scene {
  public:
    explicit ModelLoading(SceneData data) : Scene(data, "ModelLoading"){};

    ~ModelLoading() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Model> glModel;
    glm::mat4 projectionMatrix;

    void drawModel(const glm::vec3 &translation, const glm::vec3 &modelRotation, const glm::vec3 &cameraRotation,
                   float scale, bool drawWireframe);

    void showSettings(bool &rotate, bool &rotateWithMouse, float &mouseRotationSpeed, glm::vec3 &translation,
                      glm::vec3 &modelRotation, glm::vec3 &cameraRotation, float &scale, bool &drawWireframe,
                      unsigned int &currentModel, std::vector<std::string> &paths, std::shared_ptr<Model> &renderModel);
};
