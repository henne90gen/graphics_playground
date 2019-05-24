#include "TextureDemo.h"

#include <vector>

#include <glad/glad.h>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void TextureDemo::setup() {
    shader = new Shader("../../../src/app/scenes/texture_demo/GammaCalculation.vertex",
                        "../../../src/app/scenes/texture_demo/GammaCalculation.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    static float vertices[12] = {
            -1.0, 1.0,  //
            -1.0, -1.0, //
            1.0, -1.0, //
            -1.0, 1.0,  //
            1.0, -1.0, //
            1.0, 1.0   //
    };
    auto *positionBuffer = new VertexBuffer(vertices, sizeof(vertices));
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(*positionBuffer, positionLayout);

    static float uvCoords[12] = {
            0.0, 1.0, //
            0.0, 0.0, //
            1.0, 0.0, //
            0.0, 1.0, //
            1.0, 0.0, //
            1.0, 1.0  //
    };
    auto *uvBuffer = new VertexBuffer(uvCoords, sizeof(uvCoords));
    VertexBufferLayout uvLayout;
    uvLayout.add<float>(shader, "vertexUV", 2);
    vertexArray->addBuffer(*uvBuffer, uvLayout);

    texture = new Texture();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("textureSampler", 0);

    float color[3] = {1.0, 1.0, 1.0};
    updateTexture(color, false);
}

void TextureDemo::destroy() {}

void TextureDemo::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    static bool checkerBoard = false;
    float previousColor[3] = {color[0], color[1], color[2]};
    bool previousCheckerboard = checkerBoard;
    ImGui::Begin("Settings");
    ImGui::ColorEdit3("Color", color);
    ImGui::Checkbox("Checkerboard", &checkerBoard);
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    texture->bind();

    if (previousCheckerboard != checkerBoard || previousColor[0] != color[0] || previousColor[1] != color[1] ||
        previousColor[2] != color[2]) {
        updateTexture(color, checkerBoard);
    }

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    vertexArray->unbind();
    shader->unbind();
}

void TextureDemo::updateTexture(const float *color, bool checkerBoard) {
    std::cout << "Updating texture..." << std::endl;
    std::vector<char> data = std::vector<char>(getWidth() * getHeight() * 3);
    for (unsigned int i = 0; i < data.size() / 3; i++) {
        float r = color[0] * 255.0;
        float g = color[1] * 255.0;
        float b = color[2] * 255.0;
        unsigned int row = i / getWidth();
        if (checkerBoard && ((i % 2 == 0 && row % 2 == 0) || (i % 2 == 1 && row % 2 == 1))) {
            r = 255.0 - r;
            g = 255.0 - g;
            b = 255.0 - b;
        }
        unsigned int idx = i * 3;
        data[idx] = static_cast<char>(r);
        data[idx + 1] = static_cast<char>(g);
        data[idx + 2] = static_cast<char>(b);
    }
    texture->update(data.data(), getWidth(), getHeight());
}
