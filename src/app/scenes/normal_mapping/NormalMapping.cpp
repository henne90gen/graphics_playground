#include <util/Image.h>
#include "NormalMapping.h"

const float FIELD_OF_VIEW = 45.0F;
const float Z_NEAR = 0.1F;
const float Z_FAR = 100.0F;

void NormalMapping::setup() {
    shader = std::make_shared<Shader>("../../../src/app/scenes/normal_mapping/NormalMappingVert.glsl",
                                      "../../../src/app/scenes/normal_mapping/NormalMappingFrag.glsl");
    shader->bind();
    auto projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("u_Projection", projectionMatrix);

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
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0},
            {0.0, 0.0, 1.0}
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
    static glm::vec3 cameraPosition = {1.0, 0.0, -5.0};
    static glm::vec3 cameraRotation = {};
    static glm::vec3 position = {};
    static glm::vec3 rotation = {};

    ImGui::Begin("Settings");
    ImGui::DragFloat3("Camera Position", (float *) &cameraPosition, 0.01F);
    ImGui::DragFloat3("Camera Rotation", (float *) &cameraRotation, 0.01F);
    ImGui::DragFloat3("Model Position", (float *) &position, 0.01F);
    ImGui::DragFloat3("Model Rotation", (float *) &rotation, 0.01F);
    ImGui::End();

    shader->bind();
    auto modelMatrix = glm::identity<glm::mat4>();
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0, 1, 0));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0, 0, 1));
    shader->setUniform("u_Model", modelMatrix);
    auto viewMatrix = createViewMatrix(cameraPosition, cameraRotation);
    shader->setUniform("u_View", viewMatrix);

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

void NormalMapping::onAspectRatioChange() {
    shader->bind();
    auto projectionMatrix = glm::perspective(glm::radians(FIELD_OF_VIEW), getAspectRatio(), Z_NEAR, Z_FAR);
    shader->setUniform("u_Projection", projectionMatrix);
}