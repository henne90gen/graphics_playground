#include "LegacyTriangle.h"

#include <array>
#include <glad/glad.h>

#include "Main.h"
#include "util/ImGuiUtils.h"
#include "util/OpenGLUtils.h"

DEFINE_SCENE_MAIN(LegacyTriangle)

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
        glVertex2f(vertices[i], vertices[i + 1]);
    }
    glEnd();
    GL_CheckErrors();
}
