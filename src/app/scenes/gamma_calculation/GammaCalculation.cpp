#include "GammaCalculation.h"

#include <cmath>
#include <vector>

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void GammaCalculation::setup() {
    shader = new Shader("../../../src/app/scenes/gamma_calculation/GammaCalculation.vertex",
                        "../../../src/app/scenes/gamma_calculation/GammaCalculation.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    static float vertices[12] = {
            -1.0, 1.0,  //
            -1.0, -1.0, //
            0.0, -1.0, //
            -1.0, 1.0,  //
            0.0, -1.0, //
            0.0, 1.0   //
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

    checkerBoardTexture = new Texture();
    glActiveTexture(GL_TEXTURE0);
    checkerBoardTexture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("textureSampler", 0);
}

void GammaCalculation::destroy() {}

void GammaCalculation::tick() {
    static float color = 0.5F;
    ImGui::Begin("Settings");
    ImGui::DragFloat("GrayValue", &color, 0.001F, 0.0F, 1.0F);
    float gammaValue = calculateGammaValue(color);
    ImGui::Text("GammaValue: %f", gammaValue);
    ImGui::End();

    shader->bind();
    vertexArray->bind();
    checkerBoardTexture->bind();

    createCheckerBoard();
    glm::mat4 modelMatrix = glm::mat4(1.0);
    shader->setUniform("modelMatrix", modelMatrix);
    shader->setUniform("shouldRenderTexture", true);
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    modelMatrix = glm::translate(modelMatrix, glm::vec3(1.0, 0.0, 0.0));
    shader->setUniform("shouldRenderTexture", false);
    shader->setUniform("grayValue", color);
    shader->setUniform("modelMatrix", modelMatrix);
    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    vertexArray->unbind();
    shader->unbind();
}

void GammaCalculation::createCheckerBoard() {
    unsigned int width = getWidth() / 2;
    std::vector<char> data = std::vector<char>(width * getHeight() * 3);
    for (unsigned int i = 0; i < data.size() / 3; i++) {
        float r = 255.0;
        float g = 255.0;
        float b = 255.0;
        unsigned int row = i / width;
        if ((i % 2 == 0 && row % 2 == 0) || (i % 2 == 1 && row % 2 == 1)) {
            r = 0.0;
            g = 0.0;
            b = 0.0;
        }
        unsigned int idx = i * 3;
        data[idx] = static_cast<char>(r);
        data[idx + 1] = static_cast<char>(g);
        data[idx + 2] = static_cast<char>(b);
    }
    checkerBoardTexture->update(data.data(), width, getHeight());
}

float GammaCalculation::calculateGammaValue(float color) {
    // 0.5 is the middle gray value
    return std::log(0.5) / std::log(color);
}
