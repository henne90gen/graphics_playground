#pragma once

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include <Image.h>
#include <ImageOps.h>
#include <gl/Texture.h>
#include <gl/VertexArray.h>
#include <gl/VertexBuffer.h>

struct RawMaterial {
    std::string name;
    std::string diffuseTextureMap;
};

struct RawMesh {
    std::string name;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::ivec3> indices;

    RawMaterial *material;
};

struct RawModel {
    std::vector<RawMesh> meshes;
    std::map<std::string, RawMaterial *> materials;
};

class OpenGLMesh {
  public:
    OpenGLMesh(const TextureSettings &textureSettings, const std::shared_ptr<Shader> &shader, bool visible = true)
        : vertexArray(std::make_shared<VertexArray>(shader)), vertexBuffer(std::make_shared<VertexBuffer>()),
          normalBuffer(std::make_shared<VertexBuffer>()), uvBuffer(std::make_shared<VertexBuffer>()),
          indexBuffer(std::make_shared<IndexBuffer>()), texture(std::make_shared<Texture>(textureSettings)),
          visible(visible) {}

    void updateMeshVertices(const RawMesh &mesh, const std::shared_ptr<Shader> &shader) const;

    void updateTexture(RawMesh &rawMesh, std::unordered_map<std::string, std::shared_ptr<Texture>> &textures);

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

    bool isLoaded() const { return loaded; }
    const std::vector<OpenGLMesh> getMeshes() const { return meshes; }
    const RawModel getRawModel() const { return rawModel; }

    static unsigned int loadFromFile(const std::string &fileName, const std::shared_ptr<Shader> &shader, Model &model);
    static unsigned int loadRawModelFromFile(const std::string &fileName, RawModel &model);

  private:
    std::vector<OpenGLMesh> meshes;
    RawModel rawModel;
    bool loaded;
};
