#pragma once

#include "scenes/Scene.h"

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Texture.h"
#include "model_loading/ModelLoader.h"

#include <functional>

struct RenderMesh {
    VertexArray *vertexArray;
    VertexBuffer *vertexBuffer;
    VertexBuffer *normalBuffer;
    VertexBuffer *textureCoordinatesBuffer;
    IndexBuffer *indexBuffer;
    Texture *texture;
    bool shouldRender = true;
};

struct RenderModel {
    std::vector<RenderMesh> meshes;
};

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

    RenderModel renderModel;
    ModelLoader::Model model = {};

    glm::mat4 projectionMatrix;

    void createCheckerBoard(RenderMesh &mesh);

    void updateModel(const std::string &modelFileName);

    void updateTexture(ModelLoader::Mesh &mesh, RenderMesh &renderMesh);
};

void
showSettings(bool &rotate, glm::vec3 &translation, glm::vec3 &modelRotation, glm::vec3 &cameraRotation, float &scale,
             bool &drawWireframe, unsigned int &currentModel, std::vector<std::string> &paths,
             ModelLoader::Model &model, RenderModel &renderModel);
