#pragma once

#include "scenes/Scene.h"

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Texture.h"
#include "model_loading/ModelLoader.h"

#include <functional>
#include <memory>

struct RenderMesh {
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    std::shared_ptr<VertexBuffer> normalBuffer;
    std::shared_ptr<VertexBuffer> textureCoordinatesBuffer;
    std::shared_ptr<IndexBuffer> indexBuffer;
    std::shared_ptr<Texture> texture;
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
    std::shared_ptr<Shader> shader;

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
