#include "ModelLoading.h"

#include "model_loading/ModelLoader.h"
#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

void ModelLoading::setup() {
    shader = new Shader("../../../src/app/scenes/model_loading/ModelLoadingVert.glsl",
                        "../../../src/app/scenes/model_loading/ModelLoadingFrag.glsl");
    shader->bind();

    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);

    vertexArray = new VertexArray();
    vertexArray->bind();

    vertexBuffer = new VertexBuffer();
    VertexBufferLayout bufferLayout;
    bufferLayout.add<float>(shader, "a_Position", 3);
//    bufferLayout.add<float>(shader, "a_Normal", 3);
    vertexArray->addBuffer(*vertexBuffer, bufferLayout);
    indexBuffer = new IndexBuffer();
    indexBuffer->bind();
}

void ModelLoading::destroy() {}

void ModelLoading::tick() {
    static auto translation = glm::vec3(1.7F, -1.5F, -5.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static auto modelRotation = glm::vec3();
    static auto cameraRotation = glm::vec3(0.25F, 0.0F, 0.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static float scale = 0.5F; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static bool rotate = true;
    static bool drawWireframe = false;
    static unsigned int currentModel = 0;
    static unsigned int prevModel = currentModel + 1;

    const float rotationSpeed = 0.03F;
    if (rotate) {
        modelRotation.y += rotationSpeed;
    }

    ImGui::Begin("Settings");
    std::vector<std::string> paths = {};
    ImGui::FileSelector("Models", "../../../src/app/scenes/model_loading/models/", currentModel, paths);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Position", reinterpret_cast<float *>(&translation), 0.05F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float *>(&cameraRotation), 0.01F);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,cppcoreguidelines-pro-type-reinterpret-cast)
    ImGui::DragFloat3("Model Rotation", reinterpret_cast<float *>(&modelRotation), 0.01F);
    ImGui::Checkbox("Rotate", &rotate);
    ImGui::Checkbox("Wireframe", &drawWireframe);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    ImGui::DragFloat("Scale", &scale, 0.001F);
    ImGui::End();

    shader->bind();
    vertexArray->bind();

    if (prevModel != currentModel) {
        auto model = ModelLoader::fromFile(paths[currentModel]);
        std::vector<float> vertices = {};
        interleaveModelData(model, vertices, false);
        vertexBuffer->update(vertices);
        indexBuffer->update(model.indices);
        prevModel = currentModel;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0F);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
    glm::mat4 viewMatrix = createViewMatrix(translation, cameraRotation);
    shader->setUniform("u_Model", modelMatrix);
    shader->setUniform("u_View", viewMatrix);
    shader->setUniform("u_Projection", projectionMatrix);

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }

    GL_Call(glDrawElements(GL_TRIANGLES, indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

    if (drawWireframe) {
        GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    vertexArray->unbind();
    shader->unbind();
}

void ModelLoading::interleaveModelData(ModelLoader::Model &model, std::vector<float> &vertices, bool interleaveNormals,
                                       bool interleaveTextureCoordtinates) {
    bool shouldInterleaveNormals = interleaveNormals && !model.normals.empty();
    bool shouldInterleaveTextureCoordinates = interleaveTextureCoordtinates && !model.textureCoordinates.empty();
    bool invalidNormals = shouldInterleaveNormals && model.vertices.size() != model.normals.size();
    bool invalidTextureCoordinates =
            shouldInterleaveTextureCoordinates && model.vertices.size() != model.textureCoordinates.size();
    if (invalidNormals || invalidTextureCoordinates) {
        std::cerr << "Could not interleave model data. Vertices: " << model.vertices.size() << ", Normals: "
                  << model.normals.size() << ", TextureCoordinates: " << model.textureCoordinates.size() << std::endl;
        return;
    }

    for (unsigned long i = 0; i < model.vertices.size(); i++) {
        vertices.push_back(model.vertices[i].x);
        vertices.push_back(model.vertices[i].y);
        vertices.push_back(model.vertices[i].z);
        if (shouldInterleaveNormals) {
            vertices.push_back(model.normals[i].x);
            vertices.push_back(model.normals[i].y);
            vertices.push_back(model.normals[i].z);
        }
        if (shouldInterleaveTextureCoordinates) {
            vertices.push_back(model.textureCoordinates[i].x);
            vertices.push_back(model.textureCoordinates[i].y);
        }
    }
}
