#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/Texture.h"
#include "opengl/Shader.h"

class FontDemo : public Scene {
public:
    FontDemo(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "FontDemo") {};

    ~FontDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    VertexArray *vertexArray;
    Texture *texture;
    Shader *shader;

    void updateTexture(std::array<float, 3> color);
};
