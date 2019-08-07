#include "RayTracing2D.h"

void RayTracing2D::setup() {
    GL_Call(glDisable(GL_DEPTH_TEST));

    shader = std::make_shared<Shader>("scenes/ray_tracing_2d/RayTracing2DVert.glsl",
                                      "scenes/ray_tracing_2d/RayTracing2DFrag.glsl");
    shader->bind();
    onAspectRatioChange();

    addWalls();

    createLightSourceVA();
    createWallVA();
}

void RayTracing2D::destroy() {
    GL_Call(glEnable(GL_DEPTH_TEST));
}

void RayTracing2D::onAspectRatioChange() {
    projectionMatrix = glm::ortho(-getAspectRatio(), getAspectRatio(), -1.0F, 1.0F);
    shader->setUniform("u_Projection", projectionMatrix);
}

void RayTracing2D::tick() {
    static DrawToggles drawToggles = {};
    static glm::vec3 cameraPosition = glm::vec3();
    static float zoom = 2.5F;
    static glm::vec2 lightPosition = glm::vec2();
    static float cutoff = 0.5F;

    ImGui::Begin("Settings");
    ImGui::Checkbox("Draw Wireframe", &drawToggles.drawWireframe);
    ImGui::Checkbox("Draw Light Source", &drawToggles.drawLightSource);
    ImGui::Checkbox("Draw Walls", &drawToggles.drawWalls);
    ImGui::Checkbox("Draw Rays", &drawToggles.drawRays);
    ImGui::Checkbox("Draw Intersection Points", &drawToggles.drawIntersectionPoints);
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), 0.001F);
    ImGui::DragFloat("Zoom", &zoom, 0.001F);
    ImGui::DragFloat2("Light Position", reinterpret_cast<float *>(&lightPosition), 0.001F);
    ImGui::DragFloat("Cutoff", &cutoff, 0.001F);
    ImGui::End();

    auto viewMatrix = createViewMatrix(cameraPosition, glm::vec3());
    viewMatrix = glm::scale(viewMatrix, glm::vec3(zoom, zoom, zoom));

    auto lightMatrix = glm::identity<glm::mat4>();
    lightMatrix = glm::translate(lightMatrix, glm::vec3(lightPosition.x, lightPosition.y, 0.0F));

    auto rays = RayTracer2D::calculateRays(walls, lightPosition, cutoff);
    createRaysVA(rays);
    createIntersectionPointsVA(rays);

    renderScene(drawToggles, viewMatrix, lightMatrix);
}

void
RayTracing2D::renderScene(DrawToggles &drawToggles, const glm::mat4 &viewMatrix, const glm::mat4 &lightMatrix) const {
    shader->bind();
    shader->setUniform("u_View", viewMatrix);
    if (drawToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    if (drawToggles.drawLightSource) {
        shader->setUniform("u_DrawMode", 0);
        shader->setUniform("u_Model", lightMatrix);
        lightSourceVA->bind();
        GL_Call(glDrawElements(GL_TRIANGLES, lightSourceVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (drawToggles.drawWalls) {
        shader->setUniform("u_DrawMode", 1);
        wallsVA->bind();
        GL_Call(glDrawElements(GL_TRIANGLES, wallsVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (drawToggles.drawRays) {
        shader->setUniform("u_DrawMode", 2);
        raysVA->bind();
        GL_Call(glDrawElements(GL_LINES, raysVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    }

    if (drawToggles.drawIntersectionPoints) {
        shader->setUniform("u_DrawMode", 2);
        for (auto &intersectionVA : intersectionVAs) {
            intersectionVA->bind();
            GL_Call(glDrawElements(GL_TRIANGLE_FAN, intersectionVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT,
                                   nullptr));
        }
    }

    if (drawToggles.drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }
}

void RayTracing2D::createLightSourceVA() {
    lightSourceVA = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {
            {-0.01, -0.01}, //
            {0.01,  -0.01}, //
            {0.01,  0.01}, //
            {-0.01, 0.01}
    };
    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"}
    };
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    lightSourceVA->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {
            {0, 1, 2}, //
            {0, 2, 3},
    };
    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    lightSourceVA->setIndexBuffer(indexBuffer);
}

void RayTracing2D::createWallVA() {
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

    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"}
    };
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    wallsVA->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    wallsVA->setIndexBuffer(indexBuffer);
}

void RayTracing2D::addWalls() {
    glm::vec2 offset = {-1.125F, -1.0F};
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            glm::vec2 position = glm::vec2(x * 0.25F, y * 0.25F);
            position += offset;
            float scale = 0.05;
            auto transformMatrix = glm::identity<glm::mat4>();
            transformMatrix = glm::translate(transformMatrix, glm::vec3(position.x, position.y, 0.0F));
            transformMatrix = glm::scale(transformMatrix, glm::vec3(scale, scale, scale));

            auto wall = Polygon();
            wall.transformMatrix = transformMatrix;
            wall.vertices = {
                    {-1, -1}, //
                    {1,  -1}, //
                    {1,  1}, //
                    {-1, 1}
            };
            wall.indices = {
                    {0, 1, 2}, //
                    {0, 2, 3},
            };
            walls.push_back(wall);
        }
    }
}

void RayTracing2D::createRaysVA(const std::vector<Ray> &rays) {
    raysVA = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {};
    std::vector<unsigned int> indices = {};
    unsigned int currentIndex = 0;
    for (auto &ray : rays) {
        vertices.push_back(ray.startingPoint);
        vertices.push_back(ray.startingPoint + ray.direction * 10.0F);
        indices.push_back(currentIndex);
        indices.push_back(currentIndex + 1);
        currentIndex += 2;
    }

    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"}
    };
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    raysVA->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    raysVA->setIndexBuffer(indexBuffer);
}

void RayTracing2D::createIntersectionPointsVA(const std::vector<Ray> &rays) {
    const float circleSize = 0.005F;
    std::vector<glm::vec2> circleVertices = {{0.0, 0.0}};
    std::vector<unsigned int> indices = {0};
    unsigned int circleIndex = 1;
    for (unsigned int i = 0; i < 360; i += 36) {
        auto rotationMatrix = glm::rotate(glm::identity<glm::mat4>(), glm::radians((float) i),
                                          glm::vec3(0.0, 0.0, 1.0));
        glm::vec4 direction = {1.0, 0.0, 0.0, 0.0};
        direction = rotationMatrix * direction;
        direction = glm::normalize(direction);
        auto pos = glm::vec2(direction.x, direction.y) * circleSize;
        circleVertices.push_back(pos);
        indices.push_back(circleIndex++);
    }
    indices.push_back(1);

    intersectionVAs = {};
    for (auto &ray : rays) {
        for (auto &intersection : ray.intersections) {
            std::vector<glm::vec2> vertices = {};
            vertices.reserve(circleVertices.size());
            for (auto &vertex : circleVertices) {
                vertices.push_back(vertex + intersection);
            }

            std::shared_ptr<VertexArray> intersectionVA = std::make_shared<VertexArray>(shader);
            BufferLayout layout = {
                    {ShaderDataType::Float2, "a_Position"}
            };
            std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
            intersectionVA->addVertexBuffer(vertexBuffer);

            std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
            intersectionVA->setIndexBuffer(indexBuffer);
            intersectionVAs.push_back(intersectionVA);
        }
    }
}
