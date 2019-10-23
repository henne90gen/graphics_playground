#include "Shadows2D.h"

#include "util/TimeUtils.h"

void Shadows2D::setup() {
    GL_Call(glDisable(GL_DEPTH_TEST));

    shader = std::make_shared<Shader>("scenes/shadows_2d/Shadows2DVert.glsl", "scenes/shadows_2d/Shadows2DFrag.glsl");
    shader->bind();
    onAspectRatioChange();

    addWalls();

    createLightSourceVA();
    createWallVA();

    createCircleData();

    glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
}

void Shadows2D::destroy() { GL_Call(glEnable(GL_DEPTH_TEST)); }

void Shadows2D::onAspectRatioChange() {
    projectionMatrix = glm::ortho(-getAspectRatio(), getAspectRatio(), -1.0F, 1.0F);
    shader->setUniform("u_Projection", projectionMatrix);
}

void Shadows2D::tick() {
    static std::shared_ptr<PerformanceCounter> performanceCounter = std::make_shared<PerformanceCounter>();
    static DrawToggles drawToggles = {};
    static ColorConfig colorConfig = {};
    static glm::vec2 lightPosition = glm::vec2();
    static glm::vec3 cameraPosition = glm::vec3();
    static float zoom = 2.5F;

    ImGui::Begin("Settings");

    ImGui::Checkbox("Draw Wireframe", &drawToggles.drawWireframe);
    ImGui::Checkbox("Draw Light Source", &drawToggles.drawLightSource);
    ImGui::Checkbox("Draw Walls", &drawToggles.drawWalls);
    ImGui::Checkbox("Draw Rays", &drawToggles.drawRays);
    ImGui::Checkbox("Draw Intersection Points", &drawToggles.drawIntersections);
    ImGui::Checkbox("Draw Closest Intersection Points", &drawToggles.drawClosestIntersections);
    ImGui::Checkbox("Draw Shadow", &drawToggles.drawShadow);
    ImGui::Checkbox("Show Shadow Area", &drawToggles.showShadowArea);

    ImGui::ColorEdit3("Light", reinterpret_cast<float *>(&colorConfig.light));
    ImGui::ColorEdit3("Light Source", reinterpret_cast<float *>(&colorConfig.lightSource));
    ImGui::ColorEdit3("Walls", reinterpret_cast<float *>(&colorConfig.walls));
    ImGui::ColorEdit3("Rays", reinterpret_cast<float *>(&colorConfig.rays));
    ImGui::ColorEdit3("Intersections", reinterpret_cast<float *>(&colorConfig.intersections));
    ImGui::ColorEdit3("Closest Intersections", reinterpret_cast<float *>(&colorConfig.closestIntersections));

    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 0.001F);
    ImGui::DragFloat("Zoom", &zoom, 0.001F);
    ImGui::DragFloat2("Light Position", reinterpret_cast<float *>(&lightPosition), 0.001F);

    ImGui::End();

    std::vector<RayTracer2D::Ray> rays = {};
    auto screenBorder = createScreenBorder(1.0F / zoom);
    {
        TIME_SCOPE_RECORD_NAME(performanceCounter, "rays");
        rays = RayTracer2D::calculateRays(walls, screenBorder, lightPosition);
    }

    auto viewMatrix = createViewMatrix(cameraPosition, glm::vec3());
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom, zoom, zoom));

    unsigned int numVertices = 0;
    unsigned int numIndices = 0;
    std::vector<glm::vec2> shadowPolygon = {};
    {
        TIME_SCOPE_RECORD_NAME(performanceCounter, "scene");
        createRaysAndIntersectionsVA(rays, drawToggles, shadowPolygon);
        createShadowPolygonVA(shadowPolygon, viewMatrix, lightPosition, numVertices, numIndices);
    }

    ImGui::Begin("Metrics");
    ImGui::Text("Num Rays: %lu", rays.size());
    unsigned long numIntersections = getNumIntersections(rays);
    ImGui::Text("Num Intersections: %lu", numIntersections);
    ImGui::Text("Rays Average Time: %f", performanceCounter->dataPoints["rays"].average);
    ImGui::Text("Rays Standard Deviation: %f", performanceCounter->dataPoints["rays"].standardDeviation);
    ImGui::Text("Scene Average Time: %f", performanceCounter->dataPoints["scene"].average);
    ImGui::Text("Scene Standard Deviation: %f", performanceCounter->dataPoints["scene"].standardDeviation);
    if (ImGui::Button("Reset Performance Counter")) {
        performanceCounter->reset();
    }
    ImGui::Text("Num Vertices: %u", numVertices);
    ImGui::Text("Num Indices: %u", numIndices);
    ImGui::End();

    renderScene(drawToggles, viewMatrix, lightPosition, colorConfig);
}

