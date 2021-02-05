#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "ModelLoader.h"

#include <Image.h>
#include <ImageOps.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <gl/VertexBuffer.h>

class OpenGLMesh {
  public:
    OpenGLMesh(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<VertexBuffer> vertexBuffer,
               std::shared_ptr<VertexBuffer> normalBuffer, std::shared_ptr<VertexBuffer> uvBuffer,
               std::shared_ptr<IndexBuffer> indexBuffer, std::shared_ptr<Texture> texture, bool visible = true)
        : vertexArray(std::move(vertexArray)), vertexBuffer(std::move(vertexBuffer)), normalBuffer(std::move(normalBuffer)),
          uvBuffer(std::move(uvBuffer)), indexBuffer(std::move(indexBuffer)), texture(std::move(texture)), visible(visible) {}

    void updateMeshVertices(const ModelLoader::RawMesh &mesh, const std::shared_ptr<Shader> &shader) const;

    void updateTexture(ModelLoader::RawMesh &mesh) const;

    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    std::shared_ptr<VertexBuffer> normalBuffer;
    std::shared_ptr<VertexBuffer> uvBuffer;
    std::shared_ptr<IndexBuffer> indexBuffer;
    std::shared_ptr<Texture> texture;
    bool visible = true;
};

class Model {
  public:
    Model() = default;

    void loadFromFile(const std::string &fileName, const std::shared_ptr<Shader> &shader);

    const std::vector<std::shared_ptr<OpenGLMesh>> getMeshes() const { return meshes; }

    const std::shared_ptr<ModelLoader::RawModel> getRawModel() const { return rawModel; }

  private:
    std::vector<std::shared_ptr<OpenGLMesh>> meshes;
    std::shared_ptr<ModelLoader::RawModel> rawModel;
};
