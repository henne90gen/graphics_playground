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
    shader->bind();

    va->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
    va->unbind();

    shader->unbind();
}
