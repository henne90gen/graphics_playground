#pragma once

#include "scenes/Scene.h"

#include "opengl/Shader.h"
#include "opengl/VertexArray.h"
#include "opengl/IndexBuffer.h"
#include "opengl/Texture.h"
#include "opengl/OpenGLModel.h"
#include "model_loading/ModelLoader.h"

#include <functional>
#include <memory>

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

    std::unique_ptr<OpenGLModel> openGLModel;
    ModelLoader::Model model = {};

    glm::mat4 projectionMatrix;

    void createCheckerBoard(OpenGLMesh &mesh);

    void updateModel(const std::string &modelFileName);

    void updateTexture(ModelLoader::Mesh &mesh, OpenGLMesh &renderMesh);

    void drawModel(const glm::vec3 &translation, const glm::vec3 &modelRotation,
                   const glm::vec3 &cameraRotation, float scale, bool drawWireframe) const;
};

void
showSettings(bool &rotate, glm::vec3 &translation, glm::vec3 &modelRotation, glm::vec3 &cameraRotation, float &scale,
             bool &drawWireframe, unsigned int &currentModel, std::vector<std::string> &paths,
             ModelLoader::Model &model, std::unique_ptr<OpenGLModel> &renderModel);
