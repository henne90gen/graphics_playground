#pragma once

#include "scenes/Scene.h"
#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Texture.h"

#include <functional>
#include <memory>

class FourierTransform : public Scene {
public:
    FourierTransform(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "FourierTransform") {};

    ~FourierTransform() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Texture> texture;
};
