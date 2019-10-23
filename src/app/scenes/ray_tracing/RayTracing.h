#pragma once

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/Texture.h"
#include "scenes/Scene.h"
#include "ray_tracing/RayTracer.h"

#include <functional>
#include <memory>

class RayTracing : public Scene {
  public:
    RayTracing(SceneData &data) : Scene(data, "RayTracing"){};

    ~RayTracing() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;

    std::vector<RayTracer::Object> objects = {};
    std::vector<RayTracer::Light> lights = {};
};
