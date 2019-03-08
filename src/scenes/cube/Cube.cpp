#include "scenes/cube/Cube.h"

#include <glad/glad.h>

#include "ImGuiUtils.h"
#include "OpenGLUtils.h"

void Cube::setup() {
    GL_Call(glGenBuffers(1, &vertexbuffer));
    GL_Call(glGenBuffers(1, &uvBuffer));
    GL_Call(glGenTextures(1, &textureId));

    programId = loadShaders("../src/scenes/texture/Texture.vertex", "../src/scenes/texture/Texture.fragment");
    GL_Call(glUseProgram(programId));

    GL_Call(glEnableVertexAttribArray(0));

    GL_Call(positionLocation = glGetAttribLocation(programId, "position"));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer));
    GL_Call(glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0));

    GL_Call(uvLocation = glGetAttribLocation(programId, "vertexUV"));
    GL_Call(glBindBuffer(GL_ARRAY_BUFFER, uvBuffer));
    GL_Call(glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, (void *)0));

    GL_Call(glBindTexture(GL_TEXTURE_2D, textureId));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    GLuint textureSampler = glGetUniformLocation(programId, "textureSampler");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(textureSampler, 0);
}

void Cube::destroy() {}

void Cube::tick() {}
