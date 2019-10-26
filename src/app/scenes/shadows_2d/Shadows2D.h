#pragma once

#include <functional>
#include <memory>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "scenes/Scene.h"

#include "ray_tracing_2d/RayTracer2D.h"

struct DrawToggles {
    bool drawWireframe = false;
    bool drawLightSource = true;
    bool drawWalls = true;
    bool drawRays = false;
    bool drawIntersections = false;
    bool drawClosestIntersections = false;
    bool drawShadow = true;
    bool showShadowArea = true;
};

struct ColorConfig {
    glm::vec3 light = glm::vec3(0.8, 0.8, 0.8);
    glm::vec3 walls = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 rays = glm::vec3(1.0, 0.0, 0.0);
    glm::vec3 intersections = glm::vec3(1.0, 0.0, 1.0);
    glm::vec3 closestIntersections = glm::vec3(1.0, 1.0, 0.0);
    glm::vec3 lightSource = glm::vec3(1.0, 1.0, 1.0);
};

class Shadows2D : public Scene {
  public:
    Shadows2D(SceneData &data) : Scene(data, "Shadows2D"){};

    ~Shadows2D() override = default;

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
    std::vector<std::shared_ptr<VertexArray>> closestIntersectionVAs = {};
    std::shared_ptr<VertexArray> shadowPolygonVA;
    std::vector<RayTracer2D::Polygon> walls;
    std::vector<glm::vec2> circleVertices = {{0.0, 0.0}};
    std::vector<unsigned int> circleIndices = {0};

    void createLightSourceVA();

    void createWallVA();

    void createRaysAndIntersectionsVA(const std::vector<RayTracer2D::Ray> &rays, const DrawToggles &drawToggles,
                                      std::vector<glm::vec2> &shadowPolygon);

    void createShadowPolygonVA(std::vector<glm::vec2> &vertices, const glm::mat4 &viewMatrix,
                               const glm::vec2 &lightPosition, unsigned int &numVertices, unsigned int &numIndices);

    std::shared_ptr<VertexArray> createIntersectionVA(const glm::vec2 &intersection);

    void addWalls();
    RayTracer2D::Polygon createScreenBorder(float scale);

    void renderScene(const DrawToggles &drawToggles, const glm::mat4 &viewMatrix, const glm::vec2 &lightPosition,
                     const ColorConfig &colorConfig) const;

    static unsigned long getNumIntersections(const std::vector<RayTracer2D::Ray> &rays);

    void createCircleData();
    void renderShadow(const DrawToggles &drawToggles, const ColorConfig &colorConfig) const;
};
