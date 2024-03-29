#include "Triangle.h"

#include <array>
#include <glad/glad.h>

#include "Main.h"
#include "util/ImGuiUtils.h"
#include "util/OpenGLUtils.h"

DEFINE_SCENE_MAIN(Triangle)
DEFINE_DEFAULT_SHADERS(triangle_Triangle)

void Triangle::setup() {
    shader = CREATE_DEFAULT_SHADER(triangle_Triangle);
    shader->bind();

    positionBuffer = new VertexBuffer();
    positionBuffer->bind();
    GL_Call(positionLocation = glGetAttribLocation(shader->getId(), "position"));
    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)nullptr));

    colorBuffer = new VertexBuffer();
    colorBuffer->bind();
    GL_Call(colorLocation = glGetAttribLocation(shader->getId(), "color"));
    GL_Call(glEnableVertexAttribArray(colorLocation));
    GL_Call(glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, (void *)nullptr));

    Shader::unbind();
}

void Triangle::destroy() {}

void Triangle::tick() {
    const unsigned int COLOR_COUNT = 3;
    const unsigned int VERTEX_COUNT = 6;
    static std::array<float, COLOR_COUNT> color = {1.0, 1.0, 1.0};
    static std::array<float, VERTEX_COUNT> vertices = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color.data(), vertices.data());

    positionBuffer->update(vertices.data(), vertices.size() * sizeof(float));

    const unsigned int TOTAL_COLOR_COUNT = 9;
    std::array<float, TOTAL_COLOR_COUNT> colors = {};
    for (unsigned long i = 0; i < colors.size(); i++) {
        colors[i] = color[i % 3];
    }
    colorBuffer->update(colors.data(), colors.size() * sizeof(float));

    shader->bind();

    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glEnableVertexAttribArray(colorLocation));

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 3));

    GL_Call(glDisableVertexAttribArray(colorLocation));
    GL_Call(glDisableVertexAttribArray(positionLocation));

    Shader::unbind();
}
