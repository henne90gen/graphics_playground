#pragma once

#include <vector>

#include "util/OpenGLUtils.h"

struct TextureSettings {
    unsigned int activeTexture = GL_TEXTURE0;
    unsigned int dataType = GL_RGB;
    unsigned int openGlDataType = GL_RGB;
    unsigned int magnificationFilter = GL_NEAREST;
    unsigned int minificationFilter = GL_NEAREST;
};

class Texture {
  public:
    explicit Texture();
    explicit Texture(TextureSettings settings);
    ~Texture() = default;

    void update(const unsigned char *data, unsigned int width, unsigned int height,
                unsigned int unpackAlignment = 4) const;
    void update(const std::vector<unsigned char> &data, unsigned int width, unsigned int height,
                unsigned int unpackAlignment = 4) const;
    void update(const std::vector<glm::vec3> &data, unsigned int width, unsigned int height,
                unsigned int unpackAlignment = 4) const;
    void update(const std::vector<glm::vec4> &data, unsigned int width, unsigned int height,
                unsigned int unpackAlignment = 4) const;

    void bind() const;
    static void unbind();

    void setDataType(unsigned int dataType) { this->settings.dataType = dataType; }
    void setOpenGlDataType(unsigned int openGlDataType) { this->settings.openGlDataType = openGlDataType; }

    unsigned int getInternalId() { return id; }

  private:
    unsigned int id = 0;
    TextureSettings settings = {};

    void init();
};
