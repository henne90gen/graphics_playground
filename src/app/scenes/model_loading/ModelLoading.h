#pragma once

#include "scenes/Scene.h"

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Texture.h"

#include <functional>
#include <model_loading/ModelLoader.h>

class ModelLoading : public Scene {
public:
    ModelLoading(GLFWwindow *window, std::function<void(void)> &backToMainMenu)
            : Scene(window, backToMainMenu, "ModelLoading") {};

    ~ModelLoading() override = default;

    void setup() override;

    void tick() override;

    void destroy() override;

private:
    Shader *shader;
    VertexArray *vertexArray;
    VertexBuffer *vertexBuffer;
    VertexBuffer *normalBuffer;
    VertexBuffer *textureCoordinatesBuffer;
    IndexBuffer *indexBuffer;
    Texture *texture;

    glm::mat4 projectionMatrix;

    void createCheckerBoard();

    void updateModel(const std::string &modelFileName);
};
