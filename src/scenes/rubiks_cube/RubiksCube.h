#pragma once

#include "scenes/Scene.h"

#include <functional>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"

enum Face {
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM
};

enum Direction {
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

struct Rotation {
    Face face;
    Direction direction;
    float currentAngle;
};

struct CubeRotation {
    glm::vec3 finalRotation;
    glm::vec3 currentRotation;
};

class RubiksCube : public Scene {
public:
    RubiksCube(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "RubiksCube") {};

    ~RubiksCube() override = default;;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    VertexArray *vertexArray{};
    Shader *shader{};
    IndexBuffer *indexBuffer{};

    float *vertices{};
    CubeRotation cubeRotations[27]{};
    unsigned int cubePositions[27]{};

    Rotation rotation{};

    bool rotate(Rotation &rot);
};
