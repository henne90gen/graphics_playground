#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"

class LightDemo : public Scene {
public:
    LightDemo(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "LightDemo") {};

    ~LightDemo() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    std::shared_ptr<IndexBuffer> indexBuffer;
};
