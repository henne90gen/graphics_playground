#include "Shadows2D.h"

#include <algorithm>

#include "util/ImGuiUtils.h"
#include "util/TimeUtils.h"

const int stencilMask = 0xFF;
const int inverseStencilMask = 0x00;

DEFINE_SHADER(shadows_2d_Shadows2D)

void Shadows2D::setup() {
    GL_Call(glDisable(GL_DEPTH_TEST));
    GL_Call(glEnable(GL_PRIMITIVE_RESTART));
    GL_Call(glPrimitiveRestartIndex(~0));

    shader = SHADER(shadows_2d_Shadows2D);
    shader->bind();
    onAspectRatioChange();

    createWalls(glm::ivec2(10, 10));

    createLightSourceVA();
    createWallVA();

    createCircleData();

    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
}

void Shadows2D::destroy() { GL_Call(glEnable(GL_DEPTH_TEST)); }

void Shadows2D::onAspectRatioChange() {
    projectionMatrix = glm::ortho(-getAspectRatio(), getAspectRatio(), -1.0F, 1.0F);
}

void showSettings(DrawToggles &drawToggles, ColorConfig &colorConfig, glm::vec3 &cameraPosition, float &zoom,
                  glm::vec2 &lightPosition, glm::ivec2 &wallCount, bool &runAsync) {
    const float dragSpeed = 0.001F;

    ImGui::Begin("Settings");

    ImGui::Checkbox("Draw Wireframe", &drawToggles.drawWireframe);
    ImGui::Checkbox("Draw Light Source", &drawToggles.drawLightSource);
    ImGui::Checkbox("Draw Walls", &drawToggles.drawWalls);
    ImGui::Checkbox("Draw Rays", &drawToggles.drawRays);
    ImGui::Checkbox("Draw Intersection Points", &drawToggles.drawIntersections);
    ImGui::Checkbox("Draw Closest Intersection Points", &drawToggles.drawClosestIntersections);
    ImGui::Checkbox("Draw Shadow", &drawToggles.drawShadow);
    ImGui::Checkbox("Show Shadow Area", &drawToggles.showShadowArea);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::ColorEdit3("Light", reinterpret_cast<float *>(&colorConfig.light));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::ColorEdit3("Light Source", reinterpret_cast<float *>(&colorConfig.lightSource));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::ColorEdit3("Walls", reinterpret_cast<float *>(&colorConfig.walls));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::ColorEdit3("Rays", reinterpret_cast<float *>(&colorConfig.rays));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::ColorEdit3("Intersections", reinterpret_cast<float *>(&colorConfig.intersections));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::ColorEdit3("Closest Intersections", reinterpret_cast<float *>(&colorConfig.closestIntersections));

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat("Zoom", &zoom, dragSpeed);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat2("Light Position", reinterpret_cast<float *>(&lightPosition), dragSpeed);

    ImGui::SliderInt2("Walls", reinterpret_cast<int *>(&wallCount), 0, 10);

    ImGui::Checkbox("Run async", &runAsync);

    ImGui::End();
}

void Shadows2D::tick() {
    static DrawToggles drawToggles = {};
    static ColorConfig colorConfig = {};
    static glm::vec2 lightPosition = glm::vec2();
    static glm::vec3 cameraPosition = glm::vec3();
    static glm::ivec2 wallCount = {10, 10};
    static float zoom = 2.5F; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static bool runAsync = true;

    glm::ivec2 prevWallCount = wallCount;
    showSettings(drawToggles, colorConfig, cameraPosition, zoom, lightPosition, wallCount, runAsync);
    if (prevWallCount != wallCount) {
        createWalls(wallCount);
    }

    std::vector<RayTracer2D::Ray> rays = {};
    auto screenBorder = createScreenBorder(1.0F / zoom);
    {
        RECORD_SCOPE_NAME("Rays");
        rays = RayTracer2D::calculateRays(walls, screenBorder, lightPosition, runAsync);
    }

    auto viewMatrix = createViewMatrix(cameraPosition, glm::vec3());
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom, zoom, zoom));

    unsigned int numVertices = 0;
    unsigned int numIndices = 0;
    std::vector<glm::vec2> shadowPolygon = {};
    {
        RECORD_SCOPE_NAME("Scene");
        createRaysAndIntersectionsVA(rays, drawToggles, shadowPolygon);
        createShadowPolygonVA(shadowPolygon, viewMatrix, lightPosition, numVertices, numIndices);
    }

    {
        RECORD_SCOPE_NAME("Render");
        renderScene(drawToggles, viewMatrix, lightPosition, colorConfig);
    }

    ImGui::Begin("Metrics");
    ImGui::Text("Num Rays: %lu", rays.size());
    unsigned long numIntersections = getNumIntersections(rays);
    ImGui::Text("Num Intersections: %lu", numIntersections);
    ImGui::Text("Num Vertices: %u", numVertices);
    ImGui::Text("Num Indices: %u", numIndices);
    ImGui::End();
}

