#include "opengl/Texture.h"

#include "OpenGLUtils.h"

Texture::Texture() { GL_Call(glGenTextures(1, &id)); }

Texture::~Texture() {}

void Texture::update(const char* data, unsigned int width, unsigned int height) {
    bind();
    GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
}

void Texture::bind() const { GL_Call(glBindTexture(GL_TEXTURE_2D, id)); }

void Texture::unbind() const { GL_Call(glBindTexture(GL_TEXTURE_2D, 0)); }
