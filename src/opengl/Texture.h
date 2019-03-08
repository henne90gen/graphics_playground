#pragma once

class Texture {
  public:
    Texture();
    ~Texture();

    void update(const char* data, unsigned int width, unsigned int height);

    void bind() const;
    void unbind() const;

  private:
    unsigned int id;
};
