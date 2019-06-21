#include "GammaCalculation.h"

#include <cmath>
#include <vector>

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "util/OpenGLUtils.h"
#include "util/ImGuiUtils.h"

void GammaCalculation::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/gamma_calculation/GammaCalculationVert.glsl",
                                      "../../../src/app/scenes/gamma_calculation/GammaCalculationFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>();
    vertexArray->bind();

    std::vector<float> vertices = {
            -1.0, 1.0,  //
            -1.0, -1.0, //
            0.0, -1.0, //
            -1.0, 1.0,  //
            0.0, -1.0, //
            0.0, 1.0   //
    };
    auto *positionBuffer = new VertexBuffer(vertices);
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(*positionBuffer, positionLayout);

    std::vector<float> uvCoords = {
            0.0, 1.0, //
            0.0, 0.0, //
            1.0, 0.0, //
            0.0, 1.0, //
            1.0, 0.0, //
            1.0, 1.0  //
    };
    auto *uvBuffer = new VertexBuffer(uvCoords);
    VertexBufferLayout uvLayout;
    uvLayout.add<float>(shader, "vertexUV", 2);
    vertexArray->addBuffer(*uvBuffer, uvLayout);

    checkerBoardTexture = std::make_shared<Texture>();
    glActiveTexture(GL_TEXTURE0);
    checkerBoardTexture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("textureSampler", 0);
}

void GammaCalculation::destroy() {}

void GammaCalculation::tick() {
    static float color = 0.5F; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    ImGui::Begin("Settings");
    const float dragSpeed = 0.001F;
    const float minColor = 0.0F;
    const float maxColor = 1.0F;
    ImGui::DragFloat("GrayValue", &color, dragSpeed, minColor, maxColor);
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
    std::vector<unsigned char> data = std::vector<unsigned char>(width * getHeight() * 3);
    for (unsigned long i = 0; i < data.size() / 3; i++) {
        const float fullBrightness = 255.0F;
        float r = fullBrightness;
        float g = fullBrightness;
        float b = fullBrightness;
        unsigned int row = i / width;
        if ((i % 2 == 0 && row % 2 == 0) || (i % 2 == 1 && row % 2 == 1)) {
            r = 0.0F;
            g = 0.0F;
            b = 0.0F;
        }
        unsigned int idx = i * 3;
        data[idx] = static_cast<char>(r);
        data[idx + 1] = static_cast<char>(g);
        data[idx + 2] = static_cast<char>(b);
    }
    checkerBoardTexture->update(data.data(), width, getHeight());
}

float GammaCalculation::calculateGammaValue(float color) {
    const float middleGrayValue = 0.5F;
    return std::log(middleGrayValue) / std::log(color);
}
