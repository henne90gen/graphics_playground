#pragma once

#include <functional>
#include <memory>

#include "RubiksCube.h"
#include "RubiksCubeLogic.h"
#include "Scene.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"

class RubiksCubeScene : public Scene {
  public:
    explicit RubiksCubeScene() : Scene("RubiksCube"){};

    ~RubiksCubeScene() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<IndexBuffer> indexBuffer;

    glm::mat4 projectionMatrix;

    float *vertices;
    unsigned int *indices;

    std::shared_ptr<RubiksCube> rubiksCube;
};
