#pragma once

#include "Scene.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "ray_tracing/RayTracer.h"

#include <functional>
#include <future>
#include <memory>
#include <util/TimeUtils.h>

class RayTracing : public Scene {
  public:
    RayTracing() : Scene("RayTracing"){};

    ~RayTracing() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

  private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> rayTracedTextureArray;
    std::shared_ptr<Texture> texture;
    glm::mat4 projectionMatrix;

    std::vector<RayTracer::Object> objects = {};
    RayTracer::Light light = {};
    std::future<void> currentResult = std::future<void>();

    void onAspectRatioChange() override;

    void renderRayTracedTexture(const std::vector<glm::vec3> &pixels, unsigned int width, unsigned int height,
                                const glm::vec3 &cameraPosition, float zDistance);
    void renderScene(const glm::vec3 &rayTracerCameraPosition, float zDistance, const std::vector<RayTracer::Ray> &rays,
                     const glm::vec3 &rayColor, bool shouldRenderRays);
    void renderLines(const glm::vec3 &rayTracerCameraPosition, float zDistance);
    void renderObject(const RayTracer::Object &object);
    void renderRays(const std::vector<RayTracer::Ray> &rays, const glm::vec3 &color);
    void setupRayTracedTexture();
    void renderCube(const glm::vec3 &position, const glm::vec3 &color);
};
