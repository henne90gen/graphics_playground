#include "LegacyTriangle.h"

#include <glad/glad.h>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void LegacyTriangle::setup() {}

void LegacyTriangle::destroy() {}

void LegacyTriangle::tick() {
    GL_Call(glMatrixMode(GL_PROJECTION));
    GL_Call(glLoadIdentity());

    static std::array<float, 3> color = {1.0, 1.0, 1.0};
    static std::array<float, 6> vertices = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color.data(), vertices.data());

    // DO NOT USE glGetError inside a glBegin/glEnd block
    // https://stackoverflow.com/a/13442010/2840827
    glBegin(GL_TRIANGLES);
    glColor3f(color[0], color[1], color[2]);
    for (unsigned int i = 0; i < sizeof(vertices) / sizeof(float); i += 2) {
        glVertex2f(vertices[i], vertices[i + 1]);
    }
    glEnd();
    GL_CheckErrors();
}
