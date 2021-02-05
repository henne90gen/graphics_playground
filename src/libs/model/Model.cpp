#include "Model.h"

#include <util/TimeUtils.h>

void Model::loadFromFile(const std::string &fileName, const std::shared_ptr<Shader> &shader) {
    TIME_SCOPE_NAME("loadFromFile");

    meshes.clear();

    shader->bind();
    rawModel = std::make_shared<ModelLoader::RawModel>();
    unsigned int error = ModelLoader::fromFile(fileName, rawModel);
    if (error != 0) {
        std::cout << "Could not load model." << std::endl;
        return;
    }

    for (auto &rawMesh : rawModel->meshes) {
        TextureSettings textureSettings = {};
        textureSettings.dataType = GL_RGBA;
        auto glMesh = std::make_shared<OpenGLMesh>(       //
              std::make_shared<VertexArray>(shader),      //
              std::make_shared<VertexBuffer>(),           //
              std::make_shared<VertexBuffer>(),           //
              std::make_shared<VertexBuffer>(),           //
              std::make_shared<IndexBuffer>(),            //
              std::make_shared<Texture>(textureSettings), //
              true                                        //
        );

        glMesh->vertexArray->bind();

        glMesh->updateMeshVertices(rawMesh, shader);

        glMesh->indexBuffer->bind();
        glMesh->indexBuffer->update(rawMesh.indices);

        glMesh->texture->bind();
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

        glMesh->updateTexture(rawMesh);

        meshes.push_back(glMesh);
    }
}

void OpenGLMesh::updateMeshVertices(const ModelLoader::RawMesh &mesh, const std::shared_ptr<Shader> &shader) const {
    //    TIME_SCOPE_NAME("updateMeshVertices");
    bool hasNormals = !mesh.normals.empty();
    shader->setUniform("u_HasNormals", hasNormals);
    bool hasTexture = !mesh.uvs.empty();
    shader->setUniform("u_HasTexture", hasTexture);

    BufferLayout positionLayout = {{Float3, "a_Position"}};
    vertexBuffer->setLayout(positionLayout);
    vertexBuffer->update(mesh.vertices);
    vertexArray->addVertexBuffer(vertexBuffer);

    if (hasNormals) {
        BufferLayout normalLayout = {{Float3, "a_Normal"}};
        normalBuffer->setLayout(normalLayout);
        normalBuffer->update(mesh.normals);
        vertexArray->addVertexBuffer(normalBuffer);
    }

    if (hasTexture) {
        BufferLayout uvLayout = {{Float2, "a_UV"}};
        uvBuffer->setLayout(uvLayout);
        uvBuffer->update(mesh.uvs);
        vertexArray->addVertexBuffer(uvBuffer);
    }
}

void OpenGLMesh::updateTexture(ModelLoader::RawMesh &mesh) const {
    Image image = {};
    //    if (!mesh.material || !ImageOps::load(mesh.material->diffuseTextureMap, image)) {
    ImageOps::createCheckerBoard(image);
    //    }
    texture->update(image.pixels.data(), image.width, image.height);
}
