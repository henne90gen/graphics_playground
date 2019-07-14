#include <util/Image.h>
#include "NormalMapping.h"

void NormalMapping::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/normal_mapping/NormalMappingVert.glsl",
                                      "../../../src/app/scenes/normal_mapping/NormalMappingFrag.glsl");
    shader->bind();

    vertexArray = std::make_shared<VertexArray>(shader);
    vertexArray->bind();

    std::vector<glm::vec3> vertices = {
            {-1.0, -1.0, 0.0},
            {1.0,  -1.0, 0.0},
            {1.0,  1.0,  0.0},
            {-1.0, 1.0,  0.0},
    };
    std::vector<glm::vec2> uvs = {
            {0.0, 1.0},
            {1.0, 1.0},
            {1.0, 0.0},
            {0.0, 0.0}
    };
    std::vector<glm::vec3> normals = {
            {0.0, 0.0, 0.0},
            {0.0, 0.0, 0.0},
            {0.0, 0.0, 0.0},
            {0.0, 0.0, 0.0}
    };

    std::vector<float> vertexData;
    interleaveVertexData(vertices, uvs, normals, vertexData);
    BufferLayout layout = {
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_UV"},
            {ShaderDataType::Float3, "a_Normal"}
    };
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertexData, layout);
    vertexArray->addVertexBuffer(vertexBuffer);

    std::vector<glm::ivec3> indices = {
            {0, 1, 2},
            {0, 2, 3}
    };
    auto indexBuffer = std::make_shared<IndexBuffer>(indices);
    vertexArray->setIndexBuffer(indexBuffer);

    texture = std::make_shared<Texture>(GL_RGBA);
    glActiveTexture(GL_TEXTURE0);
    texture->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("u_TextureSampler", 0);
    auto image = Image("../../../src/app/scenes/normal_mapping/TestTex.png");
    texture->update(image);

    normalMap = std::make_shared<Texture>(GL_RGBA);
    glActiveTexture(GL_TEXTURE1);
    normalMap->bind();
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    shader->setUniform<int>("u_NormalSampler", 1);
    auto normalMapImage = Image("../../../src/app/scenes/normal_mapping/normals.jpg");
    normalMap->update(normalMapImage);
}

void NormalMapping::destroy() {}

void NormalMapping::tick() {
    shader->bind();
    vertexArray->bind();
    GL_Call(glDrawElements(GL_TRIANGLES, vertexArray->getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr));
}

void NormalMapping::interleaveVertexData(const std::vector<glm::vec3> &positions, const std::vector<glm::vec2> &uvs,
                                         const std::vector<glm::vec3> &normals, std::vector<float> &output) {
    if (positions.size() != uvs.size() || positions.size() != normals.size()) {
        std::cerr << "Could not interleave vertex data. positions: " << positions.size() << ", uvs: " << uvs.size()
                  << ", normals: " << normals.size() << std::endl;
        return;
    }
    for (unsigned long i = 0; i < positions.size(); i++) {
        output.push_back(positions[i].x);
        output.push_back(positions[i].y);
        output.push_back(positions[i].z);

        output.push_back(uvs[i].x);
        output.push_back(uvs[i].y);

        output.push_back(normals[i].x);
        output.push_back(normals[i].y);
        output.push_back(normals[i].z);
    }
}