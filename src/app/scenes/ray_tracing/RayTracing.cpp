#include "RayTracing.h"
#include <ray_tracing/RayTracer.h>

#include "ray_tracing/RayTracer.h"
#include "util/ImGuiUtils.h"
#include "util/RenderUtils.h"
#include "util/TimeUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.01F;
const float Z_FAR = 100.0F;

void RayTracing::setup() {
    shader =
          std::make_shared<Shader>("scenes/ray_tracing/RayTracingVert.glsl", "scenes/ray_tracing/RayTracingFrag.glsl");
    shader->bind();
    onAspectRatioChange();

    setupRayTracedTexture();

    light = {{1, 1, -2}, 0.75};
    objects.push_back(RayTracer::sphere({}, {}, 0.5));
    objects.push_back(RayTracer::sphere({0, 0, -2}, {1, 0, 0}, 0.5));
    objects.push_back(RayTracer::sphere({1, 0, -2}, {0, 1, 0}, 0.5));
    objects.push_back(RayTracer::sphere({0, 1, -2}, {0, 0, 1}, 0.5));
    objects.push_back(RayTracer::plane({0, 0, 5}, {1, 1, 0}, {0, 0, 1}));
}

void RayTracing::onAspectRatioChange() {
    auto projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("u_Projection", projectionMatrix);
}

void RayTracing::destroy() {}

void RayTracing::tick() {
    static int dimensions[2] = {250, 250};
    static glm::vec3 rayTracerCameraPosition = {0.5, 0.5, 0};
    static float zDistance = -1.0F;
    static glm::vec3 cameraPosition = {-5, 0, -2};
    static glm::vec3 cameraRotation = {0, -1, 0};
    static bool runAsync = false;
    float dragSpeed = 0.001F;

    ImGui::Begin("Settings");
    ImGui::DragInt2("Dimensions", dimensions, 1.0, 1, 1000);
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), dragSpeed);
    ImGui::DragFloat3("Position of Ray-Tracer Camera", reinterpret_cast<float *>(&rayTracerCameraPosition), dragSpeed);
    ImGui::DragFloat("Z-Distance", &zDistance, dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&light.position), dragSpeed);
    ImGui::Checkbox("Run Async", &runAsync);
    ImGui::End();

    // FIXME find a better solution for getting the light into the scene
    objects[0] = RayTracer::sphere(light.position, {1, 1, 1}, 0.1);

    std::vector<glm::vec3> pixels = {};
    unsigned int width = dimensions[0];
    unsigned int height = dimensions[1];
    {
        TIME_SCOPE_RECORD_NAME(perfCounter, "RayTrace");
        RayTracer::rayTrace(objects, light, rayTracerCameraPosition, zDistance, pixels, width, height, runAsync);
    }

    {
        TIME_SCOPE_RECORD_NAME(perfCounter, "Render");
        shader->bind();
        auto viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
        shader->setUniform("u_View", viewMatrix);

        renderRayTracedTexture(pixels, width, height, rayTracerCameraPosition, zDistance);
        renderScene(rayTracerCameraPosition, zDistance);
    }

    ImGui::Metrics(perfCounter);
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

void RayTracing::renderScene(const glm::vec3 &rayTracerCameraPosition, const float zDistance) {
    // render camera
    renderCube(rayTracerCameraPosition, {1, 1, 1});

    renderLines(rayTracerCameraPosition, zDistance);

    // render objects
    for (auto &object : objects) {
        renderObject(object);
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
    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    array.setIndexBuffer(indexBuffer);
    array.bind();

    shader->setUniform("u_UseTexture", false);
    shader->setUniform("u_Model", glm::mat4(1.0F));
    shader->setUniform("u_Color", {0, 1, 1});
    GL_Call(glDrawElements(GL_LINES, array.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void RayTracing::renderCube(const glm::vec3 &position, const glm::vec3 &color) {
    auto modelMatrix = glm::translate(glm::mat4(1.0F), position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1, 0.1, 0.1));

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
        array = createSphereVA(shader);
        float r = object.data.sphere.radius;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(r, r, r));
    }
    if (object.type == RayTracer::Object::Plane) {
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

    texture = std::make_shared<Texture>(GL_RGB);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("u_TextureSampler", 0);
}
