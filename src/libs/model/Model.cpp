#include "Model.h"

void Model::loadFromFile(const std::string &fileName, const std::shared_ptr<Shader> &shader) {
    meshes.clear();

    shader->bind();
    model = std::make_shared<ModelLoader::RawModel>();
    unsigned int error = ModelLoader::fromFile(fileName, model);
    if (error != 0) {
        std::cout << "Could not load model." << std::endl;
        return;
    }

    for (auto &mesh : model->meshes) {
        TextureSettings textureSettings = {};
        textureSettings.dataType = GL_RGBA;
        auto glMesh = std::make_shared<OpenGLMesh>(       //
              std::make_shared<VertexArray>(shader),      //
              std::make_shared<VertexBuffer>(),           //
              std::make_shared<IndexBuffer>(),            //
              std::make_shared<Texture>(textureSettings), //
              true                                        //
        );

        glMesh->vertexArray->bind();

        glMesh->updateMeshVertices(mesh, shader);

        glMesh->indexBuffer->bind();
        glMesh->indexBuffer->update(mesh.indices);

        glMesh->texture->bind();
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

        glMesh->updateTexture(mesh);

        meshes.push_back(glMesh);
    }
}

void OpenGLMesh::updateMeshVertices(const ModelLoader::RawMesh &mesh, const std::shared_ptr<Shader> &shader) const {
    bool hasNormals = !mesh.normals.empty();
    shader->setUniform("u_HasNormals", hasNormals);
    bool hasTexture = !mesh.textureCoordinates.empty();
    shader->setUniform("u_HasTexture", hasTexture);

    auto vertices = std::vector<float>(mesh.vertices.size() * 8);
#pragma omp parallel for
    for (int i = 0; i < mesh.vertices.size(); i++) {
        vertices[i * 8 + 0] = mesh.vertices[i].x;
        vertices[i * 8 + 1] = mesh.vertices[i].y;
        vertices[i * 8 + 2] = mesh.vertices[i].z;

        if (hasNormals) {
            vertices[i * 8 + 3] = mesh.normals[i].x;
            vertices[i * 8 + 4] = mesh.normals[i].y;
            vertices[i * 8 + 5] = mesh.normals[i].z;
        } else {
            vertices[i * 8 +3] = 0;
            vertices[i * 8 + 4] = 0;
            vertices[i * 8 + 5] = 0;
        }
        if (hasTexture) {
            vertices[i * 8 + 6] = mesh.textureCoordinates[i].x;
            vertices[i * 8 + 7] = mesh.textureCoordinates[i].y;
        } else {
            vertices[i * 8 + 6] = 0;
            vertices[i * 8 + 7] = 0;
        }
    }

    BufferLayout positionLayout = {{Float3, "a_Position"}, {Float3, "a_Normal"}, {Vec2, "a_UV"}};
    vertexBuffer->setLayout(positionLayout);
    vertexBuffer->update(vertices);
    vertexArray->addVertexBuffer(vertexBuffer);
}

void OpenGLMesh::updateTexture(ModelLoader::RawMesh &mesh) const {
    Image image = {};
    if (!mesh.material || !ImageOps::load(mesh.material->diffuseTextureMap, image)) {
        ImageOps::createCheckerBoard(image);
    }
    texture->update(image.pixels.data(), image.width, image.height);
}
