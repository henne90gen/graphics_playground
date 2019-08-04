#include "RayCasting2D.h"

void RayCasting2D::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/ray_casting_2d/RayCasting2DVert.glsl",
                                      "../../../src/app/scenes/ray_casting_2d/RayCasting2DFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);

    std::vector<glm::vec2> vertices = {};
    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"}
    };
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);

    std::vector<unsigned int> indices = {};
    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);
}

void RayCasting2D::destroy() {}

void RayCasting2D::tick() {
    shader->bind();
    vertexArray->bind();

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
