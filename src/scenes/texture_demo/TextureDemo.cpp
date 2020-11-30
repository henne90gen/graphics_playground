#include "TextureDemo.h"

#include <array>
#include <vector>

#include <glad/glad.h>

#include "util/ImGuiUtils.h"
#include "util/OpenGLUtils.h"

DEFINE_SHADER(texture_demo_TextureDemo)

void TextureDemo::setup() {
    shader = SHADER(texture_demo_TextureDemo);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<float> vertices = {
            -1.0, 1.0, 0.0, 1.0,  //
            -1.0, -1.0, 0.0, 0.0,  //
            1.0, -1.0, 1.0, 0.0, //
            -1.0, 1.0, 0.0, 1.0,  //
            1.0, -1.0, 1.0, 0.0, //
            1.0, 1.0, 1.0, 1.0  //
    };
    BufferLayout bufferLayout = {
                    {ShaderDataType::Float2, "position"},
                    {ShaderDataType::Float2, "vertexUV"}
            };
    auto buffer = std::make_shared<VertexBuffer>(vertices, bufferLayout);
    vertexArray->addVertexBuffer(buffer);

    texture = std::make_shared<Texture>();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("textureSampler", 0);

    std::array<float, 3> color = {1.0, 1.0, 1.0};
    updateTexture(color, false);
}

void TextureDemo::destroy() {}

void TextureDemo::tick() {
    static std::array<float, 3> color = {1.0, 1.0, 1.0};
    static bool checkerBoard = false;
    std::array<float, 3> previousColor = {color[0], color[1], color[2]};
    bool previousCheckerboard = checkerBoard;
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color", color.data());
    ImGui::Checkbox("Checkerboard", &checkerBoard);
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    texture->bind();

    static bool initialized = false;
    bool checkerBoardChanged = previousCheckerboard != checkerBoard;
    bool colorChanged = previousColor[0] != color[0] || previousColor[1] != color[1] || previousColor[2] != color[2];
    if (!initialized || checkerBoardChanged || colorChanged) {
        updateTexture(color, checkerBoard);
        initialized = true;
    }

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    vertexArray->unbind();
    shader->unbind();
}

void TextureDemo::updateTexture(const std::array<float, 3> color, bool checkerBoard) {
    std::vector<unsigned char> data = std::vector<unsigned char>(getWidth() * getHeight() * 3);
    for (unsigned long i = 0; i < data.size() / 3; i++) {
        const float colorScaleFactor = 255.0F;
        float r = color[0] * colorScaleFactor;
        float g = color[1] * colorScaleFactor;
        float b = color[2] * colorScaleFactor;
        unsigned int row = i / getWidth();
        if (checkerBoard && ((i % 2 == 0 && row % 2 == 0) || (i % 2 == 1 && row % 2 == 1))) {
            r = colorScaleFactor - r;
            g = colorScaleFactor - g;
            b = colorScaleFactor - b;
        }
        unsigned int idx = i * 3;
        data[idx] = static_cast<char>(r);
        data[idx + 1] = static_cast<char>(g);
        data[idx + 2] = static_cast<char>(b);
    }
    texture->update(data.data(), getWidth(), getHeight());
}
