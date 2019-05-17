#pragma once

#include "scenes/Scene.h"
#include "rubiks_cube/RubiksCube.h"
#include "rubiks_cube/RubiksCubeLogic.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"

class RubiksCubeScene : public Scene {
public:
    RubiksCubeScene(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "RubiksCube") {};

    ~RubiksCubeScene() override = default;;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;

    float *vertices;
    unsigned int *indices;

    RubiksCube rubiksCube;
};
