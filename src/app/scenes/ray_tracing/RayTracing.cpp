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
}

void RayTracing::destroy() {}

void RayTracing::tick() {
    static unsigned int width = 1000;
    static unsigned int height = 1000;
    static glm::vec3 cameraPosition = {0, 0, 0};

    std::vector<glm::vec3> pixels = {};
    RayTracer::rayTrace(objects, lights, cameraPosition, pixels, width, height);

    texture->update(pixels, width, height);
    vertexArray->bind();
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
}
