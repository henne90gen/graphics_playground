#include "scenes/AdvancedTriangle.h"

#include <glad/glad.h>

#include "ImGuiUtils.h"

void AdvancedTriangle::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    static float vertices[6] = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color, vertices);
}
