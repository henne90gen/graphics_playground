#include "scenes/triangle/Triangle.h"

#include <glad/glad.h>

#include "ImGuiUtils.h"
#include "OpenGLUtils.h"

void Triangle::setup() {
    GL_Call(glGenBuffers(1, &vertexbuffer));
    GL_Call(glGenBuffers(1, &colorBuffer));

    programId = loadShaders("../src/scenes/triangle/Triangle.vertex", "../src/scenes/triangle/Triangle.fragment");
    GL_Call(glUseProgram(programId));

    GL_Call(positionLocation = glGetAttribLocation(programId, "position"));
    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0));

    GL_Call(colorLocation = glGetAttribLocation(programId, "color"));
    GL_Call(glEnableVertexAttribArray(colorLocation));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, colorBuffer));
    GL_Call(glVertexAttribPointer(colorLocation, 3, GL_FLOAT, GL_FALSE, 0, (void *)0));

    GL_Call(glUseProgram(0));
}

void Triangle::destroy() {}

void Triangle::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    static float vertices[6] = {-1, -1, 0, 1, 1, -1};
    pickColorAndVertices(color, vertices);

    GL_Call(glUseProgram(programId));

    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    float colors[9] = {};
    for (int i = 0; i < 9; i++) {
        colors[i] = color[i % 3];
    }
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, colorBuffer));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW));

    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glEnableVertexAttribArray(colorLocation));

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 3));

    GL_Call(glDisableVertexAttribArray(colorLocation));
    GL_Call(glDisableVertexAttribArray(positionLocation));

    GL_Call(glUseProgram(0));
}
