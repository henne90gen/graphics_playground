#pragma once

#include <functional>
#include <memory>

#include "AnimationRubiksCube.h"
#include "Scene.h"
#include "gl/IndexBuffer.h"
#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "gl/VertexBuffer.h"

class RubiksCubeScene : public Scene {
  public:
    explicit RubiksCubeScene() : Scene("RubiksCube"){};

    ~RubiksCubeScene() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<IndexBuffer> indexBuffer;

    float *vertices;
    unsigned int *indices;

    std::shared_ptr<rubiks::AnimationRubiksCube> rubiksCube;
};
