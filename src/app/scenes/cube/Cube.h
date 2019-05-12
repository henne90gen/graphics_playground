#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <glad/glad.h>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"

class Cube : public Scene {
public:
    Cube(GLFWwindow *window, std::function<void(void)> &backToMainMenu) : Scene(window, backToMainMenu, "Cube") {};

    virtual ~Cube() {};

    virtual void setup() override;

    virtual void tick() override;

    virtual void destroy() override;

private:
    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;
};
