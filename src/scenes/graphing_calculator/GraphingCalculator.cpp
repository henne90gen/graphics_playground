#include "GraphingCalculator.h"

#include <Main.h>

#include <util/RenderUtils.h>

DEFINE_SCENE_MAIN(GraphingCalculator)
DEFINE_DEFAULT_SHADERS(graphing_calculator_GraphingCalculator)

void GraphingCalculator::setup() {
    shader = CREATE_DEFAULT_SHADER(graphing_calculator_GraphingCalculator);
    shader->bind();

    va = createQuadVA(shader, glm::vec2(2, 2));
}

void GraphingCalculator::destroy() {}

void GraphingCalculator::tick() {
    static glm::vec2 boundsX = glm::vec2(-1.0, 1.0);
    static glm::vec2 boundsY = glm::vec2(-1.0, 1.0);
    static float axisWidth = 0.001F;
    static float lineWidth = 1.0F;

    ImGui::Begin("Settings");
    ImGui::DragFloat2("Bounds X", reinterpret_cast<float *>(&boundsX), 0.1F);
    ImGui::DragFloat2("Bounds Y", reinterpret_cast<float *>(&boundsY), 0.1F);
    ImGui::DragFloat("Axis Width", &axisWidth, 0.001F);
    ImGui::DragFloat("Line Width", &lineWidth, 0.001F);
    ImGui::End();

    shader->bind();

    const auto resolution = glm::vec2(getWidth(), getHeight());
    // std::cout << resolution.x << ", " << resolution.y << std::endl;
    // std::cout << boundsX.x << ", " << boundsX.y << std::endl;
    // std::cout << boundsY.x << ", " << boundsY.y << std::endl;
    shader->setUniform("resolution", resolution);
    shader->setUniform("horizontalCoordinateBounds", boundsX);
    shader->setUniform("verticalCoordinateBounds", boundsY);
    shader->setUniform("axisWidth", axisWidth);
    shader->setUniform("lineWidth", lineWidth);

    va->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    va->unbind();

    shader->unbind();
}
