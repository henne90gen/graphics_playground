#include "LegacyTriangle.h"

#include <array>
#include <glad/glad.h>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void LegacyTriangle::setup() {}

void LegacyTriangle::destroy() {}

void LegacyTriangle::tick() {
    GL_Call(glMatrixMode(GL_PROJECTION));
    GL_Call(glLoadIdentity());

    const unsigned int COLOR_COUNT = 3;
    const unsigned int VERTEX_COUNT = 6;
    static std::array<float, COLOR_COUNT> color = {1.0, 1.0, 1.0};
    static std::array<float, VERTEX_COUNT> vertices = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color.data(), vertices.data());

    // DO NOT USE glGetError inside a glBegin/glEnd block
    // https://stackoverflow.com/a/13442010/2840827
    glBegin(GL_TRIANGLES);
    glColor3f(color[0], color[1], color[2]);
    for (unsigned long i = 0; i < vertices.size(); i += 2) {
        glVertex2f(vertices[i], vertices[i + 1]); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }
    glEnd();
    GL_CheckErrors();
}
