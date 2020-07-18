#pragma once

#include "scenes/Scene.h"

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

    explicit MetaBallsScene(SceneData &data) : Scene(data, "MetaBalls"){};
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

    glm::mat4 projectionMatrix;

    void updateSurface(const glm::ivec3 &dimensions, MetaBallsFuncType funcType,
                       const std::vector<MetaBall> &metaballs);
    void drawSurface(bool drawWireframe) const;
};
