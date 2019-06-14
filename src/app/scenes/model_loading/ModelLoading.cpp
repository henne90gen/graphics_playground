#include <util/ImageUtils.h>
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
    normalBuffer = new VertexBuffer();
    textureCoordinatesBuffer = new VertexBuffer();

    VertexBufferLayout bufferLayout = {};
    bufferLayout.add<float>(shader, "a_Position", 3);
    vertexArray->addBuffer(*vertexBuffer, bufferLayout);

    bufferLayout = {};
    bufferLayout.add<float>(shader, "a_Normal", 3);
    vertexArray->addBuffer(*normalBuffer, bufferLayout);

    bufferLayout = {};
    bufferLayout.add<float>(shader, "a_UV", 2);
    vertexArray->addBuffer(*textureCoordinatesBuffer, bufferLayout);

    indexBuffer = new IndexBuffer();
    indexBuffer->bind();

    texture = new Texture(GL_RGBA);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform("u_TextureSampler", 0);
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

    std::vector<std::string> paths = {};
    showSettings(rotate, translation, modelRotation, cameraRotation, scale, drawWireframe, currentModel, paths, model);

    shader->bind();
    vertexArray->bind();

    if (prevModel != currentModel) {
        std::string modelFileName = paths[currentModel];
        updateModel(modelFileName);
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

void showSettings(bool &rotate, glm::vec3 &translation, glm::vec3 &modelRotation, glm::vec3 &cameraRotation,
                  float &scale, bool &drawWireframe, unsigned int &currentModel,
                  std::vector<std::string> &paths, ModelLoader::Model &model) {
    ImGui::Begin("Settings");
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

    ImGui::Text("Number of meshes: %ld", model.meshes.size());
    for (auto &mesh : model.meshes) {
        ImGui::Text("\tName: %s", mesh.name.c_str());
        ImGui::Text("\t\tNumber of vertices: %ld", mesh.vertices.size());
        ImGui::Text("\t\tNumber of normals: %ld", mesh.normals.size());
        ImGui::Text("\t\tNumber of texture coordinates: %ld", mesh.textureCoordinates.size());
        ImGui::Text("\t\tNumber of indices: %ld", mesh.indices.size());
    }
    ImGui::Text("Number of materials: %ld", model.materials.size());
    for (auto &entry : model.materials) {
        auto &material = entry.second;
        ImGui::Text("\tName: %s", material.name.c_str());
        ImGui::Text("\t\tDiffuse Texture Map: %s", material.diffuseTextureMap.c_str());
    }

    ImGui::End();
}

void ModelLoading::updateModel(const std::string &modelFileName) {
    int error = ModelLoader::fromFile(modelFileName, model);
    if (error) {
        std::cout << "Could not load model." << std::endl;
        return;
    }

    ModelLoader::Mesh mesh = model.meshes[0];
    vertexBuffer->update(mesh.vertices);
    if (!mesh.normals.empty()) {
        normalBuffer->update(mesh.normals);
        shader->setUniform("u_HasNormals", true);
    } else {
        shader->setUniform("u_HasNormals", false);
    }
    if (!mesh.textureCoordinates.empty()) {
        textureCoordinatesBuffer->update(mesh.textureCoordinates);
        shader->setUniform("u_HasTexture", true);
    } else {
        shader->setUniform("u_HasTexture", false);
    }
    indexBuffer->update(mesh.indices);

    if (!mesh.hasMaterial) {
        createCheckerBoard();
        return;
    }
    Image image = {};
    error = loadPng(mesh.material.diffuseTextureMap, image);
    if (error) {
        createCheckerBoard();
        return;
    }

    texture->update(image.pixels.data(), image.width, image.height, image.bitDepth);
}

void ModelLoading::createCheckerBoard() {
    unsigned int width = 128;
    unsigned int height = 128;
    int numberOfChannels = 4;
    std::vector<char> data = std::vector<char>(width * height * numberOfChannels);
    for (unsigned long i = 0; i < data.size() / numberOfChannels; i++) {
        const float fullBrightness = 255.0F;
        float r = fullBrightness;
        float g = fullBrightness;
        float b = fullBrightness;
        unsigned int row = i / width;
        if ((i % 2 == 0 && row % 2 == 0) || (i % 2 == 1 && row % 2 == 1)) {
            r = 0.0F;
            g = 0.0F;
            b = 0.0F;
        }
        unsigned int idx = i * numberOfChannels;
        data[idx] = static_cast<char>(r);
        data[idx + 1] = static_cast<char>(g);
        data[idx + 2] = static_cast<char>(b);
    }
    texture->update(data.data(), width, height);
}
