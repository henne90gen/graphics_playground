#include "Triangle.h"

#include <glad/glad.h>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void Triangle::setup() {
    shader = new Shader("../../../src/app/scenes/triangle/Triangle.vertex",
                        "../../../src/app/scenes/triangle/Triangle.fragment");
    shader->bind();

    positionBuffer = new VertexBuffer();
    colorBuffer = new VertexBuffer();

    positionBuffer->bind();
    GL_Call(positionLocation = glGetAttribLocation(shader->getId(), "position"));
    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *) nullptr));

    colorBuffer->bind();
    GL_Call(colorLocation = glGetAttribLocation(shader->getId(), "color"));
    GL_Call(glEnableVertexAttribArray(colorLocation));
    GL_Call(glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, (void *) nullptr));

    shader->unbind();
}

void Triangle::destroy() {}

void Triangle::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    static float vertices[6] = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color, vertices);

    shader->bind();

    positionBuffer->update(vertices, sizeof(vertices));

    float colors[9] = {};
    for (int i = 0; i < 9; i++) {
        colors[i] = color[i % 3];
    }
    colorBuffer->update(colors, sizeof(colors));

    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glEnableVertexAttribArray(colorLocation));

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 3));

    GL_Call(glDisableVertexAttribArray(colorLocation));
    GL_Call(glDisableVertexAttribArray(positionLocation));

    shader->unbind();
}
