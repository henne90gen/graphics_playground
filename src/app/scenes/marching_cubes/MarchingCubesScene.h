#pragma once

#include "scenes/Scene.h"

#include <functional>
#include <memory>
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

protected:
    void onAspectRatioChange() override;

public:
    std::shared_ptr<Shader> shader;

    std::shared_ptr<VertexArray> cubeVertexArray;
    std::shared_ptr<IndexBuffer> cubeIndexBuffer;

    std::shared_ptr<VertexArray> surfaceVertexArray;
    std::shared_ptr<VertexBuffer> surfaceVertexBuffer;
    std::shared_ptr<IndexBuffer> surfaceIndexBuffer;

    std::shared_ptr<MarchingCubes> marchingCubes;

    glm::mat4 projectionMatrix;

    void drawCube();

    void drawSurface(bool drawWireframe);

    void showSettings(glm::vec3 &translation, glm::vec3 &cameraRotation, glm::vec3 &modelRotation,
                      float &scale, bool &rotate, bool &drawWireframe) const;
};
