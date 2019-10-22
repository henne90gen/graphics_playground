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
    unsigned int width = 10;
    unsigned int height = 10;
    std::vector<glm::vec3> pixels = {};
    pixels.resize(width * height);
    pixels[0] = {1,1,1};
    pixels[2] = {1,1,1};
    texture->update(pixels, width, height);

    vertexArray->bind();
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));
}
