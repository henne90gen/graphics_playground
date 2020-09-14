#pragma once

#include "util/OpenGLUtils.h"
#include <util/Image.h>

class Texture {
  public:
    explicit Texture(unsigned int dataType = GL_RGB, unsigned int openGlDataType = GL_RGB);
    ~Texture() = default;

    void update(const unsigned char *data, unsigned int width, unsigned int height,
                unsigned int unpackAlignment = 4) const;
    void update(const std::vector<glm::vec3> &data, unsigned int width, unsigned int height,
                unsigned int unpackAlignment = 4) const;
    void update(const std::vector<glm::vec4> &data, unsigned int width, unsigned int height,
                unsigned int unpackAlignment = 4) const;
    void update(Image &image) const;

    void bind() const;
    static void unbind();

    void setDataType(unsigned int _dataType) { this->dataType = _dataType; }
    void setOpenGlDataType(unsigned int _openGlDataType) { this->openGlDataType = _openGlDataType; }

  private:
    unsigned int id = 0;
    unsigned int dataType;
    unsigned int openGlDataType;
};
