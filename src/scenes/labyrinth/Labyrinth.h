#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/VertexArray.h"
#include "opengl/IndexBuffer.h"

class Labyrinth : public Scene {
public:
    Labyrinth(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "Labyrinth") {};

    virtual ~Labyrinth() {};

    virtual void setup() override;

    virtual void tick() override;

    virtual void destroy() override;

private:
    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;
};
