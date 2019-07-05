#include "FourierTransform.h"

void FourierTransform::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/fourier_transform/FourierTransformVert.glsl",
                                      "../../../src/app/scenes/fourier_transform/FourierTransformFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<glm::vec2> vertices = {
            {-1.0, -1.0},
            {0.0,  0.0},
            {1.0,  -1.0},
            {1.0,  0.0},
            {1.0,  1.0},
            {1.0,  1.0},
            {-1.0, 1.0},
            {0.0,  1.0}
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

    texture = std::make_shared<Texture>(GL_RGBA);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("u_TextureSampler", 0);
}

void FourierTransform::destroy() {
    shader.reset();
}

void FourierTransform::tick() {
    static std::vector<glm::vec2> mousePositions = {};
    ImGui::Begin("Settings");
    ImGui::Text("Mouse: (%f, %f)", getInput()->mouse.pos.x, getInput()->mouse.pos.y);
    float mouseX = getInput()->mouse.pos.x / getWidth();
    mouseX = mouseX * 2.0F - 1.0F;
    float mouseY = (getHeight() - getInput()->mouse.pos.y) / getHeight();
    mouseY = mouseY * 2.0F - 1.0F;
    ImGui::Text("Relative Mouse: (%f, %f)", mouseX, mouseY);
    ImGui::Text("Num of Positions: %zu", mousePositions.size());
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    texture->bind();

    const unsigned int width = getWidth();
    const unsigned int height = getHeight();
    static auto colors = std::vector<glm::vec4>(width * height);
    unsigned int i = (height - (unsigned int) getInput()->mouse.pos.y) * width + (unsigned int) getInput()->mouse.pos.x;
    colors[i] = {1.0, 1.0, 1.0, 1.0};
    mousePositions.emplace_back(getInput()->mouse.pos.x, getInput()->mouse.pos.y);
    texture->update(colors.data(), width, height);

    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
