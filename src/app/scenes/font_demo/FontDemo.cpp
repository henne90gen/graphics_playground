#include "FontDemo.h"

void FontDemo::setup() {
    shader = new Shader("../../../src/app/scenes/font_demo/FontDemo.vertex",
                        "../../../src/app/scenes/font_demo/FontDemo.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    std::vector<float> vertices = {
            -1.0, 1.0, 0.0, 1.0,  //
            -1.0, -1.0, 0.0, 0.0,  //
            1.0, -1.0, 1.0, 0.0, //
            -1.0, 1.0, 0.0, 1.0,  //
            1.0, -1.0, 1.0, 0.0, //
            1.0, 1.0, 1.0, 1.0  //
    };
    auto *buffer = new VertexBuffer(vertices);
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "position", 2);
    bufferLayout.add<float>(shader, "vertexUV", 2);
    vertexArray->addBuffer(*buffer, bufferLayout);


    texture = new Texture();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("textureSampler", 0);

    std::array<float, 3> color = {1.0, 1.0, 1.0};
    updateTexture(color);
}

void FontDemo::destroy() {}

void FontDemo::tick() {
    static std::array<float, 3> color = {1.0, 1.0, 1.0};
    std::array<float, 3> previousColor = {color[0], color[1], color[2]};
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color", color.data());
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    texture->bind();

    static bool initialized = false;
    bool colorChanged = previousColor[0] != color[0] || previousColor[1] != color[1] || previousColor[2] != color[2];
    if (!initialized || colorChanged) {
        updateTexture(color);
        initialized = true;
    }

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    vertexArray->unbind();
    shader->unbind();
}

void FontDemo::updateTexture(const std::array<float, 3> color) {
    std::vector<char> data = std::vector<char>(getWidth() * getHeight() * 3);
    for (unsigned long i = 0; i < data.size() / 3; i++) {
        const float colorScaleFactor = 255.0F;
        float r = color[0] * colorScaleFactor;
        float g = color[1] * colorScaleFactor;
        float b = color[2] * colorScaleFactor;
        unsigned int idx = i * 3;
        data[idx] = static_cast<char>(r);
        data[idx + 1] = static_cast<char>(g);
        data[idx + 2] = static_cast<char>(b);
    }
    texture->update(data.data(), getWidth(), getHeight());
}
