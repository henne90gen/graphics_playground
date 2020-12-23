#include "Tesselation.h"

#include "Main.h"
#include "util/RenderUtils.h"

DEFINE_SCENE_MAIN(Tesselation)
DEFINE_SHADER(tesselation_Tesselation)

void Tesselation::setup() {
    shader = SHADER(tesselation_Tesselation);
    va = createQuadVA(shader);
}

void Tesselation::destroy() {}

void Tesselation::tick() {
    shader->bind();
    va->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, va->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
