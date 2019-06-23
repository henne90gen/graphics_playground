#include "Model.h"
#include <shared/Image.h>

void Model::loadFromFile(const std::string &fileName, const std::shared_ptr<Shader> &shader) {
    meshes.clear();

    model = std::make_shared<ModelLoader::RawModel>();
    int error = ModelLoader::fromFile(fileName, model);
    if (error != 0) {
        std::cout << "Could not load model." << std::endl;
        return;
    }

    for (auto &mesh : model->meshes) {
        auto glMesh = std::make_shared<OpenGLMesh>(
                std::make_shared<VertexArray>(shader),
                std::make_shared<VertexBuffer>(),
                std::make_shared<IndexBuffer>(),
                std::make_shared<Texture>(GL_RGBA),
                true
        );

        glMesh->vertexArray->bind();

        glMesh->updateMeshVertices(mesh, shader);

        glMesh->indexBuffer->bind();
        glMesh->indexBuffer->update(mesh.indices);

        glMesh->texture->bind();
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

        glMesh->updateTexture(mesh);

        meshes.push_back(glMesh);
    }
}

void OpenGLMesh::updateMeshVertices(const ModelLoader::RawMesh &mesh, const std::shared_ptr<Shader> &shader) {
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
    vertexBuffer->setLayout(positionLayout);
    vertexBuffer->update(vertices);
    vertexArray->addVertexBuffer(vertexBuffer);
}

void OpenGLMesh::updateTexture(ModelLoader::RawMesh &mesh) {
    if (!mesh.material) {
        createCheckerBoardTexture();
        return;
    }

    Image image(mesh.material->diffuseTextureMap);
    image.load();
    if (!image.isLoaded()) {
        createCheckerBoardTexture();
        return;
    }

    texture->update(image.getPixels().data(), image.getWidth(), image.getHeight());
}

void OpenGLMesh::createCheckerBoardTexture() {
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
    texture->update(data.data(), width, height);
}
