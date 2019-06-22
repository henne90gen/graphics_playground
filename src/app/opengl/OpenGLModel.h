#pragma once

#include <vector>
#include <memory>

#include "opengl/VertexArray.h"
#include "opengl/VertexBuffer.h"
#include "opengl/Texture.h"

struct OpenGLMesh {
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<VertexBuffer> vertexBuffer;
    std::shared_ptr<IndexBuffer> indexBuffer;
    std::shared_ptr<Texture> texture;
    bool visible = true;
};

class OpenGLModel {
public:
    OpenGLModel() = default;

    void addMesh(const std::shared_ptr<OpenGLMesh> &mesh) {
        meshes.push_back(mesh);
    }

    const std::vector<std::shared_ptr<OpenGLMesh>> getMeshes() const { return meshes; }

private:
    std::vector<std::shared_ptr<OpenGLMesh>> meshes;
};
