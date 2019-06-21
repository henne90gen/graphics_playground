#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>
#include <glad/glad.h>

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/Texture.h"
#include "opengl/Shader.h"

class TextureDemo : public Scene {
public:
    TextureDemo(GLFWwindow *window, std::function<void(void)> &backToMainMenu) : Scene(window, backToMainMenu,
                                                                                       "TextureDemo") {};

    ~TextureDemo() override = default;;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;

    void updateTexture(std::array<float, 3> color, bool checkerBoard);
};
