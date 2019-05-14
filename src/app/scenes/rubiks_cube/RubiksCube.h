#pragma once

#include "scenes/Scene.h"
#include "RubiksCubeLogic.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"

class RubiksCube : public Scene {
public:
    RubiksCube(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "RubiksCube") {};

    ~RubiksCube() override = default;;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;

    float *vertices;
    unsigned int *indices;
    std::vector<CubeRotation> cubeRotations;
    std::vector<unsigned int> cubePositions;

    std::vector<Rotation> rotations;
    unsigned int currentRotationIndex = 0;
};
