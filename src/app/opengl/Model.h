#include <utility>

#pragma once

#include <vector>
#include <memory>

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/Texture.h"
#include "model_loading/ModelLoader.h"

class OpenGLMesh {
public:
    OpenGLMesh(std::shared_ptr<VertexArray> vertexArray,
               std::shared_ptr<VertexBuffer> vertexBuffer,
               std::shared_ptr<IndexBuffer> indexBuffer,
               std::shared_ptr<Texture> texture,
               bool visible = true) : vertexArray(std::move(vertexArray)), vertexBuffer(std::move(vertexBuffer)),
                                      indexBuffer(std::move(indexBuffer)), texture(std::move(texture)),
                                      visible(visible) {}

    void updateMeshVertices(const ModelLoader::RawMesh &mesh, const std::shared_ptr<Shader> &shader);

    void updateTexture(ModelLoader::RawMesh &mesh);

    static void createCheckerBoard(Image &image);

    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    std::shared_ptr<IndexBuffer> indexBuffer;
    std::shared_ptr<Texture> texture;
    bool visible = true;
};

class Model {
public:
    Model() = default;

    void loadFromFile(const std::string &fileName, const std::shared_ptr<Shader> &shader);

    const std::vector<std::shared_ptr<OpenGLMesh>> getMeshes() const { return meshes; }

    const std::shared_ptr<ModelLoader::RawModel> getOriginalModel() const { return model; }

private:
    std::vector<std::shared_ptr<OpenGLMesh>> meshes;
    std::shared_ptr<ModelLoader::RawModel> model;
};
