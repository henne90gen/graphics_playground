#pragma once

#include "Scene.h"

#include <functional>
#include <glad/glad.h>
#include <memory>

#include <ImGuiUtilsFastNoise.h>

#include "MarchingCubes.h"
#include "gl/IndexBuffer.h"
#include "gl/Shader.h"
#include "gl/Texture.h"
#include "gl/VertexArray.h"
#include "gl/VertexBuffer.h"
#include "util/TimeUtils.h"

class MarchingCubesScene : public Scene {
  public:
    explicit MarchingCubesScene() : Scene("MarchingCubes"){};

    ~MarchingCubesScene() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  protected:
    void onAspectRatioChange() override;

  public:
    std::shared_ptr<Shader> shader;

    std::shared_ptr<VertexArray> cubeVertexArray;
    std::shared_ptr<IndexBuffer> cubeIndexBuffer;

    std::shared_ptr<VertexArray> surfaceVertexArray;
    std::shared_ptr<VertexBuffer> surfaceVertexBuffer;
    std::shared_ptr<IndexBuffer> surfaceIndexBuffer;

    std::shared_ptr<MarchingCubes> marchingCubes;

    glm::mat4 projectionMatrix;

    void drawCube() const;

    void drawSurface(bool drawWireframe) const;

    void showSettings(glm::vec3 &translation, glm::vec3 &cameraRotation, glm::vec3 &modelRotation, float &scale,
                      bool &rotate, bool &drawWireframe) const;
};
