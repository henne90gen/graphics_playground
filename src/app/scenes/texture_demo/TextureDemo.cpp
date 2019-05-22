#include "TextureDemo.h"

#include <glad/glad.h>

#include "opengl/Utils.h"
#include "util/ImGuiUtils.h"

void TextureDemo::setup() {
    shader = new Shader("../../../src/app/scenes/texture_demo/TextureDemo.vertex",
                        "../../../src/app/scenes/texture_demo/TextureDemo.fragment");
    shader->bind();

    vertexArray = new VertexArray();
    vertexArray->bind();

    static float vertices[12] = {
            -1.0, 1.0,  //
            -1.0, -1.0, //
            1.0, -1.0, //
            -1.0, 1.0,  //
            1.0, -1.0, //
            1.0, 1.0   //
    };
    auto *positionBuffer = new VertexBuffer(vertices, sizeof(vertices));
    VertexBufferLayout positionLayout;
    positionLayout.add<float>(shader, "position", 2);
    vertexArray->addBuffer(*positionBuffer, positionLayout);

    static float uvCoords[12] = {
            0.0, 1.0, //
            0.0, 0.0, //
            1.0, 0.0, //
            0.0, 1.0, //
            1.0, 0.0, //
            1.0, 1.0  //
    };
    auto *uvBuffer = new VertexBuffer(uvCoords, sizeof(uvCoords));
    VertexBufferLayout uvLayout;
    uvLayout.add<float>(shader, "vertexUV", 2);
    vertexArray->addBuffer(*uvBuffer, uvLayout);

    texture = new Texture();
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    shader->setUniform<int>("textureSampler", 0);
}

void TextureDemo::destroy() {}

void TextureDemo::tick() {
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

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    vertexArray->unbind();
    shader->unbind();
}
