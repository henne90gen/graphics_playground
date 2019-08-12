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
    bool drawIntersections = true;
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
    std::vector<glm::vec2> circleVertices = {{0.0, 0.0}};
    std::vector<unsigned int> circleIndices = {0};


    void createLightSourceVA();

    void createWallVA();

    void createRaysAndIntersectionsVA(const std::vector<Ray> &rays, DrawToggles &drawToggles);

    void addIntersections(const Ray &ray);

    void addWalls();

    void renderScene(DrawToggles &drawToggles, const glm::mat4 &viewMatrix, const glm::mat4 &lightMatrix) const;

    static unsigned long getNumIntersections(const std::vector<Ray> &rays);

    void createCircleData();
};
