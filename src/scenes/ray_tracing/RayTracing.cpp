#include "RayTracing.h"

#include "Main.h"
#include "RayTracer.h"
#include "util/RenderUtils.h"
#include "util/TimeUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.01F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(RayTracing)
DEFINE_DEFAULT_SHADERS(ray_tracing_RayTracing)

void RayTracing::setup() {
    shader = CREATE_DEFAULT_SHADER(ray_tracing_RayTracing);
    shader->bind();
    onAspectRatioChange();

    setupRayTracedTexture();

    light = {{1, 1, -0.5}, 0.75};
    objects.push_back(RayTracer::sphere({}, {}, 0.5));

    objects.push_back(RayTracer::plane({-2, 0, 0}, {0, 1, 1}, {1, 0, 0}));
    objects.push_back(RayTracer::plane({0, 0, -5}, {1, 1, 0}, {0, 0, 1}));
    objects.push_back(RayTracer::plane({0, 2, 0}, {1, 0, 1}, {0, -1, 0}));
    objects.push_back(RayTracer::sphere({0, 0, -2}, {1, 0, 0}, 0.5, 1.0));
    objects.push_back(RayTracer::sphere({1, 0, -1.5}, {0, 1, 0}, 0.25));
    objects.push_back(RayTracer::sphere({0, 1, -1.5}, {0, 0, 1}, 0.25));
    objects.push_back(RayTracer::sphere({1, 1, -3}, {1, 0.5, 0.5}, 0.5));
}

void RayTracing::onAspectRatioChange() {
    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void RayTracing::destroy() {}

void RayTracing::tick() {
    static std::array<int, 2> dimensions = {250, 250};
    static glm::vec3 rayTracerCameraPosition = {0, 0.25, 1.0};
    static float zDistance = -2.1F;
    static bool runAsync = true;
    static glm::vec3 rayColor = {1, 1, 1};
    static bool shouldRenderRays = false;
    static int maxRayDepth = 5;
    static glm::vec3 glassSpherePosition = {0, 0, -2};
    const float dragSpeed = 0.001F;

    ImGui::Begin("Settings");
    ImGui::DragInt2("Dimensions", dimensions.data(), 1.0, 1, 1000);
    ImGui::DragFloat3("Position of Ray-Tracer Camera", reinterpret_cast<float *>(&rayTracerCameraPosition), dragSpeed);
    ImGui::DragFloat("Z-Distance", &zDistance, dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&light.position), dragSpeed);
    ImGui::Checkbox("Run Async", &runAsync);
    ImGui::Checkbox("Render Rays", &shouldRenderRays);
    ImGui::ColorEdit3("Ray Color", reinterpret_cast<float *>(&rayColor));
    ImGui::DragInt("Max Ray Depth", &maxRayDepth, 1.0F, 0, 100);
    ImGui::DragFloat3("Glass Sphere Position", reinterpret_cast<float *>(&glassSpherePosition), dragSpeed);
    ImGui::End();

    // FIXME find a better solution for getting the light into the scene
    const double lightRadius = 0.1;
    objects[0] = RayTracer::sphere(light.position, {1, 1, 1}, lightRadius);
    const int glassSphereIndex = 4;
    objects[glassSphereIndex] = RayTracer::sphere(glassSpherePosition, {1, 0, 0}, 0.5, 1.0);

    std::vector<glm::vec3> pixels = {};
    unsigned int width = dimensions[0];
    unsigned int height = dimensions[1];
    std::vector<RayTracer::Ray> rays = {};
    {
        RECORD_SCOPE_NAME("RayTrace");
        RayTracer::rayTrace(objects, light, rayTracerCameraPosition, zDistance, pixels, width, height, maxRayDepth,
                            rays, runAsync);
    }

    {
        RECORD_SCOPE_NAME("Render");
        shader->bind();
        auto viewMatrix = getCamera().getViewMatrix();
        shader->setUniform("u_View", viewMatrix);
        shader->setUniform("u_Projection", projectionMatrix);

        renderRayTracedTexture(pixels, width, height, rayTracerCameraPosition, zDistance);
        renderScene(rayTracerCameraPosition, zDistance, rays, rayColor, shouldRenderRays);
    }
}

void RayTracing::renderRayTracedTexture(const std::vector<glm::vec3> &pixels, const unsigned int width,
                                        const unsigned int height, const glm::vec3 &cameraPosition,
                                        const float zDistance) {
    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0F), cameraPosition);
    modelMatrix = glm::translate(modelMatrix, {0, 0, zDistance});

    shader->setUniform("u_UseTexture", true);
    shader->setUniform("u_Model", modelMatrix);
    texture->update(pixels, width, height);
    rayTracedTextureArray->bind();
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void RayTracing::renderScene(const glm::vec3 &rayTracerCameraPosition, const float zDistance,
                             const std::vector<RayTracer::Ray> &rays, const glm::vec3 &rayColor,
                             bool shouldRenderRays) {
    // render camera
    renderCube(rayTracerCameraPosition, {1, 1, 1});

    renderLines(rayTracerCameraPosition, zDistance);

    // render objects
    for (auto &object : objects) {
        renderObject(object);
    }

    if (shouldRenderRays) {
        renderRays(rays, rayColor);
    }
}

