#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <glad/glad.h>

#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/IndexBuffer.h"
#include "marching_cubes/MarchingCubes.h"

class MarchingCubesScene : public Scene {
public:
    MarchingCubesScene(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "MarchingCubes") {};

    ~MarchingCubesScene() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

public:
    VertexArray *vertexArray;
    Shader *shader;
    IndexBuffer *indexBuffer;

    MarchingCubes *marchingCubes;

    void drawCube(const glm::vec3 &translation, const glm::vec3 &cameraRotation, float scale);
};