void Shadows2D::renderScene(const DrawToggles &drawToggles, const glm::mat4 &viewMatrix, const glm::vec2 &lightPosition,
                            const ColorConfig &colorConfig) const {
    auto lightMatrix = glm::identity<glm::mat4>();
    lightMatrix = glm::translate(lightMatrix, glm::vec3(lightPosition.x, lightPosition.y, 0.0F));

    shader->bind();
    shader->setUniform("u_View", viewMatrix);
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
        for (auto &intersectionVA : intersectionVAs) {
            intersectionVA->bind();
            GL_Call(glDrawElements(GL_TRIANGLE_FAN, intersectionVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT,
                                   nullptr));
        }
    }

    if (drawToggles.drawClosestIntersections) {
        shader->setUniform("u_Color", colorConfig.closestIntersections);
        for (auto &intersectionVA : closestIntersectionVAs) {
            intersectionVA->bind();
            GL_Call(glDrawElements(GL_TRIANGLE_FAN, intersectionVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT,
                                   nullptr));
        }
    }

    if (drawToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    if (drawToggles.drawShadow) {
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);
    }
}

void Shadows2D::renderShadow(const DrawToggles &drawToggles,
                             const ColorConfig &colorConfig) const {
    if (!drawToggles.drawShadow) {
        return;
    }

    // set up the stencil buffer for writing to it
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    shader->setUniform("u_Color", colorConfig.light);
    shadowPolygonVA->bind();
    GL_Call(glDrawElements(GL_TRIANGLE_FAN, shadowPolygonVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));

    // set up the stencil buffer for reading from it
    if (drawToggles.showShadowArea) {
        glStencilFunc(GL_EQUAL, 1, 0xFF);
    } else {
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    }
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
}

void Shadows2D::createLightSourceVA() {
    lightSourceVA = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {{-0.01, -0.01}, //
                                       {0.01, -0.01},  //
                                       {0.01, 0.01},   //
                                       {-0.01, 0.01}};
    BufferLayout layout = {{ShaderDataType::Float2, "a_Position"}};
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    lightSourceVA->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {
          {0, 1, 2}, //
          {0, 2, 3},
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

void Shadows2D::addWalls() {
    glm::vec2 offset = {-0.375F, -0.25F};
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 4; x++) {
            glm::vec2 position = glm::vec2((float)x * 0.25F, (float)y * 0.25F);
            position += offset;
            float scale = 0.05;
            auto transformMatrix = glm::identity<glm::mat4>();
            transformMatrix = glm::translate(transformMatrix, glm::vec3(position.x, position.y, 0.0F));
            transformMatrix = glm::scale(transformMatrix, glm::vec3(scale, scale, scale));

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
    intersectionVAs = {};
    closestIntersectionVAs = {};
    raysVA = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {};
    std::vector<unsigned int> indices = {};
    unsigned int currentIndex = 0;
    for (auto &ray : rays) {
        if (drawToggles.drawRays) {
            vertices.push_back(ray.startingPoint);
            vertices.push_back(ray.startingPoint + ray.direction * 10.0F);
            indices.push_back(currentIndex);
            indices.push_back(currentIndex + 1);
            currentIndex += 2;
        }

        for (auto &intersection : ray.intersections) {
            if (drawToggles.drawClosestIntersections && intersection == ray.closestIntersection) {
                continue;
            }
            auto va = createIntersectionVA(intersection);
            intersectionVAs.push_back(va);
        }

        if (!ray.intersections.empty()) {
            auto va = createIntersectionVA(ray.closestIntersection);
            closestIntersectionVAs.push_back(va);
            shadowPolygon.push_back(ray.closestIntersection);
        }
    }

    BufferLayout layout = {{ShaderDataType::Float2, "a_Position"}};
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    raysVA->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    raysVA->setIndexBuffer(indexBuffer);
}

std::shared_ptr<VertexArray> Shadows2D::createIntersectionVA(const glm::vec2 &intersection) {
    std::vector<glm::vec2> vertices = {};
    vertices.reserve(circleVertices.size());
    for (auto &vertex : circleVertices) {
        vertices.push_back(vertex + intersection);
    }

    std::shared_ptr<VertexArray> intersectionVA = std::make_shared<VertexArray>(shader);
    BufferLayout layout = {{ShaderDataType::Float2, "a_Position"}};
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    intersectionVA->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(circleIndices);
    intersectionVA->setIndexBuffer(indexBuffer);
    return intersectionVA;
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
    for (unsigned int i = 0; i < 360; i += 36) {
        auto rotationMatrix = glm::rotate(glm::identity<glm::mat4>(), glm::radians((float)i), glm::vec3(0.0, 0.0, 1.0));
        glm::vec4 direction = {1.0, 0.0, 0.0, 0.0};
        direction = rotationMatrix * direction;
        direction = glm::normalize(direction);
        auto pos = glm::vec2(direction.x, direction.y) * circleSize;
        circleVertices.push_back(pos);
        circleIndices.push_back(circleIndex++);
    }
    circleIndices.push_back(1);
}

void Shadows2D::createShadowPolygonVA(std::vector<glm::vec2> &vertices, const glm::mat4 &viewMatrix,
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