void RayTracing::renderLines(const glm::vec3 &rayTracerCameraPosition, const float zDistance) {
    VertexArray array = VertexArray(shader);
    const float factor = 100.0F;
    std::vector<glm::vec3> vertices = {
          rayTracerCameraPosition,                               //
          glm::normalize(glm::vec3(-1, -1, zDistance)) * factor, //
          glm::normalize(glm::vec3(1, -1, zDistance)) * factor,  //
          glm::normalize(glm::vec3(1, 1, zDistance)) * factor,   //
          glm::normalize(glm::vec3(-1, 1, zDistance)) * factor,  //
    };
    BufferLayout bufferLayout = {
          {Float3, "a_Position"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    array.addVertexBuffer(buffer);

    std::vector<unsigned int> indices = {
          0, 1, //
          0, 2, //
          0, 3, //
          0, 4, //
    };
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    array.setIndexBuffer(indexBuffer);
    array.bind();

    shader->setUniform("u_UseTexture", false);
    shader->setUniform("u_Model", glm::mat4(1.0F));
    shader->setUniform("u_Color", glm::vec3(0.0F, 1.0F, 1.0F));
    GL_Call(glDrawElements(GL_LINES, array.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void RayTracing::renderRays(const std::vector<RayTracer::Ray> &rays, const glm::vec3 &color) {
    std::vector<glm::vec3> vertices = {};
    std::vector<unsigned int> indices = {};

    unsigned int currentIndex = 0;
    const float factor = 100.0F;
    for (auto ray : rays) {
        vertices.push_back(ray.startingPoint);
        vertices.push_back(ray.startingPoint + glm::normalize(ray.direction) * factor);
        indices.push_back(currentIndex++);
        indices.push_back(currentIndex++);
    }

    VertexArray array = VertexArray(shader);
    BufferLayout bufferLayout = {
          {Float3, "a_Position"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    array.addVertexBuffer(buffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    array.setIndexBuffer(indexBuffer);
    array.bind();

    shader->setUniform("u_UseTexture", false);
    shader->setUniform("u_Model", glm::mat4(1.0F));
    shader->setUniform("u_Color", color);
    GL_Call(glDrawElements(GL_LINES, array.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void RayTracing::renderCube(const glm::vec3 &position, const glm::vec3 &color) {
    auto modelMatrix = glm::translate(glm::mat4(1.0F), position);
    const double modelScale = 0.1;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(modelScale, modelScale, modelScale));

    shader->setUniform("u_UseTexture", false);
    shader->setUniform("u_Model", modelMatrix);
    shader->setUniform("u_Color", color);
    std::shared_ptr<VertexArray> array = createCubeVA(shader);
    array->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, array->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void RayTracing::renderObject(const RayTracer::Object &object) {
    std::shared_ptr<VertexArray> array;
    auto modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::translate(modelMatrix, object.position);

    if (object.type == RayTracer::Object::Sphere) {
        array = Sphere().createVA(shader);
        float r = object.data.sphere.radius;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(r, r, r));
    }
    if (object.type == RayTracer::Object::Plane) {
        array = std::make_shared<VertexArray>(shader);
        std::vector<glm::vec3> vertices = {
              {-1, -1, 0}, //
              {1, -1, 0},  //
              {1, 1, 0},   //
              {-1, 1, 0},  //
        };
        BufferLayout bufferLayout = {
              {Float3, "a_Position"},
        };
        auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
        array->addVertexBuffer(buffer);

        std::vector<glm::ivec3> indices = {
              {0, 1, 2}, //
              {0, 2, 3}, //
        };
        array->setIndexBuffer(std::make_shared<IndexBuffer>(indices));
        glm::vec3 orig = {0, 0, 1};
        glm::quat rotation = glm::rotation(orig, object.data.plane.normal);
        const int modelScale = 10;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(modelScale, modelScale, modelScale));
        modelMatrix = modelMatrix * glm::toMat4(rotation);
    }

    if (array == nullptr) {
        std::cout << "Could not render object with type " << object.type << std::endl;
        return;
    }

    shader->setUniform("u_UseTexture", false);
    shader->setUniform("u_Color", object.color);
    shader->setUniform("u_Model", modelMatrix);
    array->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, array->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void RayTracing::setupRayTracedTexture() {
    rayTracedTextureArray = std::make_shared<VertexArray>(shader);
    std::vector<float> vertices = {
          -1, -1, 0, 0, 0, //
          1,  -1, 0, 1, 0, //
          1,  1,  0, 1, 1, //
          -1, -1, 0, 0, 0, //
          1,  1,  0, 1, 1, //
          -1, 1,  0, 0, 1, //
    };
    BufferLayout bufferLayout = {
          {Float3, "a_Position"},
          {Float2, "a_UV"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    rayTracedTextureArray->addVertexBuffer(buffer);

    texture = std::make_shared<Texture>();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("u_TextureSampler", 0);
}
