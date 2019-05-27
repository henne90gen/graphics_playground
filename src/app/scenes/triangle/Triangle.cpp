#include "Triangle.h"

#include <glad/glad.h>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void Triangle::setup() {
    shader = new Shader("../../../src/app/scenes/triangle/Triangle.vertex",
                        "../../../src/app/scenes/triangle/Triangle.fragment");
    shader->bind();

    positionBuffer = new VertexBuffer();
    positionBuffer->bind();
    GL_Call(positionLocation = glGetAttribLocation(shader->getId(), "position"));
    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *) nullptr));

    colorBuffer = new VertexBuffer();
    colorBuffer->bind();
    GL_Call(colorLocation = glGetAttribLocation(shader->getId(), "color"));
    GL_Call(glEnableVertexAttribArray(colorLocation));
    GL_Call(glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, (void *) nullptr));

    shader->unbind();
}

void Triangle::destroy() {}

void Triangle::tick() {
    static std::array<float, 3> color = {1.0, 1.0, 1.0};
    static std::array<float, 6> vertices = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color.data(), vertices.data());

    shader->bind();

    positionBuffer->update(vertices.data(), vertices.size() * sizeof(float));

    std::array<float, 9> colors = {};
    for (unsigned int i = 0; i < colors.size(); i++) {
        colors[i] = color[i % 3];
    }
    colorBuffer->update(colors.data(), colors.size() * sizeof(float));

    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glEnableVertexAttribArray(colorLocation));

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 3));

    GL_Call(glDisableVertexAttribArray(colorLocation));
    GL_Call(glDisableVertexAttribArray(positionLocation));

    shader->unbind();
}
