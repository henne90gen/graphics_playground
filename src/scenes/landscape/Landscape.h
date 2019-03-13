#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/VertexArray.h"
#include "opengl/IndexBuffer.h"

class Landscape : public Scene {
public:
    Landscape(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "Landscape") {};

    virtual ~Landscape() {};

    virtual void setup() override;

    virtual void tick() override;

    virtual void destroy() override;

private:
    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;
};
