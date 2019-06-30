#pragma once

#include "scenes/Scene.h"
#include "rubiks_cube/RubiksCube.h"
#include "rubiks_cube/RubiksCubeLogic.h"

#include <functional>
#include <memory>

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

protected:
    void onAspectRatioChange() override;

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<IndexBuffer> indexBuffer;

    glm::mat4 projectionMatrix;

    float *vertices;
    unsigned int *indices;

    std::shared_ptr<RubiksCube> rubiksCube;
};
