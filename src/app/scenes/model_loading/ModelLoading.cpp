#include <shared/Image.h>
#include "ModelLoading.h"

#include "model_loading/ModelLoader.h"
#include "util/ImGuiUtils.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

void ModelLoading::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/model_loading/ModelLoadingVert.glsl",
                                      "../../../src/app/scenes/model_loading/ModelLoadingFrag.glsl");
    shader->bind();

    projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
}

void ModelLoading::destroy() {}

void ModelLoading::tick() {
    static auto translation = glm::vec3(1.7F, -3.5F, -12.0F); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
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
    showSettings(
            rotate,
            translation,
            modelRotation,
            cameraRotation,
            scale,
            drawWireframe,
            currentModel,
            paths,
            model,
            openGLModel
    );

    shader->bind();

    if (prevModel != currentModel) {
        std::string modelFileName = paths[currentModel];
        updateModel(modelFileName);
        prevModel = currentModel;
    }

    drawModel(translation, modelRotation, cameraRotation, scale, drawWireframe);

    shader->unbind();
}

void ModelLoading::drawModel(const glm::vec3 &translation, const glm::vec3 &modelRotation,
                             const glm::vec3 &cameraRotation, float scale, bool drawWireframe) const {
    if (!openGLModel) {
        return;
    }

    for (auto &mesh : openGLModel->getMeshes()) {
        if (!mesh->visible) {
            continue;
        }

        mesh->vertexArray->bind();

        glm::mat4 modelMatrix = glm::mat4(1.0F);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.x, glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.y, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, modelRotation.z, glm::vec3(0, 0, 1));
        glm::mat4 viewMatrix = createViewMatrix(translation, cameraRotation);
        shader->setUniform("u_Model", modelMatrix);
        shader->setUniform("u_View", viewMatrix);
        shader->setUniform("u_Projection", projectionMatrix);

        shader->setUniform("u_TextureSampler", 0);
        mesh->texture->bind();

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        }

        GL_Call(glDrawElements(GL_TRIANGLES, mesh->indexBuffer->getCount(), GL_UNSIGNED_INT, nullptr));

        if (drawWireframe) {
            GL_Call(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        mesh->vertexArray->unbind();
    }
}

void
showSettings(bool &rotate, glm::vec3 &translation, glm::vec3 &modelRotation, glm::vec3 &cameraRotation, float &scale,
             bool &drawWireframe, unsigned int &currentModel, std::vector<std::string> &paths,
             ModelLoader::Model &model, std::unique_ptr<OpenGLModel> &renderModel) {
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
    for (unsigned long i = 0; i < model.meshes.size(); i++) {
        auto &mesh = model.meshes[i];
        auto &renderMesh = renderModel->getMeshes()[i];
        ImGui::Checkbox(("\tName: " + mesh.name).c_str(), &renderMesh->visible);
        ImGui::Text("\t\tNumber of vertices: %ld", mesh.vertices.size());
        ImGui::Text("\t\tNumber of normals: %ld", mesh.normals.size());
        ImGui::Text("\t\tNumber of texture coordinates: %ld", mesh.textureCoordinates.size());
        ImGui::Text("\t\tNumber of indices: %ld", mesh.indices.size());
    }
    ImGui::Text("Number of materials: %ld", model.materials.size());
    for (auto &entry : model.materials) {
        auto &material = entry.second;
        ImGui::Text("\tName: %s", material->name.c_str());
        ImGui::Text("\t\tDiffuse Texture Map: %s", material->diffuseTextureMap.c_str());
    }

    ImGui::End();
}

void ModelLoading::updateModel(const std::string &modelFileName) {
    int error = ModelLoader::fromFile(modelFileName, model);
    if (error) {
        std::cout << "Could not load model." << std::endl;
        return;
    }

    openGLModel = std::make_unique<OpenGLModel>();

    for (auto &mesh : model.meshes) {
        OpenGLMesh renderMesh = {
                std::make_shared<VertexArray>(shader),
                std::make_shared<VertexBuffer>(),
                std::make_shared<IndexBuffer>(),
                std::make_shared<Texture>(GL_RGBA)
        };

        renderMesh.vertexArray->bind();

        updateVertices(mesh, renderMesh);

        renderMesh.indexBuffer->bind();
        renderMesh.indexBuffer->update(mesh.indices);

        renderMesh.texture->bind();
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

        updateTexture(mesh, renderMesh);

        openGLModel->addMesh(std::make_shared<OpenGLMesh>(renderMesh));
    }
}

void ModelLoading::updateVertices(const ModelLoader::Mesh &mesh, const OpenGLMesh &renderMesh) const {
    bool hasNormals = !mesh.normals.empty();
    shader->setUniform("u_HasNormals", hasNormals);
    bool hasTexture = !mesh.textureCoordinates.empty();
    shader->setUniform("u_HasTexture", hasTexture);

    std::vector<float> vertices;
    for (unsigned long i = 0; i < mesh.vertices.size(); i++) {
        vertices.push_back(mesh.vertices[i].x);
        vertices.push_back(mesh.vertices[i].y);
        vertices.push_back(mesh.vertices[i].z);

        if (hasNormals) {
            vertices.push_back(mesh.normals[i].x);
            vertices.push_back(mesh.normals[i].y);
            vertices.push_back(mesh.normals[i].z);
        } else {
            vertices.push_back(0);
            vertices.push_back(0);
            vertices.push_back(0);
        }
        if (hasTexture) {
            vertices.push_back(mesh.textureCoordinates[i].x);
            vertices.push_back(mesh.textureCoordinates[i].y);
        } else {
            vertices.push_back(0);
            vertices.push_back(0);
        }
    }

    BufferLayout positionLayout = {
            {Float3, "a_Position"},
            {Float3, "a_Normal"},
            {Float2, "a_UV"}
    };
    renderMesh.vertexBuffer->setLayout(positionLayout);
    renderMesh.vertexBuffer->update(vertices);
    renderMesh.vertexArray->addVertexBuffer(renderMesh.vertexBuffer);
}

void ModelLoading::updateTexture(ModelLoader::Mesh &mesh, OpenGLMesh &renderMesh) {
    if (!mesh.material) {
        createCheckerBoard(renderMesh);
        return;
    }

    Image image(mesh.material->diffuseTextureMap);
    image.load();
    if (!image.isLoaded()) {
        createCheckerBoard(renderMesh);
        return;
    }

    renderMesh.texture->update(image.getPixels().data(), image.getWidth(), image.getHeight());
}

void ModelLoading::createCheckerBoard(OpenGLMesh &mesh) {
    unsigned int width = 128;
    unsigned int height = 128;
    int numberOfChannels = 4;
    std::vector<unsigned char> data = std::vector<unsigned char>(width * height * numberOfChannels);
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
    mesh.texture->update(data.data(), width, height);
}
