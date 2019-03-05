#include "scenes/texture/Texture.h"

#include <glad/glad.h>

#include "ImGuiUtils.h"
#include "OpenGLUtils.h"

void Texture::setup() {

}

void Texture::destroy() {

}

void Texture::tick() {
    static float color[3] = {1.0, 1.0, 1.0};
    pickColor(color);
    static float vertices[12] = {
        -1, 1,  //
        -1, -1, //
        1,  -1, //
        -1, 1,  //
        1,  -1, //
        1,  1   //
    };
    static float uvCoords[12] = {
        0, 1, //
        0, 0, //
        1, 0, //
        0, 1, //
        1, 0, //
        1, 1  //
    };

    GLuint programId = loadShaders("../src/scenes/texture/Texture.vertex", "../src/scenes/texture/Texture.fragment");

    GL_Call(glUseProgram(programId));

    GLuint vertexbuffer;
    GL_Call(glGenBuffers(1, &vertexbuffer));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    GL_Call(GLuint positionLocation = glGetAttribLocation(programId, "position"));
    GL_Call(glEnableVertexAttribArray(positionLocation));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0));

    unsigned int width = 128;
    unsigned int height = 128;
    void *data[128 * 128 * 3] = {};
    for (unsigned int row = 0; row < height; row++) {
        for (unsigned int col = 0; col < width; col++) {
            unsigned int index = row * width + col;
            char *ptr = (char *)&data[index];
            *ptr = color[2] * 255;
            *(ptr + 1) = color[1] * 255;
            *(ptr + 2) = color[0] * 255;
        }
    }
    GLuint textureId;
    GL_Call(glGenTextures(1, &textureId));
    GL_Call(glBindTexture(GL_TEXTURE_2D, textureId));
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    GLuint textureSampler = glGetUniformLocation(programId, "textureSampler");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(textureSampler, 0);

    GLuint uvBuffer;
    GL_Call(glGenBuffers(1, &uvBuffer));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, uvBuffer));
    GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(uvCoords), uvCoords, GL_STATIC_DRAW));

    GL_Call(GLuint uvLocation = glGetAttribLocation(programId, "vertexUV"));
    GL_Call(glEnableVertexAttribArray(uvLocation));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, uvBuffer));
    GL_Call(glVertexAttribPointer(uvLocation, 3, GL_FLOAT, GL_TRUE, 0, (void *)0));

    GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

    GL_Call(glDisableVertexAttribArray(uvLocation));
    GL_Call(glDisableVertexAttribArray(positionLocation));

    GL_Call(glUseProgram(0));
}
