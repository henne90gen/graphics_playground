#pragma once

#include <functional>
#include <memory>

#include "scenes/Scene.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"

#include "ray_tracing_2d/RayTracer2D.h"

struct DrawToggles {
    bool drawWireframe = false;
    bool drawLightSource = true;
    bool drawWalls = true;
    bool drawRays = true;
    bool drawIntersectionPoints = true;
};

class RayTracing2D : public Scene {
public:
    RayTracing2D(SceneData &data)
            : Scene(data, "RayCasting2D") {};

    ~RayTracing2D() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

    void onAspectRatioChange() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> lightSourceVA;
    std::shared_ptr<VertexArray> wallsVA;
    std::shared_ptr<VertexArray> raysVA;
    std::vector<std::shared_ptr<VertexArray>> intersectionVAs = {};
    glm::mat4 projectionMatrix;
    std::vector<Polygon> walls;

    void createLightSourceVA();

    void createWallVA();

    void createRaysVA(const std::vector<Ray> &rays);

    void createIntersectionPointsVA(const std::vector<Ray> &rays);

    void addWalls();

    void renderScene(DrawToggles &drawToggles, const glm::mat4 &viewMatrix, const glm::mat4 &lightMatrix) const;
};