void Shadows2D::renderScene(const DrawToggles &drawToggles, const glm::mat4 &viewMatrix, const glm::vec2 &lightPosition,
                            const ColorConfig &colorConfig) {
    auto lightMatrix = glm::identity<glm::mat4>();
    lightMatrix = glm::translate(lightMatrix, glm::vec3(lightPosition.x, lightPosition.y, 0.0F));

    shader->bind();
    shader->setUniform("u_View", viewMatrix);
    shader->setUniform("u_Projection", projectionMatrix);
    if (drawToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    renderShadow(drawToggles, colorConfig);

    if (drawToggles.drawLightSource) {
        shader->setUniform("u_Color", colorConfig.lightSource);
        shader->setUniform("u_Model", lightMatrix);
        lightSourceVA->bind();
        GL_Call(glDrawElements(GL_TRIANGLES, lightSourceVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
        shader->setUniform("u_Model", glm::mat4(1.F));
    }

    if (drawToggles.drawWalls) {
        shader->setUniform("u_Color", colorConfig.walls);
        wallsVA->bind();
        GL_Call(glDrawElements(GL_TRIANGLES, wallsVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (drawToggles.drawRays) {
        shader->setUniform("u_Color", colorConfig.rays);
        raysVA->bind();
        GL_Call(glDrawElements(GL_LINES, raysVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (drawToggles.drawIntersections) {
        shader->setUniform("u_Color", colorConfig.intersections);
        intersectionVA->bind();
        GL_Call(
              glDrawElements(GL_TRIANGLE_FAN, intersectionVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (drawToggles.drawClosestIntersections) {
        shader->setUniform("u_Color", colorConfig.closestIntersections);
        closestIntersectionVA->bind();
        GL_Call(glDrawElements(GL_TRIANGLE_FAN, closestIntersectionVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT,
                               nullptr));
    }

    if (drawToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    if (drawToggles.drawShadow) {
        glStencilMask(stencilMask);
        glDisable(GL_STENCIL_TEST);
    }
}

void Shadows2D::renderShadow(const DrawToggles &drawToggles, const ColorConfig &colorConfig) const {
    if (!drawToggles.drawShadow) {
        return;
    }

    // set up the stencil buffer for writing to it
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, stencilMask);
    glStencilMask(stencilMask);

    shader->setUniform("u_Color", colorConfig.light);
    shadowPolygonVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLE_FAN, shadowPolygonVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    // set up the stencil buffer for reading from it
    if (drawToggles.showShadowArea) {
        glStencilFunc(GL_EQUAL, 1, stencilMask);
    } else {
        glStencilFunc(GL_NOTEQUAL, 1, stencilMask);
    }
    glStencilMask(inverseStencilMask);
    glDisable(GL_DEPTH_TEST);
}

void Shadows2D::createLightSourceVA() {
    lightSourceVA = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {
          {-0.01, -0.01}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0.01, -0.01},  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0.01, 0.01},   // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {-0.01, 0.01},  // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    };
    BufferLayout layout = {{ShaderDataType::Float2, "a_Position"}};
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    lightSourceVA->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {
          {0, 1, 2}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
          {0, 2, 3}, // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    };
    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    lightSourceVA->setIndexBuffer(indexBuffer);
}

void Shadows2D::createWallVA() {
    wallsVA = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {};
    std::vector<glm::ivec3> indices = {};
    unsigned int currentIndex = 0;
    for (auto &wall : walls) {
        for (auto &vertex : wall.vertices) {
            auto transformedVertex = wall.transformMatrix * glm::vec4(vertex.x, vertex.y, 0.0F, 1.0F);
            vertices.emplace_back(transformedVertex.x, transformedVertex.y);
        }
        for (auto &index : wall.indices) {
            indices.emplace_back(index.x + currentIndex, index.y + currentIndex, index.z + currentIndex);
        }
        currentIndex += wall.vertices.size();
    }

    BufferLayout layout = {{ShaderDataType::Float2, "a_Position"}};
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    wallsVA->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    wallsVA->setIndexBuffer(indexBuffer);
}

void Shadows2D::createWalls(const glm::ivec2 &wallCount) {
    walls = {};
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    glm::vec2 offset = {-0.375F, -0.25F};
    const float wallScale = 0.02F;
    const float positionScale = 0.1F;
    for (int y = 0; y < wallCount.y; y++) {
        for (int x = 0; x < wallCount.x; x++) {
            glm::vec2 position =
                  glm::vec2(static_cast<float>(x) * positionScale, static_cast<float>(y) * positionScale);
            position += offset;
            auto transformMatrix = glm::identity<glm::mat4>();
            transformMatrix = glm::translate(transformMatrix, glm::vec3(position.x, position.y, 0.0F));
            transformMatrix = glm::scale(transformMatrix, glm::vec3(wallScale, wallScale, wallScale));

            auto wall = RayTracer2D::Polygon();
            wall.transformMatrix = transformMatrix;
            wall.vertices = {{-1, -1}, //
                             {1, -1},  //
                             {1, 1},   //
                             {-1, 1}};
            wall.indices = {
                  {0, 1, 2}, //
                  {0, 2, 3},
            };
            walls.push_back(wall);
        }
    }
}

RayTracer2D::Polygon Shadows2D::createScreenBorder(float scale) {
    auto transformMatrix = glm::identity<glm::mat4>();
    transformMatrix = glm::scale(transformMatrix, glm::vec3(scale, scale, scale));

    float x = getAspectRatio();
    RayTracer2D::Polygon wall = {};
    wall.transformMatrix = transformMatrix;
    wall.vertices = {{-x, -1.0}, //
                     {x, -1.0},  //
                     {x, 1.0},   //
                     {-x, 1.0}};
    wall.indices = {
          {0, 1, 2}, //
          {0, 2, 3},
    };
    return wall;
}

void Shadows2D::createRaysAndIntersectionsVA(const std::vector<RayTracer2D::Ray> &rays, const DrawToggles &drawToggles,
                                             std::vector<glm::vec2> &shadowPolygon) {
    std::vector<glm::vec2> rayVertices = {};
    std::vector<unsigned int> rayIndices = {};
    std::vector<glm::vec2> intersectionVertices = {};
    std::vector<unsigned int> intersectionIndices = {};
    std::vector<glm::vec2> closestIntersectionVertices = {};
    std::vector<unsigned int> closestIntersectionIndices = {};
    unsigned int currentIndex = 0;
    unsigned int intersectionIndex = 0;
    unsigned int closestIntersectionIndex = 0;
    {
        RECORD_SCOPE_NAME("Core Loop");
        for (auto &ray : rays) {
            RECORD_SCOPE_NAME("Core Loop Local");
            if (drawToggles.drawRays) {
                rayVertices.push_back(ray.startingPoint);
                const float scaleFactor = 10.0F;
                rayVertices.push_back(ray.startingPoint + ray.direction * scaleFactor);
                rayIndices.push_back(currentIndex);
                rayIndices.push_back(currentIndex + 1);
                currentIndex += 2;
            }

            for (auto &intersection : ray.intersections) {
                if (drawToggles.drawClosestIntersections && intersection == ray.closestIntersection) {
                    continue;
                }
                addIntersection(intersection, intersectionVertices, intersectionIndices, intersectionIndex);
            }

            if (!ray.intersections.empty()) {
                addIntersection(ray.closestIntersection, closestIntersectionVertices, closestIntersectionIndices,
                                closestIntersectionIndex);
                shadowPolygon.push_back(ray.closestIntersection);
            }
        }
    }

    raysVA = createVertexArray(rayVertices, rayIndices);
    intersectionVA = createVertexArray(intersectionVertices, intersectionIndices);
    closestIntersectionVA = createVertexArray(closestIntersectionVertices, closestIntersectionIndices);
}

void Shadows2D::addIntersection(const glm::vec2 &intersection, std::vector<glm::vec2> &vertices,
                                std::vector<unsigned int> &indices, unsigned int &startingIndex) {
    for (const auto &vertex : circleVertices) {
        vertices.push_back(vertex + intersection);
    }

    for (const auto &index : circleIndices) {
        indices.push_back(index + startingIndex);
    }
    startingIndex += circleVertices.size();
    indices.push_back(~0);
}

std::shared_ptr<VertexArray> Shadows2D::createVertexArray(const std::vector<glm::vec2> &vertices,
                                                          const std::vector<unsigned int> &indices) {
    std::shared_ptr<VertexArray> va = std::make_shared<VertexArray>(shader);
    BufferLayout layout = {{ShaderDataType::Float2, "a_Position"}};
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    va->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    va->setIndexBuffer(indexBuffer);
    return va;
}

unsigned long Shadows2D::getNumIntersections(const std::vector<RayTracer2D::Ray> &rays) {
    unsigned long result = 0;
    for (auto &ray : rays) {
        result += ray.intersections.size();
    }
    return result;
}

void Shadows2D::createCircleData() {
    const float circleSize = 0.005F;
    unsigned int circleIndex = 1;
    const unsigned int fullCircle = 360;
    const unsigned int stepWidth = 36;
    for (unsigned int i = 0; i < fullCircle; i += stepWidth) {
        auto rotationMatrix =
              glm::rotate(glm::identity<glm::mat4>(), glm::radians(static_cast<float>(i)), glm::vec3(0.0, 0.0, 1.0));
        glm::vec4 direction = {1.0, 0.0, 0.0, 0.0};
        direction = rotationMatrix * direction;
        direction = glm::normalize(direction);
        auto pos = glm::vec2(direction.x, direction.y) * circleSize;
        circleVertices.push_back(pos);
        circleIndices.push_back(circleIndex++);
    }
    circleIndices.push_back(1);
}

void Shadows2D::createShadowPolygonVA(std::vector<glm::vec2> &vertices, const glm::mat4 & /*viewMatrix*/,
                                      const glm::vec2 &lightPosition, unsigned int &numVertices,
                                      unsigned int &numIndices) {
    // sort all vertices in a circle
    std::sort(vertices.begin(), vertices.end(), [&lightPosition](const glm::vec2 &first, const glm::vec2 &second) {
        auto firstP = first - lightPosition;
        auto secondP = second - lightPosition;
        double radiansFirst = glm::atan(firstP.y, firstP.x);
        double radiansSecond = glm::atan(secondP.y, secondP.x);
        return radiansFirst < radiansSecond;
    });

    std::vector<unsigned int> indices = {};
    vertices.push_back(lightPosition);
    indices.push_back(vertices.size() - 1);
    for (unsigned long i = 0; i < vertices.size() - 1; i++) {
        indices.push_back(i);
    }
    indices.push_back(0);

    shadowPolygonVA = std::make_shared<VertexArray>(shader);
    BufferLayout layout = {{ShaderDataType::Float2, "a_Position"}};
    std::shared_ptr<VertexBuffer> buffer = std::make_shared<VertexBuffer>(vertices, layout);
    shadowPolygonVA->addVertexBuffer(buffer);

    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    shadowPolygonVA->setIndexBuffer(indexBuffer);

    numVertices = vertices.size();
    numIndices = indices.size();
}
