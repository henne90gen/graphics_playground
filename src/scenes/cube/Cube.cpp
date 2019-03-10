#include "scenes/cube/Cube.h"

#include <glad/glad.h>

#include "ImGuiUtils.h"
#include "OpenGLUtils.h"

void Cube::setup() {
    vertexArray = new VertexArray();

    static float vertices[12] = {
        -1.0, 1.0,  //
        -1.0, -1.0, //
        1.0,  -1.0, //
        -1.0, 1.0,  //
        1.0,  -1.0, //
        1.0,  1.0   //
    };
    static float uvCoords[12] = {
        0.0, 1.0, //
        0.0, 0.0, //
        1.0, 0.0, //
        0.0, 1.0, //
        1.0, 0.0, //
        1.0, 1.0  //
    };
    VertexBuffer positionBuffer = VertexBuffer(vertices, sizeof(vertices));
    VertexBufferLayout positionLayout = VertexBufferLayout();
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(positionBuffer, positionLayout);

    VertexBuffer uvBuffer = VertexBuffer(uvCoords, sizeof(uvCoords));
    VertexBufferLayout uvLayout = VertexBufferLayout();
    uvLayout.add<float>(shader, "vertexUV", 2);
    vertexArray->addBuffer(uvBuffer, uvLayout);

    texture = new Texture();

    shader =
        new Shader("../src/scenes/texture_demo/TextureDemo.vertex", "../src/scenes/texture_demo/TextureDemo.fragment");
    shader->bind();

    // positionBuffer->bind();
    // GL_Call(positionLocation = glGetAttribLocation(shader->getId(), "position"));
    // GL_Call(glEnableVertexAttribArray(positionLocation));
    // GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0));

    // uvBuffer->bind();
    // GL_Call(uvLocation = glGetAttribLocation(shader->getId(), "vertexUV"));
    // GL_Call(glEnableVertexAttribArray(uvLocation));
    // GL_Call(glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0));

    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    shader->setUniform<int>("textureSampler", 0);
}

void Cube::destroy() {}

void Cube::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    pickColor(color);

    shader->bind();

    vertexArray->bind();

    unsigned int width = 128;
    unsigned int height = 128;
    char data[128 * 128 * 3] = {};
    for (unsigned int i = 0; i < sizeof(data) / sizeof(char); i += 3) {
        data[i] = color[0] * 255;
        data[i + 1] = color[1] * 255;
        data[i + 2] = color[2] * 255;
    }
    texture->update(data, width, height);

    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glEnableVertexAttribArray(uvLocation));

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    GL_Call(glDisableVertexAttribArray(uvLocation));
    GL_Call(glDisableVertexAttribArray(positionLocation));

    vertexArray->unbind();

    GL_Call(glUseProgram(0));
}
