#include "AtmosphericScattering.h"

#include <Main.h>
#include <util/RenderUtils.h>

DEFINE_SCENE_MAIN(AtmosphericScattering)
DEFINE_DEFAULT_SHADERS(atmospheric_scattering_AtmosphericScattering)

void AtmosphericScattering::setup() {
    shader = CREATE_DEFAULT_SHADER(atmospheric_scattering_AtmosphericScattering);
    quadVA = createQuadVA(shader);
}

void AtmosphericScattering::destroy() {}

void AtmosphericScattering::tick() {
    shader->bind();
    quadVA->bind();

//    auto viewMatrix = createViewMatrix(cameraPosition, cameraRotation);

    GL_Call(glDrawElements(GL_TRIANGLES, quadVA->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}
