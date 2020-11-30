#pragma once

#include "Scene.h"

#include <functional>
#include <glad/glad.h>
#include <memory>

#include "opengl/IndexBuffer.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "util/TimeUtils.h"

#include "meta_balls/MetaBalls.h"

class MetaBallsScene : public Scene {
  public:
    enum MetaBallsFuncType { EXP = 0, INVERSE_DIST = 1, TEST_SPHERE = 2 };

    explicit MetaBallsScene() : Scene("MetaBalls"){};
    ~MetaBallsScene() override = default;

    void setup() override;
    void tick() override;
    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  private:
    std::shared_ptr<Shader> shader;

    std::shared_ptr<VertexArray> surfaceVertexArray;
    std::shared_ptr<VertexBuffer> surfaceVertexBuffer;
    std::shared_ptr<IndexBuffer> surfaceIndexBuffer;
    std::shared_ptr<VertexArray> bbVa;

    glm::mat4 projectionMatrix;

    void updateSurface(const glm::ivec3 &dimensions, MetaBallsFuncType funcType,
                       const std::vector<MetaBall> &metaballs);
    void drawSurface(const glm::mat4 &modelMatrix, bool drawWireframe);
    void drawBoundingBox(glm::mat4 modelMatrix, const glm::vec3 &modelCenter, const glm::ivec3 &dimensions);
};
