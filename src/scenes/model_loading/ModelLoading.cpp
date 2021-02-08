#include "ModelLoading.h"

#include "Image.h"
#include "Main.h"
#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

DEFINE_SCENE_MAIN(ModelLoading)
DEFINE_DEFAULT_SHADERS(model_loading_ModelLoading)

void ModelLoading::setup() {
    shader = CREATE_DEFAULT_SHADER(model_loading_ModelLoading);
    shader->bind();
    onAspectRatioChange();
}

void ModelLoading::destroy() {}

void ModelLoading::tick() {
    static glm::vec3 translation = {1.7F, -3.5F, -12.0F}; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static glm::vec3 modelRotation = {0.0F, 1.0F, 0.0F};
    static float scale = 0.5F; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static bool rotate = false;
    static bool rotateWithMouse = false;
    static float mouseRotationSpeed = 5.0F;
    static bool drawWireframe = false;
    static unsigned int currentModel = 3;
    static unsigned int prevModel = currentModel + 1;

    static glm::vec2 lastMousePos = {};

    const float rotationSpeed = 0.03F;
    if (rotate) {
        modelRotation.y += rotationSpeed;
    }

    std::vector<std::string> paths = {};
    showSettings(rotate, rotateWithMouse, mouseRotationSpeed, translation, modelRotation, scale, drawWireframe,
                 currentModel, paths);

    shader->bind();

    if (prevModel != currentModel) {
        RECORD_SCOPE_NAME("LoadingModel");
        std::string modelFileName = paths[currentModel];
        Model::loadFromFile(modelFileName, shader, model);
        prevModel = currentModel;
    }

    drawModel(translation, modelRotation, scale, drawWireframe);
}

void ModelLoading::drawModel(const glm::vec3 &translation, const glm::vec3 &modelRotation, float scale,
                             bool drawWireframe) {
    if (!model.isLoaded()) {
        return;
    }

    RECORD_SCOPE_NAME("RenderModel");

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    shader->setUniform("u_Model", modelMatrix);
    shader->setUniform("u_View", getCamera().getViewMatrix());
    shader->setUniform("u_Projection", getCamera().getProjectionMatrix());

    shader->setUniform("u_TextureSampler", 0);

    for (const auto &mesh : model.getMeshes()) {
        if (!mesh.visible) {
            continue;
        }

        mesh.vertexArray->bind();

        mesh.texture->bind();

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(glDrawElements(GL_TRIANGLES, mesh.indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh.vertexArray->unbind();
    }
}

void ModelLoading::showSettings(bool &rotate, bool &rotateWithMouse, float &mouseRotationSpeed, glm::vec3 &translation,
                                glm::vec3 &modelRotation, float &scale, bool &drawWireframe, unsigned int &currentModel,
                                std::vector<std::string> &paths) {
    ImGui::Begin("Settings");
    ImGui::FileSelector("Models", "model_loading_resources/models/", currentModel, paths);
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    ImGui::Checkbox("Wireframe", &drawWireframe);
    ImGui::Checkbox("Rotate", &rotate);
    ImGui::Checkbox("Rotate With Mouse", &rotateWithMouse);
    ImGui::DragFloat("Mouse Rotation Speed", &mouseRotationSpeed, 0.01F);
    ImGui::DragFloat("Scale", &scale, 0.001F);

    if (model.isLoaded()) {
        const auto &rawModel = model.getRawModel();
        ImGui::Text("Number of meshes: %ld", rawModel.meshes.size());
        for (unsigned long i = 0; i < rawModel.meshes.size(); i++) {
            const auto &mesh = rawModel.meshes[i];
            auto renderMesh = model.getMeshes()[i];
            ImGui::Checkbox(("\tName: " + mesh.name).c_str(), &renderMesh.visible);
            ImGui::Text("\t\tNumber of vertices: %ld", mesh.vertices.size());
            ImGui::Text("\t\tNumber of normals: %ld", mesh.normals.size());
            ImGui::Text("\t\tNumber of texture coordinates: %ld", mesh.uvs.size());
            ImGui::Text("\t\tNumber of indices: %ld", mesh.indices.size());
        }
        ImGui::Text("Number of materials: %ld", rawModel.materials.size());
        for (auto &entry : rawModel.materials) {
            auto &material = entry.second;
            ImGui::Text("\tName: %s", material->name.c_str());
            ImGui::Text("\t\tDiffuse Texture Map: %s", material->diffuseTextureMap.c_str());
        }
    }

    ImGui::End();
}
