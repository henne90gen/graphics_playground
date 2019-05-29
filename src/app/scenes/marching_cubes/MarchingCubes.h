#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <glad/glad.h>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"

class MarchingCubes : public Scene {
public:
    MarchingCubes(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "MarchingCubes") {};

    ~MarchingCubes() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

public:
    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;
};
