#include "GammaCalculation.h"

#include <cmath>
#include <vector>

#include <glad/glad.h>
#include <glm/ext.hpp>

#include "Main.h"
#include "util/ImGuiUtils.h"
#include "util/OpenGLUtils.h"

DEFINE_SCENE_MAIN(GammaCalculation)
DEFINE_DEFAULT_SHADERS(gamma_calculation_GammaCalculation)

void GammaCalculation::setup() {
    shader = CREATE_DEFAULT_SHADER(gamma_calculation_GammaCalculation);
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<float> vertices = {
          -1.0, 1.0,  //
          -1.0, -1.0, //
          0.0,  -1.0, //
          -1.0, 1.0,  //
          0.0,  -1.0, //
          0.0,  1.0   //
    };
    BufferLayout positionLayout = {{ShaderDataType::Vec2, "position"}};
    auto positionBuffer = std::make_shared<VertexBuffer>(vertices, positionLayout);
    vertexArray->addVertexBuffer(positionBuffer);

    std::vector<float> uvCoords = {
          0.0, 1.0, //
          0.0, 0.0, //
          1.0, 0.0, //
          0.0, 1.0, //
          1.0, 0.0, //
          1.0, 1.0  //
    };
    BufferLayout uvLayout = {{ShaderDataType::Vec2, "vertexUV"}};
    auto uvBuffer = std::make_shared<VertexBuffer>(uvCoords, uvLayout);
    vertexArray->addVertexBuffer(uvBuffer);

    checkerBoardTexture = std::make_shared<Texture>();
    glActiveTexture(GL_TEXTURE0);
    checkerBoardTexture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("textureSampler", 0);
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
