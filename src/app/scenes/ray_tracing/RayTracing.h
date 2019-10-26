#pragma once

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "ray_tracing/RayTracer.h"
#include "scenes/Scene.h"

#include <functional>
#include <memory>
#include <util/TimeUtils.h>

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
    std::shared_ptr<PerformanceCounter> perfCounter = std::make_shared<PerformanceCounter>();

    std::vector<RayTracer::Object> objects = {};
    RayTracer::Light light = {};

    void onAspectRatioChange() override;

    void renderRayTracedTexture(const std::vector<glm::vec3> &vector, unsigned int width, unsigned int height,
                                int zDistance, const glm::vec3 &cameraPosition);
    void renderScene(const glm::vec3 &rayTracerCameraPosition);
    void renderObject(const RayTracer::Object &object);
    void setupRayTracedTexture();
    void renderCube(const glm::vec3 &position, const glm::vec3 &color);
    RayTracer::Object convertLightToSphere();
};
