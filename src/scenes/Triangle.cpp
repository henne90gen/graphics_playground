#include "scenes/Triangle.h"

#include <glad/glad.h>

#include "ImGuiUtils.h"

void Triangle::tick() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    static float color[3] = {1.0, 1.0, 1.0};
    static float vertices[6] = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color, vertices);

    glBegin(GL_TRIANGLES);
    glColor3f(color[0], color[1], color[2]);
    for (unsigned int i = 0; i < sizeof(vertices) / sizeof(float); i += 2) {
        glVertex2f(vertices[i], vertices[i + 1]);
    }
    glEnd();
}
