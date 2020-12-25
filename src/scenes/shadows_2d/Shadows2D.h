#pragma once

#include <functional>
#include <memory>

#include "Scene.h"
#include "gl/Shader.h"
#include "gl/VertexArray.h"

#include "RayTracer2D.h"

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
    Shadows2D() : Scene("Shadows2D"){};

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
    std::shared_ptr<VertexArray> intersectionVA;
    std::shared_ptr<VertexArray> closestIntersectionVA;
    std::shared_ptr<VertexArray> shadowPolygonVA;
    std::vector<RayTracer2D::Polygon> walls;
    std::vector<glm::vec2> circleVertices = {{0.0, 0.0}};
    std::vector<unsigned int> circleIndices = {0};
    glm::mat4 projectionMatrix;

    void createLightSourceVA();

    void createWallVA();

    void createRaysAndIntersectionsVA(const std::vector<RayTracer2D::Ray> &rays, const DrawToggles &drawToggles,
                                      std::vector<glm::vec2> &shadowPolygon);

    void createShadowPolygonVA(std::vector<glm::vec2> &vertices, const glm::mat4 &viewMatrix,
                               const glm::vec2 &lightPosition, unsigned int &numVertices, unsigned int &numIndices);

    void addIntersection(const glm::vec2 &intersection, std::vector<glm::vec2> &vertices,
                         std::vector<unsigned int> &indices, unsigned int &startingIndex);
    std::shared_ptr<VertexArray> createVertexArray(const std::vector<glm::vec2> &vertices,
                                                   const std::vector<unsigned int> &indices);

    void createWalls(const glm::ivec2 &wallCount);
    RayTracer2D::Polygon createScreenBorder(float scale);

    void renderScene(const DrawToggles &drawToggles, const glm::mat4 &viewMatrix, const glm::vec2 &lightPosition,
                     const ColorConfig &colorConfig);

    static unsigned long getNumIntersections(const std::vector<RayTracer2D::Ray> &rays);

    void createCircleData();
    void renderShadow(const DrawToggles &drawToggles, const ColorConfig &colorConfig) const;
};
