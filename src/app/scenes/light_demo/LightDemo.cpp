#include "LightDemo.h"

void LightDemo::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/light_demo/LightDemoVert.glsl",
                                      "../../../src/app/scenes/light_demo/LightDemoFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    BufferLayout bufferLayout = {
            {ShaderDataType::Float3, "a_Position"}
    };
    auto vertices = std::vector<glm::vec3>();
    vertexBuffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);

    auto indices = std::vector<glm::ivec3>();
    indexBuffer = std::make_shared<IndexBuffer>();
    vertexArray->setIndexBuffer(indexBuffer);
}

void LightDemo::destroy() {}

void LightDemo::tick() {}
