#include "FourierTransform.h"

void FourierTransform::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/fourier_transform/FourierTransformVert.glsl",
                                      "../../../src/app/scenes/fourier_transform/FourierTransformFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<glm::vec2> vertices = {
            {0.0, 0.0},
            {0.0, 0.0},
            {1.0, 0.0},
            {1.0, 0.0},
            {1.0, 1.0},
            {1.0, 1.0},
            {0.0, 1.0},
            {0.0, 1.0}
    };
    BufferLayout layout = {
            {ShaderDataType::Float2, "a_Position"},
            {ShaderDataType::Float2, "a_UV"}
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, layout);
    vertexArray->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {
            {0, 1, 2},
            {0, 2, 3}
    };
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);

    texture = std::make_shared<Texture>();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("u_TextureSampler", 0);

    const unsigned int width = 128;
    const unsigned int height = 128;
    const unsigned int channels = 4;
    auto colors = std::vector<unsigned char>(width * height * channels);
    for (unsigned long i = 0; i < colors.size(); i += 4) {
        colors[i] = 255;
        colors[i + 1] = 255;
        colors[i + 2] = 255;
        colors[i + 3] = 255;
    }
    texture->update(colors.data(), width, height);
}

void FourierTransform::destroy() {
    shader.reset();
}

void FourierTransform::tick() {
    shader->bind();
    vertexArray->bind();
    texture->bind();

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
