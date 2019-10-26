#include "RayTracing.h"

void RayTracing::setup() {
    shader =
          std::make_shared<Shader>("scenes/ray_tracing/RayTracingVert.glsl", "scenes/ray_tracing/RayTracingFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    std::vector<glm::vec2> vertices = {
          {-1, -1}, {0, 0}, //
          {1, -1},  {1, 0}, //
          {1, 1},   {1, 1}, //
          {-1, -1}, {0, 0}, //
          {1, 1},   {1, 1}, //
          {-1, 1},  {0, 1}, //
    };
    BufferLayout bufferLayout = {
          {ShaderDataType::Float2, "a_Position"},
          {ShaderDataType::Float2, "a_UV"},
    };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    vertexArray->addVertexBuffer(buffer);

    texture = std::make_shared<Texture>(GL_RGB);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("u_TextureSampler", 0);

    lights.push_back({{0, 0, 2}, 0.75});
    objects.push_back(RayTracer::sphere({0, 0, 2}, {1, 0, 0}, 0.5));
}

void RayTracing::destroy() {}

void RayTracing::tick() {
    static int dimensions[2] = {500, 500};
    static glm::vec3 cameraPosition = {0, 0, 0};
    static int zDistance = 250;
    float dragSpeed = 0.001F;

    ImGui::Begin("Settings");
    ImGui::DragInt2("Dimensions", dimensions, 1.0, 1, 1000);
    ImGui::DragInt("Z-Distance", &zDistance, 1.0, 1, 1000);
    ImGui::DragFloat3("Camera Position", reinterpret_cast<float *>(&cameraPosition), dragSpeed);
    ImGui::DragFloat3("Light Position", reinterpret_cast<float *>(&lights[0].position), dragSpeed);
    ImGui::End();

    std::vector<glm::vec3> pixels = {};
    unsigned int width = dimensions[0];
    unsigned int height = dimensions[1];
    RayTracer::rayTrace(objects, lights, cameraPosition, pixels, width, height, zDistance);

    texture->update(pixels, width, height);
    vertexArray->bind();
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
}
